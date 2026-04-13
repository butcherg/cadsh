#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "manifold/manifold.h"
//#include "meshIO.h"
#include "manifoldIO.h"
#include "mathparser.h"
//#include "heightmap.h"
#include "manifold_tidbits.h"

static std::vector<manifold::Manifold> m;
static bool verbose = false;
static bool all = false;

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
	} else err("loadHeightMap: file open failed");
	return hm;
}


std::string executeParameter(std::string parameter) 
{
	std::vector<std::string> t = split(parameter, ":");
	
	 if (t[0] == "help") {  //cmd --load:filename
		std::cout << std::endl << "Usage: cadsh [cmd ...]" << std::endl << std::endl << "Commands:" << std::endl;
		std::cout << " -input/output:" << std::endl;
		std::cout << "   --load:filename" << std::endl;
		std::cout << "   --save:filename" << std::endl;
		std::cout << " -primitives:" << std::endl;
		std::cout << "   --cube:x,y,z[,'ctr']" << std::endl;
		std::cout << "   --cylinder:h,rl[,rh[,seg[,'ctr']]]" << std::endl;
		std::cout << "   --sphere:r[,seg]" << std::endl;
		std::cout << "   --tetrahedron" << std::endl;
		std::cout << "   --extrude:polyfilename,height[,div[,twistdeg[,scaletop]]]" << std::endl;
		std::cout << "   --revolve:polyfilename,segments,degrees" << std::endl;
		std::cout << " -operators:" << std::endl;
		std::cout << "   --translate:x,y,z" << std::endl;
		std::cout << "   --rotate:x,y,z" << std::endl;
		std::cout << "   --scale:s|x,y,z" << std::endl;
		std::cout << "   --simplify:s" << std::endl;
		std::cout << " -aggregators:" << std::endl;
		std::cout << "   --union" << std::endl;
		std::cout << "   --subtract" << std::endl;
		std::cout << "   --intersect" << std::endl;
		std::cout << "   --hull" << std::endl;
		std::cout << " -helpers:" << std::endl;
		std::cout << "   --help" << std::endl;
		std::cout << "   --status" << std::endl;
		std::cout << "   --verbose" << std::endl;
		std::cout << "   --transform:all|last" << std::endl;
	 }

	//settings
		
	else if (t[0] == "verbose") verbose=true;
		
	else if (t[0] == "transform") {
		if (t.size() >= 2) {
			if (t[1] == "all")
				all=true;
			else
				all=false;
		}
		else return "transform: no parameter";
	}
		
	//cmd -input/output:
	
	else if (t[0] == "clear") { 
		int msize = m.size();
		if (verbose) {
			if (msize == 1)
				std::cout << "clear:" << m.size() << " mesh" << std::endl;
			else
				std::cout << "clear:" << m.size() << " meshes" << std::endl;
		}
		m.clear();
	}
		
	else if (t[0] == "load") {  //cmd --load:filename
		if (t.size() >= 2) {
			std::filesystem::path p = std::string(t[1]);
			if (p.extension() == ".3mf") {
				std::vector<manifold::Manifold> mm = ImportMeshes3MF(t[1]);
				int count = 0;
				for (auto msh : mm) {
					m.push_back(msh);
					count++;
				}
				if (verbose) std::cout << "load:" << t[1] << ", " << count << " meshes" << std::endl;
			}
			else if (p.extension() == ".stl") {
				manifold::MeshGL msh = ImportMeshSTL(t[1]);
				if (msh.Merge()) 
					if (verbose) 
						std::cout << "load: STL file fixed" << std::endl;
				manifold::Manifold mm(msh);
				if (mm.Status() != manifold::Manifold::Error::NoError)
					return "load: STL too borked to make a Manifold";
				m.push_back(mm);
				if (verbose) std::cout << "load:" << t[1] << ", " << m.size() << " meshes" << std::endl;
			}
			else
				std::cout << "invalid filename: " << t[1] << std::endl;
		}
		else return "load: no parameters";
	}
		
	else if (t[0] == "save") {  //cmd --save:filename
		if (t.size() >= 2) {
			std::filesystem::path p = std::string(t[1]);
			if (p.extension() == ".3mf") {
				if (verbose) std::cout << "save:" << t[1] << std::endl;
				ExportMeshes3MF(t[1], m);
				
			}
			else
				std::cout << "invalid filename: " << t[1] << std::endl;
		}
		else return "save: no parameters";
	}
	
	else if (t[0] == "status") {
		if (m.size() == 1)
			std::cout << m.size() << " mesh" << std::endl;
		else
			std::cout << m.size() << " meshes" << std::endl;
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
		
	else if (t[0] == "calculatenormals") {
			
		if (all) {
			if (verbose) std::cout << "calculatenormals, " << m.size() << " meshes" << std::endl;
			for (auto &mm : m)
				mm = mm.CalculateNormals(0); 
		}
		else {
			if (verbose) std::cout << "calculatenormals, last mesh" << std::endl;
			m[m.size()-1] = m[m.size()-1].CalculateNormals(0);
		}
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
			else return "cube: insufficient parameters";
			if (p.size() >=4) {
				if (p[3] == "ctr")
					ctr = true;
			}
			
			m.push_back(manifold::Manifold::Cube({x,y,z}, ctr));
			if (verbose) std::cout << "cube: " << x << "," << y << "," << z << " " << std::endl;
			//if (verbose) std::cout << "cube: " << x << "," << y << "," << z << " " << manifoldError(m[m.size()-1].Status())  << td::endl;
		}
		else return "cube: no parameters";
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
			else return "cylinder: need at least h and rl";
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
		else return "cylinder: no parameters";
	}
		
	else if (t[0] == "sphere") {  //cmd --sphere:r[,seg]
		if (t.size() >= 2) {
			std::vector<std::string> p = split(t[1], ",");
			double r;
			int seg=0;
			if (p.size() >= 1) {
				r = toD(p[0]);
			}
			else return "sphere: needs at least r";
			if (p.size() >= 2) {
				seg = toI(p[1]);
			}
			m.push_back(manifold::Manifold::Sphere(r, seg));
			if (verbose) std::cout << "sphere: " << r << " " << manifoldError(m[m.size()-1].Status()) << std::endl;
		}
		else return "sphere: no parameters";
	}

	else if (t[0] == "icosahedron") {  //cmd --tetrahedron
		manifold::MeshGL mesh = icosahedron();
		//std::cout << "numPts: " << mesh.NumVert() << "  numTris: " << mesh.NumTri() << std::endl;
		m.push_back(manifold::Manifold(mesh));
		if (verbose) std::cout << "icosahedron: "<< manifoldError(m[m.size()-1].Status()) << std::endl;
	}
		
	else if (t[0] == "tetrahedron") {  //cmd --tetrahedron
		m.push_back(manifold::Manifold::Tetrahedron());
		if (verbose) std::cout << "tetrahedron: "<< manifoldError(m[m.size()-1].Status()) << std::endl;
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
			else return "extrude: needs at least polygon and h";
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
				else return "extrude: malformed scale";
			}
			
			m.push_back(manifold::Manifold::Extrude(pg, h, d, t, s));
			if (verbose) std::cout << "extrude: "<< manifoldError(m[m.size()-1].Status()) << std::endl;
				
		}
		else return "extrude: no parameters";
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
			else return "revolve: needs at least polygon";
			if (p.size() >= 2) {
				seg = toI(p[1]);
			}
			if (p.size() >= 3) {
				d = toI(p[2]);
			}
			
			m.push_back(manifold::Manifold::Revolve(pg, seg, d));
			if (verbose) std::cout << "revolve: "<< manifoldError(m[m.size()-1].Status()) << std::endl;
				
		}
		else return "revolve: no parameters";
	}
		
	else if (t[0] == "heightmap") {  //cmd --heightmap:heightmapfile[height[contour]]
		if (t.size() >= 2) {
			std::vector<std::string> p = split(t[1], ",");
			std::string filename;
			float height = -1.0;
			bool contour = false;
			if (p.size() >= 1) {
				filename = p[0];
			}
			else return "heightmap: needs at least a heightmap file";
			if (p.size() >= 2) {
				height = toD(p[1]);
			}
			if (p.size() >= 3) {
				if (p[2] == "true")
					contour = true;
				else if (p[2] == "false")
					contour = false;
				else return "heightmap: contour paramter not valid: "+p[2];
			}
			
			std::vector<std::vector<float>> hm = loadHeightMap(filename);
			manifold::MeshGL mesh =  heightmap2mesh(hm, height, contour);
			//ExportMeshGL3MF("test.3mf", mesh);
			m.push_back(manifold::Manifold(mesh));
			if (verbose) std::cout << "heightmap: "<< manifoldError(m[m.size()-1].Status()) << std::endl;
		}
		else return "heightmap: no parameters";
	}
		
		
	//cmd -operators (work on only last mesh):
		
	else if (t[0] == "translate") {  //cmd --translate:x,y,z
		if (t.size() >= 2) {
			std::vector<std::string> p = split(t[1], ",");
			if (p.size() == 3) {
				double x =toD(p[0]); double y = toD(p[1]); double z = toD(p[2]);
				if (all) {
					if (verbose) std::cout << "translate(all): " << x << "," << y << "," << z << std::endl;
					for (auto &mm : m)
						mm = mm.Translate({x,y,z}); 
				}
				else {
					if (verbose) std::cout << "translate(last): " << x << "," << y << "," << z << std::endl;
					m[m.size()-1] = m[m.size()-1].Translate({x,y,z}); 
				}
				
			}
			else return "translate: invalid parameters";
		}
	}
		
	else if (t[0] == "rotate") {  //cmd --rotate:x,y,z
		if (t.size() >= 2) {
			std::vector<std::string> p = split(t[1], ",");
			if (p.size() == 3) {
				double x =toD(p[0]); double y = toD(p[1]); double z = toD(p[2]);
				if (all) {
					if (verbose) std::cout << "rotate(all): " << x << "," << y << "," << z << std::endl;
					for (auto &mm : m)
						mm = mm.Rotate(x,y,z); 
				}
				else {
					if (verbose) std::cout << "rotate(last): " << x << "," << y << "," << z << std::endl;
					m[m.size()-1] = m[m.size()-1].Rotate(x,y,z);
				}
			}
			else return "rotate: invalid parameters";
		}
	}
		
	else if (t[0] == "scale") { //cmd --scale:s|x,y,z
		manifold::vec3 s;
		if (t.size() >= 2) {
				
			std::vector<std::string> p = split(t[1], ",");
			if (p.size() == 1) {
				s[0] = s[1] =s[2] = toD(t[1]);
			}
			else if (p.size() == 3) {
				s.x =toD(p[0]); s.y = toD(p[1]); s.z = toD(p[2]);
			}
			else {
				return "scale: malformed parameters";
			}
		}
		else return "scale: no parameters";
			
		if (all) {
			if (verbose) std::cout << "scale (all): " << s.x << "," << s.y << "," << s.z << std::endl;
			for (auto &mm : m)
				mm = mm.Scale(s); 
		}
		else {
			if (verbose) std::cout << "scale (last): " << s.x << "," << s.y << "," << s.z << std::endl;
			m[m.size()-1] = m[m.size()-1].Scale(s);
		}
			
	}
		
	else if (t[0] == "simplify") { //cmd --simplify:s
		if (t.size() == 2) {
			double s = toD(t[1]);
			if (all) {
				if (verbose) std::cout << "simplify(all): " << s << "..." << std::endl;
				for (auto &mm : m) {
					int before = mm.NumTri();
					mm = mm.Simplify(s);
					int after = mm.NumTri();
					if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
				}
			}
			else {
				if (verbose) std::cout << "simplify(last): " << s << "...";
				int before = m[m.size()-1].NumTri();
				m[m.size()-1] = m[m.size()-1].Simplify(s);
				int after = m[m.size()-1].NumTri();
				if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
			}
		}
		else return "simplify: no parameter";
	}
		
	else if (t[0] == "refine") { //cmd --refine:n
		if (t.size() == 2) {
			int n = toI(t[1]);
			if (all) {
				if (verbose) std::cout << "refine(all): " << n << "..." << std::endl;
				for (auto &mm : m) {
					int before = mm.NumTri();
					mm = mm.Refine(n);
					int after = mm.NumTri();
					if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
				}
			}
			else {
				if (verbose) std::cout << "refine(last): " << n << "...";
				int before = m[m.size()-1].NumTri();
				m[m.size()-1] = m[m.size()-1].Refine(n);
				int after = m[m.size()-1].NumTri();
				if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
			}
		}
		else return "refine: no parameter";
	}
		
	else if (t[0] == "refinetolength") { //cmd --refinetolength:l
		if (t.size() == 2) {
			double l = toD(t[1]);
			if (all) {
				if (verbose) std::cout << "refinetolength(all): " << l << "..." << std::endl;
				for (auto &mm : m) {
					int before = mm.NumTri();
					mm = mm.RefineToLength(l);
					int after = mm.NumTri();
					if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
				}
			}
			else {
				if (verbose) std::cout << "refinetolength(last): " << l << "...";
				int before = m[m.size()-1].NumTri();
				m[m.size()-1] = m[m.size()-1].RefineToLength(l);
				int after = m[m.size()-1].NumTri();
				if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
			}
		}
		else return "refinetolength: no parameter";
	}
		
	else if (t[0] == "refinetotolerance") { //cmd --refinetotolerance:t
		if (t.size() == 2) {
			double tl = toD(t[1]);
			if (all) {
				if (verbose) std::cout << "refinetotolerance(all): " << tl << "..." << std::endl;
				for (auto &mm : m) {
					int before = mm.NumTri();
					mm = mm.RefineToTolerance(tl);
					int after = mm.NumTri();
					if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
				}
			}
			else {
				if (verbose) std::cout << "refinetotolerance(last): " << tl << "...";
				int before = m[m.size()-1].NumTri();
				m[m.size()-1] = m[m.size()-1].RefineToTolerance(tl);
				int after = m[m.size()-1].NumTri();
				if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
			}
		}
		else return "refinetotolerance: no parameter";
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
		if (all) {
			if (verbose) std::cout << "smoothout(all): " << msa << "," << ms << "..." << std::endl;
			for (auto &mm : m) {
				int before = mm.NumTri();
				mm = mm.SmoothOut(msa, ms);
				int after = mm.NumTri();
				if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
			}
		}
		else {
			if (verbose) std::cout << "smoothout(last): " << msa << "," << ms << "...";
			int before = m[m.size()-1].NumTri();
			m[m.size()-1] = m[m.size()-1].SmoothOut(msa, ms);
			int after = m[m.size()-1].NumTri();
			if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
		}
	}
		
	else if (t[0] == "smoothbynormals") { //cmd --smoothbynormals
		double msa=60.0;
		double ms=0;
		if (t.size() >= 1) {
			if (verbose) std::cout << "smoothbynormals... " ;
			int before = m[m.size()-1].NumTri();
			m[m.size()-1] = m[m.size()-1].SmoothByNormals(0);
			int after = m[m.size()-1].NumTri();
			if (verbose) std::cout << " (triangles: " << before << "/" << after << ")" << std::endl;
		}
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
	else return "Unrecognized command: "+t[0];
	
	return "";
}



int main(int argc, char **argv)
{
	if(argc == 1) {
		std::cout << "shell mode..." << std::endl;
		std::string param;
		while (1) {
			std::cout << "> ";
			std::getline(std::cin, param);
			std::string result = executeParameter(param);
			if (result.size() > 0) std::cout << executeParameter(param) << std::endl;
		}
	}
	
	else if (argc == 2 && std::filesystem::exists(std::string(argv[1]))) {
		std::cout << "script mode..." << std::endl;
		std::string fname = argv[1];
		std::string param; 
		std::ifstream file(fname);
		if (!file.is_open()) err("File open failed: " + fname);
		while (std::getline(file, param)) {
			std::vector<std::string> l = split(param, "#");  //parse out comments
			if (l.size() >= 1 && l[0].size() > 0) {
				std::string result = executeParameter(l[0]);
				if (result.size() > 0) err(result);
			}
		}
		file.close();
	}

	else {
		std::cout << "command-line mode..." << std::endl;
		for(int i=1; i<argc; i++) {
			std::string param = std::string(argv[i]);
			std::string result = executeParameter(param);
			if (result.size() > 0) err(result);
		}
	}
	
	exit(EXIT_SUCCESS);
}