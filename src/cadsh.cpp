#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

#include "manifold/manifold.h"
#include "meshIO.h"


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

double toD(std::string s)
{
	return (double) atof(s.c_str());
}

double toI(std::string s)
{
	return atoi(s.c_str());
}

void err(std::string msg)
{
	std::cout << msg << std::endl;
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	std::vector<manifold::Manifold> m;

	for(int i=1; i<argc; i++) {
		std::string a = std::string(argv[i]);
		std::vector<std::string> t = split(a, ":");
		
		//input/output:
		
		if (t[0] == "load") {
			std::filesystem::path p = std::string(t[1]);
			if (p.extension() == ".3mf") {
				std::vector<manifold::MeshGL> mm =  ImportMeshes3MF(t[1]);
				for (auto msh : mm)
					m.push_back(manifold::Manifold(msh));
			}
			else
				std::cout << "invalid filename: " << t[1] << std::endl;
		}
		
		if (t[0] == "save") {
			std::filesystem::path p = std::string(t[1]);
			if (p.extension() == ".3mf") {
				std::vector<manifold::MeshGL> mshs;
				for (auto mm : m) {
					mshs.push_back(mm.GetMeshGL());
				}
				ExportMeshes3MF(t[1], mshs);
			}
			else
				std::cout << "invalid filename: " << t[1] << std::endl;
		}
		
		
		//primitives:
		
		else if (t[0] == "cube") {  //cube:x,y,z
			if (t.size() >= 2) {
				std::vector<std::string> p = split(t[1], ",");
				if (p.size() == 3) {
					double x =toD(p[0]); double y = toD(p[1]); double z = toD(p[2]);
					m.push_back(manifold::Manifold::Cube({x,y,z}));
				}
				else err("cube: invalid parameters");
			}
			else err("cube: no parameters");
		}
		
		else if (t[0] == "cylinder") {  //cylinder:h,rl[,rh[,seg[,ctr]]]
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
					if (p[4] == "true")
						ctr = true;
					else if (p[4] == "false")
						ctr = false;
				}
				m.push_back(manifold::Manifold::Cylinder(h, rl, rh, seg, ctr));
			}
			else err("cylinder: no parameters");
		}
		
		
		//operators:
		
		else if (t[0] == "translate") {
			if (t.size() >= 3) {
				std::vector<std::string> p = split(t[1], ",");
				if (p.size() == 3) {
					double x =toD(p[0]); double y = toD(p[1]); double z = toD(p[2]);
					for (auto &mm : m) 
						mm = mm.Translate({x,y,z});
				}
				else err("translate: invalid parameters");
			}
		}
		
		else if (t[0] == "rotate") {
			if (t.size() >= 3) {
				std::vector<std::string> p = split(t[1], ",");
				if (p.size() == 3) {
					double x =toD(p[0]); double y = toD(p[1]); double z = toD(p[2]);
					for (auto &mm : m) 
						mm = mm.Rotate(x,y,z);
				}
				else err("rotate: invalid parameters");
			}
		}
		
		else if (t[0] == "scale") { //scale:s|(x,y,z)
			if (t.size() == 2) {
				double s = toD(t[1]);
				for (auto &mm : m) 
					mm = mm.Scale({s,s,s});
			}
			else if (t.size() >= 3) {
				std::vector<std::string> p = split(t[1], ",");
				if (p.size() == 3) {
					double x =toD(p[0]); double y = toD(p[1]); double z = toD(p[2]);
					for (auto &mm : m) 
						mm = mm.Scale({x,y,z});
				}
				else err("translate: invalid parameters");
			}
		}
		
		else if (t[0] == "simplify") { //simplify:s
			if (t.size() == 2) {
				double s = toD(t[1]);
				for (auto &mm : m) 
					mm = mm.Simplify(s);
			}
			else err("translate: no parameter");
		}
	}
	
	exit(EXIT_SUCCESS);
}