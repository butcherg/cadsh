
#include "manifold/manifold.h"

//utility routines:

unsigned addVertex(manifold::MeshGL &mesh, double x, double y, double z)
{
	unsigned idx = mesh.vertProperties.size();
	mesh.vertProperties.insert(mesh.vertProperties.end(), { (float) x, (float) y, (float) z} );
	return idx/3;
}	

unsigned addTriangle(manifold::MeshGL &mesh, unsigned a, unsigned b, unsigned c)
{
	unsigned idx = mesh.triVerts.size();
	mesh.triVerts.insert(mesh.triVerts.end(), { a, b, c});
	return idx/3;
}


//icosahedron:

manifold::MeshGL icosahedron()
{
  manifold::MeshGL mesh;
  mesh.numProp = 3;

  float phi = (1.0f + sqrt(5.0f)) * 0.5f; // golden ratio
  float a = 1.0f;
  float b = 1.0f / phi;

  // add vertices
  auto v1  = addVertex(mesh, 0, b, -a);
  auto v2  = addVertex(mesh, b, a, 0);
  auto v3  = addVertex(mesh, -b, a, 0);
  auto v4  = addVertex(mesh, 0, b, a);
  auto v5  = addVertex(mesh, 0, -b, a);
  auto v6  = addVertex(mesh, -a, 0, b);
  auto v7  = addVertex(mesh, 0, -b, -a);
  auto v8  = addVertex(mesh, a, 0, -b);
  auto v9  = addVertex(mesh, a, 0, b);
  auto v10 = addVertex(mesh, -a, 0, -b);
  auto v11 = addVertex(mesh, b, -a, 0);
  auto v12 = addVertex(mesh, -b, -a, 0);

  //project_to_unit_sphere(mesh);

  // add triangles
  addTriangle(mesh, v3, v2, v1);
  addTriangle(mesh, v2, v3, v4);
  addTriangle(mesh, v6, v5, v4);
  addTriangle(mesh, v5, v9, v4);
  addTriangle(mesh, v8, v7, v1);
  addTriangle(mesh, v7, v10, v1);
  addTriangle(mesh, v12, v11, v5);
  addTriangle(mesh, v11, v12, v7);
  addTriangle(mesh, v10, v6, v3);
  addTriangle(mesh, v6, v10, v12);
  addTriangle(mesh, v9, v8, v2);
  addTriangle(mesh, v8, v9, v11);
  addTriangle(mesh, v3, v6, v4);
  addTriangle(mesh, v9, v2, v4);
  addTriangle(mesh, v10, v3, v1);
  addTriangle(mesh, v2, v8, v1);
  addTriangle(mesh, v12, v10, v7);
  addTriangle(mesh, v8, v11, v7);
  addTriangle(mesh, v6, v12, v5);
  addTriangle(mesh, v11, v9, v5);

  return mesh;
}


// heightmap:

