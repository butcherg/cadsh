#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <cstdint>


struct vec3f {
	float x, y, z;
};

struct vec3i {
	unsigned x, y, z;
};

class Mesh
{
public:

	Mesh() {
		points.clear();
		triangles.clear();
	}
	
	int setPt(vec3f p)
	{
		points.push_back(p);
		return points.size()-1;
	}
	
	int setTri(vec3i t)
	{
		triangles.push_back(t);
		return triangles.size()-1;
	}
	
	std::vector<vec3f> points;
	std::vector<vec3i> triangles;
};

std::vector<std::string> hsplit(std::string s, std::string delim)
{
	std::vector<std::string> v;
	if (s.find(delim) == std::string::npos) {
		v.push_back(s);
		return v;
	}
	size_t pos=0;
	size_t start;
	while (pos < s.length()) {
		start = pos;
		pos = s.find(delim,pos);
		if (pos == std::string::npos) {
			v.push_back(s.substr(start,s.length()-start));
			return v;
		}
		v.push_back(s.substr(start, pos-start));
		pos += delim.length();
	}
	return v;
}

class HeightMap 
{
public:

	struct ht {
		float h;  //height
		unsigned i; //vertex index in the point list
	};

	HeightMap()
	{
		w = h = 0;
		triangulated=vertexed=centervertexed=false;
	}
	
	HeightMap(const HeightMap& htmp)
	{
		hm=htmp.hm;
		w=htmp.w;
		h=htmp.h;
		z = htmp.z;
		triangulated = htmp.triangulated;
		vertexed = htmp.vertexed;
		centervertexed = htmp.centervertexed;
	}
	
	void loadHeightMap(std::string filename)
	{
		std::ifstream inputFile(filename);
		if (inputFile.is_open()) {
			std::string line;
			while (getline(inputFile, line)) {
				if (line.size() == 0) continue;
				std::vector<std::string> l = hsplit(line, " ");
				std::vector<ht> lt;
				for (auto n : l) {
					ht height;
					height.h = atof(n.c_str());
					lt.push_back(height);
				}
				hm.push_back(lt);
			}
			inputFile.close();
			w = hm.size();
			h = hm[0].size();
		}
	}
	
	/*
	void loadHeightMap(std::vector<std::vector<float>> heightmap)
	{
		std::vector<std::vector<ht>> hmt(heightmap.size(), std::vector<ht>(heightmap[0].size()));
		for(unsigned y=0; y<hmt.size(); y++) 
			for(unsigned x=0; x<hmt[0].size(); x++)
				hmt[x][y].h = heightmap[x][y];
		hm = hmt;
		w = hm.size();
		h = hm[0].size();
	}
	*/
	
	void loadHeightMap(std::vector<std::vector<float>> heightmap)
	{
		for (auto l : heightmap) {
			std::vector<ht> lt;
			for (auto c : l) {
				ht height;
				height.h = c;
				lt.push_back(height);
			}
			hm.push_back(lt);
		}
		w = hm.size();
		h = hm[0].size();
	}
	
	void printHeightMap()
	{
		//std::cout << "printHeightMap..." << std::endl;
		for (unsigned y=0; y<h; y++) {
			for (unsigned x=0; x<w; x++) {
				printf("%1.1f ", hm[x][y].h); fflush(stdout);
			}
			printf("\n");
		}
	}
	
	void setHeight(float val)
	{
		for (unsigned y=0; y<h; y++) 
			for (unsigned x=0; x<w; x++)
				hm[x][y].h = val;
	}
	
	bool buildVertices(Mesh &m)
	{
		if (vertexed) return false;
		for (unsigned y=0; y<h; y++) {
			for (unsigned x=0; x<w; x++) {
				hm[x][y].i = m.setPt({ (float) x, (float) y, hm[x][y].h});
			}
		}
		return true;
	}
	
	bool twoTriangulate(Mesh &m)
	{
		if (triangulated) return false;
		for (int y = 0; y < h - 1; ++y) {
			for (int x = 0; x < w - 1; ++x) {
				unsigned topLeft = hm[x][y].i;
				unsigned topRight = hm[x+1][y].i;
				unsigned bottomLeft = hm[x][y+1].i;
				unsigned bottomRight = hm[x+1][y+1].i;
				
				m.setTri({topRight,topLeft,bottomLeft});
				m.setTri({bottomLeft,bottomRight,topRight});
			}
		}
		triangulated = true;
		return true;
	}
	
