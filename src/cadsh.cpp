#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "manifold/manifold.h"
#include "meshIO.h"
#include "mathparser.h"
//#include "heightmap.h"
#include "manifold_tidbits.h"

//use this to parse out command specs: grep "//cmd" ../src/cadsh.cpp | awk -F'//cmd' '{printf "cout << \"\t"$2 "\" << endl << endl;\n" }'

std::vector<std::string> split(std::string s, std::string delim)
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

void err(std::string msg)
{
	std::cout << msg << std::endl;
	exit(EXIT_FAILURE);
}

double toD(std::string s)
{
	Parser p;
	float a;
	if (!p.parse(s,a))
		err("parse error");
	return (double) a;
}

int toI(std::string s)
{
	Parser p;
	float a;
	if (!p.parse(s,a))
		err("parse error");
	return (int) a;
}

/*
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
*/
/*
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
*/

manifold::SimplePolygon loadpoly(std::string filename)
{
	manifold::SimplePolygon s;
	
	std::ifstream inputFile(filename);
	if (inputFile.is_open()) {
		std::string line;
		while (getline(inputFile, line)) {
			std::vector<std::string> l = split(line, ",");
			if (l.size() < 2) err("malformed polygon");
			s.push_back({atof(l[0].c_str()), atof(l[1].c_str())});
		}
		inputFile.close();
	}
	else err("polygon file load unsuccessful");
	return s;
}

std::vector<std::vector<float>> loadHeightMap(std::string filename)
{
	std::vector<std::vector<float>> hm;
	std::ifstream inputFile(filename);
	if (inputFile.is_open()) {
		std::string line;
		while (getline(inputFile, line)) {
			if (line.size() == 0) continue;
			std::vector<std::string> l = split(line, " ");
			std::vector<float> lt;
			for (auto n : l) 
				lt.push_back(atof(n.c_str()));
			hm.push_back(lt);
		}
		inputFile.close();
	}
	return hm;
}