manifold::MeshGL heightmap2mesh(std::vector<std::vector<float>> hm)
{	
	struct ht {
		float h;  //height
		unsigned i; //vertex index in the point list
	};
	
	//load heightmap into ht struct:
	std::vector<std::vector<ht>> hmp;
	for (auto r : hm) {
		std::vector<ht> rf;
		for (auto c : r) {
			ht height;
			height.h = c;
			rf.push_back(height);
		}
		hmp.push_back(rf);
	}
	
	int w = hmp.size();
	int h = hmp[0].size();

	std::vector<std::vector<ht>> base = hmp;
	
	manifold::MeshGL m;
	
	//1. build heightmap mesh:
		//a. buildVertices
		for (unsigned y=0; y<h; y++) {
			for (unsigned x=0; x<w; x++) {
				hmp[x][y].i = addVertex(m, (float) x, (float) y, hmp[x][y].h);
			}
		}
		//b. fourTriangulate:;
		//b1. add center vertices:
		int centerVertexOffset = m.vertProperties.size()/3;
		for (int y = 0; y < h-1; ++y) {
			for (int x = 0; x < w-1; ++x) {
				float x_center = x + 0.5f;
				float y_center = y + 0.5f;
				float z_center = (hmp[x][y].h + hmp[x][y+1].h + hmp[x+1][y].h + hmp[x+1][y+1].h) / 4.0f;
				addVertex(m,  x_center, y_center, z_center );
			}
		}
		
		//b2. make four-triangle sets:
		for (int y = 0; y < h - 1; ++y) {
			for (int x = 0; x < w - 1; ++x) {
				// Get indices of the four corners of the quad
				unsigned topLeft = hmp[x][y].i;
				unsigned topRight = hmp[x+1][y].i;
				unsigned bottomLeft = hmp[x][y+1].i;
				unsigned bottomRight = hmp[x+1][y+1].i;
				unsigned int quadCenter = centerVertexOffset + y * (w - 1) + x;

				addTriangle(m, bottomLeft, topLeft, quadCenter);
				addTriangle(m, topLeft, topRight, quadCenter);
				addTriangle(m, topRight, bottomRight, quadCenter);
				addTriangle(m, bottomRight, bottomLeft, quadCenter);
			}
		}
	
	//2. build base floor mesh:
		//a. setHeight
		for (unsigned y=0; y<h; y++) 
			for (unsigned x=0; x<w; x++)
				base[x][y].h = -1;
		//b. buildVertices
		for (unsigned y=0; y<h; y++) {
			for (unsigned x=0; x<w; x++) {
				base[x][y].i = addVertex(m,  (float) x, (float) y, base[x][y].h);
			}
		}
		//c. centerTriangulate
		unsigned center = addVertex(m,  (float) w/2, (float) h/2, base[0][0].h);
		
		//getEdgeIndices();
			std::vector<unsigned> bee;
			//along top (north) edge (x axis)
			for(int x=0; x<w-1; x++)
				bee.push_back(base[x][0].i);
			//along right (east) edge (x=w-1)
			for(int y=0; y<h-1; y++)
				bee.push_back(base[w-1][y].i);
			//along bottom (south) edge (y=h-1)
			for(int x=w-1; x>=0; x--)
				bee.push_back(base[x][h-1].i);
			//along left (west) edge (y axis)
			for(int y=h-2; y>=1; y--)
				bee.push_back(base[0][y].i);
			
		for(int i=1; i<bee.size(); i++) 
			addTriangle(m, center, bee[i], bee[i-1]);
		int ic = bee.size()-1;
		int jc = 0;
		addTriangle(m, center, bee[jc], bee[ic]);
	
	//3. connect the heightmap and base meshes:
		//a. heightmap getEdgeIndices
		std::vector<unsigned> he;
		//along top (north) edge (x axis)
		for(int x=0; x<w-1; x++)
			he.push_back(hmp[x][0].i);
		//along right (east) edge (x=w-1)
		for(int y=0; y<h-1; y++)
			he.push_back(hmp[w-1][y].i);
		//along bottom (south) edge (y=h-1)
		for(int x=w-1; x>=0; x--)
			he.push_back(hmp[x][h-1].i);
		//along left (west) edge (y axis)
		for(int y=h-2; y>=1; y--)
			he.push_back(hmp[0][y].i);
		//b. base getEdgeIndices
		std::vector<unsigned> be;
		//along top (north) edge (x axis)
		for(int x=0; x<w-1; x++)
			be.push_back(base[x][0].i);
		//along right (east) edge (x=w-1)
		for(int y=0; y<h-1; y++)
			be.push_back(base[w-1][y].i);
		//along bottom (south) edge (y=h-1)
		for(int x=w-1; x>=0; x--)
			be.push_back(base[x][h-1].i);
		//along left (west) edge (y axis)
		for(int y=h-2; y>=1; y--)
			be.push_back(base[0][y].i);
	
		for(int i=1; i<he.size(); i++) {
			addTriangle(m, he[i], be[i-1], be[i]);
			addTriangle(m, he[i], he[i-1], be[i-1]);
		}
	
	//4. add last base-heightmap connector triangle
		int i = he.size()-1;
		int j = 0;
	
		addTriangle(m, he[i],be[i],be[j]);
		addTriangle(m, he[j],he[i],be[j]);
	
	return m;
}