	bool fourTriangulate(Mesh &m)
	{
		if (triangulated) return false;
		
		//add center vertices:
		int centerVertexOffset = m.points.size();
		for (int y = 0; y < h-1; ++y) {
			for (int x = 0; x < w-1; ++x) {
				float x_center = x + 0.5f;
				float y_center = y + 0.5f;
				float z_center = (hm[x][y].h + hm[x][y+1].h + hm[x+1][y].h + hm[x+1][y+1].h) / 4.0f;
		
				m.setPt({ x_center, y_center, z_center });
			}
		}
		
		//make four-triangle sets:
		for (int y = 0; y < h - 1; ++y) {
			for (int x = 0; x < w - 1; ++x) {
				// Get indices of the four corners of the quad
				unsigned topLeft = hm[x][y].i;
				unsigned topRight = hm[x+1][y].i;
				unsigned bottomLeft = hm[x][y+1].i;
				unsigned bottomRight = hm[x+1][y+1].i;
				unsigned int quadCenter = centerVertexOffset + y * (w - 1) + x;

				m.setTri({bottomLeft, topLeft, quadCenter});
				m.setTri({topLeft, topRight, quadCenter});
				m.setTri({topRight, bottomRight, quadCenter});
				m.setTri({bottomRight, bottomLeft, quadCenter});
			}
		}
		triangulated = true;
		return true;
	}
	
	bool centerTriangulate(Mesh &m)
	{
		if (triangulated) return false;
		
		unsigned center = m.setPt({ (float) w/2, (float) h/2, hm[0][0].h});
		
		std::vector<unsigned> he = getEdgeIndices();
		
		for(int i=1; i<he.size(); i++) 
			m.setTri({center, he[i], he[i-1]});
	
		int i = he.size()-1;
		int j = 0;
	
		m.setTri({center, he[j], he[i]});
		
		triangulated = true;
		return true;
	}

	std::vector<unsigned> getEdgeIndices()
	{
		//std::cout << "getEdgeCoords..." << std::endl;
		std::vector<unsigned> i;

		//along top (north) edge (x axis)
		for(int x=0; x<w-1; x++)
			i.push_back(hm[x][0].i);
		//along right (east) edge (x=w-1)
		for(int y=0; y<h-1; y++)
			i.push_back(hm[w-1][y].i);
		//along bottom (south) edge (y=h-1)
		for(int x=w-1; x>=0; x--)
			i.push_back(hm[x][h-1].i);
		//along left (west) edge (y axis)
		for(int y=h-2; y>=1; y--)
			i.push_back(hm[0][y].i);

		return i;
	}

//private:
	std::vector<std::vector<ht>> hm;
	int w, h, z;
	bool triangulated, vertexed, centervertexed;
};

void heightmap2mesh(std::string filename, Mesh &mesh)
{
	HeightMap hmp;
	hmp.loadHeightMap(filename);
	HeightMap base = hmp;
	
	
	
	//build heightmap mesh:
	hmp.buildVertices(mesh);
	hmp.fourTriangulate(mesh);
	
	//build base floor mesh:
	base.setHeight(-1);
	base.buildVertices(mesh);
	//base.twoTriangulate(mesh);
	base.centerTriangulate(mesh);
	
	//connect the heightmap and base meshes:
	std::vector<unsigned> he = hmp.getEdgeIndices();
	std::vector<unsigned> be = base.getEdgeIndices();
	
	for(int i=1; i<he.size(); i++) {
		mesh.setTri({he[i], be[i-1], be[i]});
		mesh.setTri({he[i], he[i-1], be[i-1]});
	}
	
	int i = he.size()-1;
	int j = 0;
	
	mesh.setTri({he[i],be[i],be[j]});
	mesh.setTri({he[j],he[i],be[j]});
}

//std::pair<std::vector<vec3f>, std::vector<vec3i>> heightmap2Mesh(std::vector<std::vector<float>> heightmap, int baseheight=1)
std::pair<std::vector<vec3f>, std::vector<vec3i>> heightmap2Mesh(std::string heightmap, int baseheight=1)
{
	Mesh mesh;
	
	HeightMap hmp;
	hmp.loadHeightMap(heightmap);
	HeightMap base = hmp;
	
	//build heightmap mesh:
	hmp.buildVertices(mesh);
	hmp.fourTriangulate(mesh);
	
	//build base floor mesh:
	base.setHeight(-baseheight);
	base.buildVertices(mesh);
	base.centerTriangulate(mesh);
	
	//connect the heightmap and base meshes:
	std::vector<unsigned> he = hmp.getEdgeIndices();
	std::vector<unsigned> be = base.getEdgeIndices();
	
	for(int i=1; i<he.size(); i++) {
		mesh.setTri({he[i], be[i-1], be[i]});
		mesh.setTri({he[i], he[i-1], be[i-1]});
	}
	
	int i = he.size()-1;
	int j = 0;
	
	mesh.setTri({he[i],be[i],be[j]});
	mesh.setTri({he[j],he[i],be[j]});
	
	return std::make_pair(mesh.points, mesh.triangles);
}

