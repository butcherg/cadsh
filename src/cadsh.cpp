#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "manifold/manifold.h"
#include "meshIO.h"
#include "mathparser.h"


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



int main(int argc, char **argv)
{
	std::vector<manifold::Manifold> m;
	bool verbose = false;

	for(int i=1; i<argc; i++) {
		std::string a = std::string(argv[i]);
		std::vector<std::string> t = split(a, ":");
		
		//settings
		
		if (t[0] == "verbose") verbose=true;
		
		//input/output:
		
		else if (t[0] == "load") {  //cmd load:filename
			if (t.size() >= 2) {
				std::filesystem::path p = std::string(t[1]);
				if (p.extension() == ".3mf") {
					std::vector<manifold::MeshGL> mm =  ImportMeshes3MF(t[1]);
					for (auto msh : mm)
						m.push_back(manifold::Manifold(msh));
					if (verbose) std::cout << "load:" << t[1] << std::endl;
				}
				else
					std::cout << "invalid filename: " << t[1] << std::endl;
			}
			else err("load: no parameters");
		}
		
		else if (t[0] == "save") {
			if (t.size() >= 2) {
				std::filesystem::path p = std::string(t[1]);
				if (p.extension() == ".3mf") {
					std::vector<manifold::MeshGL> mshs;
					for (auto mm : m) {
						mshs.push_back(mm.GetMeshGL());
					}
					ExportMeshes3MF(t[1], mshs);
					if (verbose) std::cout << "save:" << t[1] << std::endl;
				}
				else
					std::cout << "invalid filename: " << t[1] << std::endl;
			}
			else err("save: no parameters");
		}
		
		
		//primitives:
		
		else if (t[0] == "cube") {  //cmd cube:x,y,z[,'ctr']
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
				if (verbose) std::cout << "cube: " << x << "," << y << "," << z << std::endl;
			}
			else err("cube: no parameters");
		}
		
		else if (t[0] == "cylinder") {  //cmd cylinder:h,rl[,rh[,seg[,'ctr']]]
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
			}
			else err("cylinder: no parameters");
		}
		
		else if (t[0] == "sphere") {  //cmd sphere:r[,seg]
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
				m.push_back(manifold::Manifold::Sphere(r, seg));
				if (verbose) std::cout << "sphere: " << r << std::endl;
			}
			else err("sphere: no parameters");
		}
		
		else if (t[0] == "tetrahedron") {  //cmd tetrahedron
			m.push_back(manifold::Manifold::Tetrahedron());
			if (verbose) std::cout << "tetrahedron" << std::endl;
		}
		
		else if (t[0] == "extrude") {  //cmd extrude:polyfilename,height[,div[,twistdeg[,scaletop]]]
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
				
				m.push_back(manifold::Manifold::Extrude(pg, h, d, t, s));
				if (verbose) std::cout << "extrude" << std::endl;
			}
			else err("extrude: no parameters");
		}
		
		else if (t[0] == "revolve") {  //cmd revolve:polyfilename,segments,degrees
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
				
				m.push_back(manifold::Manifold::Revolve(pg, seg, d));
				if (verbose) std::cout << "revolve" << std::endl;
			}
			else err("revolve: no parameters");
		}
		
		
		//operators (work on only last mesh):
		
		else if (t[0] == "translate") {  //cmd translate:x,y,z
			if (t.size() >= 2) {
				std::vector<std::string> p = split(t[1], ",");
				if (p.size() == 3) {
					double x =toD(p[0]); double y = toD(p[1]); double z = toD(p[2]);
					m[m.size()-1] = m[m.size()-1].Translate({x,y,z}); 
					if (verbose) std::cout << "translate: " << x << "," << y << "," << z << std::endl;
				}
				else err("translate: invalid parameters");
			}
		}
		
		else if (t[0] == "rotate") {  //cmd rotate:x,y,z
			if (t.size() >= 2) {
				std::vector<std::string> p = split(t[1], ",");
				if (p.size() == 3) {
					double x =toD(p[0]); double y = toD(p[1]); double z = toD(p[2]);
					m[m.size()-1] = m[m.size()-1].Rotate(x,y,z);
					if (verbose) std::cout << "rotate: " << x << "," << y << "," << z << std::endl;
				}
				else err("rotate: invalid parameters");
			}
		}
		
		else if (t[0] == "scale") { //cmd scale:s|x,y,z
			if (t.size() == 2) {
				double s = toD(t[1]);
				m[m.size()-1] = m[m.size()-1].Scale({s,s,s});
				if (verbose) std::cout << "scale: " << s << std::endl;
			}
			else if (t.size() >= 3) {
				std::vector<std::string> p = split(t[1], ",");
				if (p.size() == 3) {
					double x =toD(p[0]); double y = toD(p[1]); double z = toD(p[2]);
					m[m.size()-1] = m[m.size()-1].Scale({x,y,z});
					if (verbose) std::cout << "scale: " << x << "," << y << "," << z << std::endl;
				}
				else err("translate: invalid parameters");
			}
		}
		
		else if (t[0] == "simplify") { //cmd simplify:s
			if (t.size() == 2) {
				double s = toD(t[1]);
				int before = m[m.size()-1].NumTri();
				m[m.size()-1] = m[m.size()-1].Simplify(s);
				int after = m[m.size()-1].NumTri();
				if (verbose) std::cout << "simplify: " << s << " (triangles: " << before << "/" << after << ")" << std::endl;
			}
			else err("translate: no parameter");
		}
		
		else if (t[0] == "union") { //cmd union
			manifold::Manifold u = manifold::Manifold::BatchBoolean(m, manifold::OpType::Add);
			m.clear();
			m.push_back(u);
			if (verbose) std::cout << "union" << std::endl;
		}
		
		else if (t[0] == "subtract") { //cmd subtract
			manifold::Manifold s = manifold::Manifold::BatchBoolean(m, manifold::OpType::Subtract);
			m.clear();
			m.push_back(s);
			if (verbose) std::cout << "subtract" << std::endl;
		}
		
		else if (t[0] == "intersect") { //cmd intersect
			manifold::Manifold i = manifold::Manifold::BatchBoolean(m, manifold::OpType::Intersect);
			m.clear();
			m.push_back(i);
			if (verbose) std::cout << "intersect" << std::endl;
		}
		
		else if (t[0] == "hull") { //cmd hull
			manifold::Manifold u = manifold::Manifold::Hull(m);
			m.clear();
			m.push_back(u);
			if (verbose) std::cout << "hull" << std::endl;
		}
	}
	
	exit(EXIT_SUCCESS);
}