int main(int argc, char **argv)
{
	std::vector<manifold::Manifold> m;
	bool verbose = false;
	bool all = false;
	
	if (argc <2) {
		std::cout << std::endl << "Usage: cadsh [cmd ...]" << std::endl << std::endl << "Commands:" << std::endl;
		std::cout << " -input/output:" << std::endl;
		std::cout << " --load:filename" << std::endl;
		std::cout << " --save:filename" << std::endl;
		std::cout << " -primitives:" << std::endl;
		std::cout << " --cube:x,y,z[,'ctr']" << std::endl;
		std::cout << " --cylinder:h,rl[,rh[,seg[,'ctr']]]" << std::endl;
		std::cout << " --sphere:r[,seg]" << std::endl;
		std::cout << " --tetrahedron" << std::endl;
		std::cout << " --extrude:polyfilename,height[,div[,twistdeg[,scaletop]]]" << std::endl;
		std::cout << " --revolve:polyfilename,segments,degrees" << std::endl;
		std::cout << " -operators (work on only last mesh):" << std::endl;
		std::cout << " --translate:x,y,z" << std::endl;
		std::cout << " --rotate:x,y,z" << std::endl;
		std::cout << " --scale:s|x,y,z" << std::endl;
		std::cout << " --simplify:s" << std::endl;
		std::cout << " -aggregators:" << std::endl;
		std::cout << " --union" << std::endl;
		std::cout << " --subtract" << std::endl;
		std::cout << " --intersect" << std::endl;
		std::cout << " --hull" << std::endl;

		exit(EXIT_SUCCESS);
	}

	for(int i=1; i<argc; i++) {
		std::string a = std::string(argv[i]);
		std::vector<std::string> t = split(a, ":");
		

		
		//settings
		
		if (t[0] == "verbose") verbose=true;
		
		else if (t[0] == "transform")
			if (t[1] == "all")
				all=true;
			else
				all=false;
		
		
		//cmd -input/output:
		
		else if (t[0] == "load") {  //cmd --load:filename
			if (t.size() >= 2) {
				std::filesystem::path p = std::string(t[1]);
				if (p.extension() == ".3mf") {
					if (verbose) std::cout << "load:" << t[1] << std::endl;
					std::vector<manifold::MeshGL> mm =  ImportMeshes3MF(t[1]);
					for (auto msh : mm)
						m.push_back(manifold::Manifold(msh));
					
				}
				else if (p.extension() == ".stl") {
					if (verbose) std::cout << "load:" << t[1] << std::endl;
					manifold::MeshGL msh = ImportMeshSTL(t[1]);
					if (msh.Merge()) 
						if (verbose) 
							std::cout << "load: STL file fixed" << std::endl;
					manifold::Manifold mm(msh);
					if (mm.Status() != manifold::Manifold::Error::NoError)
						err("load: STL too borked to make a Manifold");
					m.push_back(mm);
					
				}
				else
					std::cout << "invalid filename: " << t[1] << std::endl;
			}
			else err("load: no parameters");
		}
		
		else if (t[0] == "save") {  //cmd --save:filename
			if (t.size() >= 2) {
				std::filesystem::path p = std::string(t[1]);
				if (p.extension() == ".3mf") {
					std::vector<manifold::MeshGL> mshs;
					for (auto mm : m) {
						mshs.push_back(mm.GetMeshGL());
					}
					if (verbose) std::cout << "save:" << t[1] << std::endl;
					ExportMeshes3MF(t[1], mshs);
					
				}
				else
					std::cout << "invalid filename: " << t[1] << std::endl;
			}
			else err("save: no parameters");
		}
		
		else if (t[0] == "info") {
			for (unsigned i=0; i<m.size(); i++)
				std::cout << i << ":" 
					<< " NumVert:" << m[i].NumVert() 
					<< " NumEdge:" << m[i].NumEdge()
					<< " NumTri:" << m[i].NumTri()
					<< " NumProp:" << m[i].NumProp()
					<< " NumPropVert:" << m[i].NumPropVert()
					<< " Genus:" << m[i].Genus()
					<< " Tolerance:" << m[i].GetTolerance()
					<< " Status:" << manifoldError(m[i].Status()) 
					<< std::endl;
		}
		
		
		//cmd -primitives:
		
		else if (t[0] == "cube") {  //cmd --cube:x,y,z[,'ctr']
			if (t.size() >= 2) {
				std::vector<std::string> p = split(t[1], ",");
				double x, y, z;
				bool ctr;
				if (p.size() >= 3) {
					x =toD(p[0]); y = toD(p[1]); z = toD(p[2]);
					
				}
				else err("cube: insufficient parameters");
				if (p.size() >=4) {
					if (p[3] == "ctr")
						ctr = true;
				}
				
				m.push_back(manifold::Manifold::Cube({x,y,z}, ctr));
				if (verbose) std::cout << "cube: " << x << "," << y << "," << z << " " << std::endl;
				//if (verbose) std::cout << "cube: " << x << "," << y << "," << z << " " << manifoldError(m[m.size()-1].Status())  << std::endl;
			}
			else err("cube: no parameters");
		}
		
		else if (t[0] == "cylinder") {  //cmd --cylinder:h,rl[,rh[,seg[,'ctr']]]
			if (t.size() >= 2) {
				std::vector<std::string> p = split(t[1], ",");
				double h, rl, rh=-1.0;
				int seg=0;
				bool ctr=false;
				
				if (p.size() >= 2) {
					h = toD(p[0]); rl = toD(p[1]);
				}
				else err("cylinder: need at least h and rl");
				if (p.size() >= 3) {
					rh = toD(p[2]);
				}
				if (p.size() >= 4) {
					seg = toI(p[3]);
				}
				if (p.size() >= 5) {
					if (p[4] == "center")
						ctr = true;
				}
				m.push_back(manifold::Manifold::Cylinder(h, rl, rh, seg, ctr));
				if (verbose) std::cout << "cylinder: " << h << "," << rl << "," << rh << std::endl;
				//if (verbose) std::cout << "cylinder: " << h << "," << rl << "," << rh << " " << manifoldError(m[m.size()-1].Status()) << std::endl;
			}
			else err("cylinder: no parameters");
		}
		
		else if (t[0] == "sphere") {  //cmd --sphere:r[,seg]
			if (t.size() >= 2) {
				std::vector<std::string> p = split(t[1], ",");
				double r;
				int seg=0;
				if (p.size() >= 1) {
					r = toD(p[0]);
				}
				else err("sphere: needs at least r");
				if (p.size() >= 2) {
					seg = toI(p[1]);
				}
				if (verbose) std::cout << "sphere: " << r << " " << manifoldError(m[m.size()-1].Status()) << std::endl;
				m.push_back(manifold::Manifold::Sphere(r, seg));
				
			}
			else err("sphere: no parameters");
		}
		
		else if (t[0] == "icosahedron") {  //cmd --tetrahedron
			if (verbose) std::cout << "icosahedron: "<< manifoldError(m[m.size()-1].Status()) << std::endl;
			manifold::MeshGL mesh = icosahedron();
			//std::cout << "numPts: " << mesh.NumVert() << "  numTris: " << mesh.NumTri() << std::endl;
			m.push_back(manifold::Manifold(mesh));
			
		}
		
		else if (t[0] == "tetrahedron") {  //cmd --tetrahedron
			if (verbose) std::cout << "tetrahedron: "<< manifoldError(m[m.size()-1].Status()) << std::endl;
			m.push_back(manifold::Manifold::Tetrahedron());
			
		}
		
		else if (t[0] == "extrude") {  //cmd --extrude:polyfilename,height[,div[,twistdeg[,scaletop]]]
			if (t.size() >= 2) {
				std::vector<std::string> p = split(t[1], ",");
				manifold::Polygons pg;
				double h;
				int d=0;
				double t=0.0;
				manifold::vec2 s = {1,1};
		
				if (p.size() >= 2) {
					pg.push_back(loadpoly(p[0]));
					h = toD(p[1]);
				}
				else err("extrude: needs at least polygon and h");
				if (p.size() >= 3) {
					d = toI(p[2]);
				}
				if (p.size() >= 4) {
					t = toI(p[3]);
				}
				if (p.size() >= 5) {
					std::vector<std::string> ss = split(p[4], "|");
					if (ss.size() >= 2) {
						s[0] = toD(ss[0]);
						s[0] = toD(ss[1]);
					}
					else err("extrude: malformed scale");
				}
				if (verbose) std::cout << "extrude: "<< manifoldError(m[m.size()-1].Status()) << std::endl;
				m.push_back(manifold::Manifold::Extrude(pg, h, d, t, s));
				
			}
			else err("extrude: no parameters");
		}
		
		else if (t[0] == "revolve") {  //cmd --revolve:polyfilename,segments,degrees
			if (t.size() >= 2) {
				std::vector<std::string> p = split(t[1], ",");
				manifold::Polygons pg;
				int seg=0;
				double d =360.0;
		
				if (p.size() >= 1) {
					pg.push_back(loadpoly(p[0]));
				}
				else err("revolve: needs at least polygon");
				if (p.size() >= 2) {
					seg = toI(p[1]);
				}
				if (p.size() >= 3) {
					d = toI(p[2]);
				}
				if (verbose) std::cout << "revolve: "<< manifoldError(m[m.size()-1].Status()) << std::endl;
				m.push_back(manifold::Manifold::Revolve(pg, seg, d));
				
			}
			else err("revolve: no parameters");
		}
		
		else if (t[0] == "heightmap") {  //cmd --revolve:polyfilename,segments,degrees
			if (t.size() >= 2) {
				if (verbose) std::cout << "heightmap: "<< manifoldError(m[m.size()-1].Status()) << std::endl;
				std::vector<std::vector<float>> hm = loadHeightMap(t[1]);
				manifold::MeshGL mesh =  heightmap2mesh(hm);
				m.push_back(manifold::Manifold(mesh));
				
			}
			else err("heightmap: no parameters");
		}

		
		//cmd -operators (work on only last mesh):
		
		else if (t[0] == "translate") {  //cmd --translate:x,y,z
			if (t.size() >= 2) {
				std::vector<std::string> p = split(t[1], ",");
				if (p.size() == 3) {
					double x =toD(p[0]); double y = toD(p[1]); double z = toD(p[2]);
					if (verbose) std::cout << "translate: " << x << "," << y << "," << z << std::endl;
					if (all)
						for (auto &mm : m)
							mm = mm.Translate({x,y,z}); 
					else
						m[m.size()-1] = m[m.size()-1].Translate({x,y,z}); 
					
				}
				else err("translate: invalid parameters");
			}
		}
		
		else if (t[0] == "rotate") {  //cmd --rotate:x,y,z
			if (t.size() >= 2) {
				std::vector<std::string> p = split(t[1], ",");
				if (p.size() == 3) {
					double x =toD(p[0]); double y = toD(p[1]); double z = toD(p[2]);
					if (verbose) std::cout << "rotate: " << x << "," << y << "," << z << std::endl;
					if (all)
						for (auto &mm : m)
							mm = mm.Rotate(x,y,z); 
					else
						m[m.size()-1] = m[m.size()-1].Rotate(x,y,z);
					
				}
				else err("rotate: invalid parameters");
			}
		}
		
		else if (t[0] == "scale") { //cmd --scale:s|x,y,z
			manifold::vec3 s;
			if (t.size() >= 2) {
				if (verbose) std::cout << "scale: " << s.x << "," << s.y << "," << s.z << std::endl;
				std::vector<std::string> p = split(t[1], ",");
				if (p.size() == 1) {
					s[0] = s[1] =s[2] = toD(t[1]);
				}
				else if (p.size() == 3) {
					s.x =toD(p[0]); s.y = toD(p[1]); s.z = toD(p[2]);
				}
				else {
					err("scale: malformed parameters");
				}
			}
			else err("scale: no parameters");
			
			if (all)
				for (auto &mm : m)
					mm = mm.Scale(s); 
			else
				m[m.size()-1] = m[m.size()-1].Scale(s);
			
			
		}
		
		else if (t[0] == "simplify") { //cmd --simplify:s
			if (t.size() == 2) {
				double s = toD(t[1]);
				if (verbose) std::cout << "simplify: " << s << "...";
				int before = m[m.size()-1].NumTri();
				m[m.size()-1] = m[m.size()-1].Simplify(s);
				int after = m[m.size()-1].NumTri();
				if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
			}
			else err("simplify: no parameter");
		}
		
		else if (t[0] == "refine") { //cmd --refine:n
			if (t.size() == 2) {
				int n = toI(t[1]);
				if (verbose) std::cout << "refine: " << n << "...";
				int before = m[m.size()-1].NumTri();
				m[m.size()-1] = m[m.size()-1].Refine(n);
				int after = m[m.size()-1].NumTri();
				if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
			}
			else err("refine: no parameter");
		}
		
		else if (t[0] == "refinetolength") { //cmd --refinetolength:l
			if (t.size() == 2) {
				int before = m[m.size()-1].NumTri();
				double l = toD(t[1]);
				if (verbose) std::cout << "refinetolength: " << l << "...";
				m[m.size()-1] = m[m.size()-1].RefineToLength(l);
				int after = m[m.size()-1].NumTri();
				if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
			}
			else err("refinetolength: no parameter");
		}
		
		else if (t[0] == "refinetotolerance") { //cmd --refinetotolerance:t
			if (t.size() == 2) {
				int before = m[m.size()-1].NumTri();
				double tl = toD(t[1]);
				if (verbose) std::cout << "refinetotolerance: " << tl << "...";
				m[m.size()-1] = m[m.size()-1].RefineToTolerance(tl);
				int after = m[m.size()-1].NumTri();
				if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
			}
			else err("refinetotolerance: no parameter");
		}
		
		else if (t[0] == "smoothout") { //cmd --smoothout:[msa[,ms]]
			
			double msa=60.0;
			double ms=0;
			if (t.size() == 2) {
				std::vector<std::string> p = split(t[1], ",");
				
				if (p.size() >= 1) {
					msa = toD(p[0]);
				}
				if (p.size() >= 2) {
					ms = toD(p[1]);
				}
			}
			if (verbose) std::cout << "smoothout: " << msa << "," << ms << "...";
			int before = m[m.size()-1].NumTri();
			m[m.size()-1] = m[m.size()-1].SmoothOut(msa, ms);
			int after = m[m.size()-1].NumTri();
			if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
		}
		
		
		//cmd -aggregators:
		
		else if (t[0] == "union") { //cmd --union
			if (verbose) std::cout << "union" << std::endl;
			manifold::Manifold u = manifold::Manifold::BatchBoolean(m, manifold::OpType::Add);
			m.clear();
			m.push_back(u);
			
		}
		
		else if (t[0] == "subtract") { //cmd --subtract
			manifold::Manifold s = manifold::Manifold::BatchBoolean(m, manifold::OpType::Subtract);
			m.clear();
			m.push_back(s);
			if (verbose) std::cout << "subtract" << std::endl;
		}
		
		else if (t[0] == "intersect") { //cmd --intersect
			manifold::Manifold i = manifold::Manifold::BatchBoolean(m, manifold::OpType::Intersect);
			m.clear();
			m.push_back(i);
			if (verbose) std::cout << "intersect" << std::endl;
		}
		
		else if (t[0] == "hull") { //cmd --hull
			manifold::Manifold u = manifold::Manifold::Hull(m);
			m.clear();
			m.push_back(u);
			if (verbose) std::cout << "hull" << std::endl;
		}
		else err("Unrecognized command: "+t[0]);
	}
	
	exit(EXIT_SUCCESS);
}