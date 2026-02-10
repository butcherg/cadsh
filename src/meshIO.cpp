

#include "manifold/meshIO.h"

#include "miniz.h"
#include "rapidxml.hpp"

#include <iostream>
#include <fstream>
//#include <ios>
#include <sstream>
#include <iomanip>  // std::setiosflags, std::setprecision

#include <map>


//3MF functions:

manifold::MeshGL ImportMesh3MF(const std::string& filename)
{
	manifold::MeshGL mesh;

	mz_zip_archive zip_archive;
    mz_bool status;
    int i;
    int file_index = -1;
    const char* model_file_name = "3D/3dmodel.model"; // Standard path in 3MF

    // 1. Initialize the zip archive struct
    memset(&zip_archive, 0, sizeof(zip_archive));

    // 2. Open the 3MF file (as a zip) for reading
    status = mz_zip_reader_init_file(&zip_archive, filename.c_str(), 0);
    if (!status) {
		return mesh;
	}

    // 3. Search for the main model file within the archive
    for (i = 0; i < (int)mz_zip_reader_get_num_files(&zip_archive); ++i) {
        char current_filename[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
		if (mz_zip_reader_get_filename(&zip_archive, i, current_filename, sizeof(current_filename))) {

            if (strcmp(model_file_name, current_filename) == 0) {
                file_index = i;
                break;
            }
        }
    }

    if (file_index == -1) {
        mz_zip_reader_end(&zip_archive);
        return mesh;
    }

    // 4. Extract the file content into a buffer
    size_t uncomp_size;
    void* uncomp_data = mz_zip_reader_extract_to_heap(&zip_archive, file_index, &uncomp_size, 0);
    if (!uncomp_data) {
        mz_zip_reader_end(&zip_archive);
        return mesh;
    }

    //printf("Successfully extracted '%s'. Size: %zu bytes.\n", model_file_name, uncomp_size);
	//std::cout << std::string((char *) uncomp_data) << std::endl;
	//fflush(stdout);

	rapidxml::xml_document<> doc;    // character type defaults to char
	doc.parse<0>((char *) uncomp_data);    // 0 means default parse flags
	
	rapidxml::xml_node<>* root_node = doc.first_node("model");
	
	if (root_node) {
		
		rapidxml::xml_node<>* vertices = doc.first_node("model")->first_node("resources")->first_node("object")->first_node("mesh")->first_node("vertices");
		
		rapidxml::xml_node<>* vert = vertices->first_node();
		int vcount = 0;
		while(vert) {
			float v[3];
			v[0] = atof(vert->first_attribute("x")->value()); 
			v[1] = atof(vert->first_attribute("y")->value()); 
			v[2] = atof(vert->first_attribute("z")->value()); 
			mesh.vertProperties.insert(mesh.vertProperties.end(), {v[0], v[1], v[2]});
			vcount++;
			vert = vert->next_sibling();
		}
		//printf("Vertices: %d\n", vcount);

		rapidxml::xml_node<>* triangles = doc.first_node("model")->first_node("resources")->first_node("object")->first_node("mesh")->first_node("triangles");
	
		rapidxml::xml_node<>* tri = triangles->first_node();
		int tcount = 0;
		while(tri) {
			uint32_t t[3];
			t[0] = atoi(tri->first_attribute("v1")->value()); 
			t[1] = atoi(tri->first_attribute("v2")->value()); 
			t[2] = atoi(tri->first_attribute("v3")->value()); 
			mesh.triVerts.insert(mesh.triVerts.end(), {t[0], t[1], t[2]});
			tcount++;
			tri = tri->next_sibling();
		}
		//printf("Faces: %d\n", tcount);
		fflush(stdout);
	}

    mz_free(uncomp_data);
    mz_zip_reader_end(&zip_archive);

	return mesh;
}

std::vector<manifold::MeshGL> ImportMeshes3MF(const std::string& filename)
{
	std::vector<manifold::MeshGL> meshes;

	mz_zip_archive zip_archive;
    mz_bool status;
    int i;
    int file_index = -1;
    const char* model_file_name = "3D/3dmodel.model"; // Standard path in 3MF

    // 1. Initialize the zip archive struct
    memset(&zip_archive, 0, sizeof(zip_archive));

    // 2. Open the 3MF file (as a zip) for reading
    status = mz_zip_reader_init_file(&zip_archive, filename.c_str(), 0);
    if (!status) {
		return meshes;
	}

    // 3. Search for the main model file within the archive
    for (i = 0; i < (int)mz_zip_reader_get_num_files(&zip_archive); ++i) {
        char current_filename[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
		if (mz_zip_reader_get_filename(&zip_archive, i, current_filename, sizeof(current_filename))) {

            if (strcmp(model_file_name, current_filename) == 0) {
                file_index = i;
                break;
            }
        }
    }

    if (file_index == -1) {
        mz_zip_reader_end(&zip_archive);
        return meshes;
    }

    // 4. Extract the file content into a buffer
    size_t uncomp_size;
    void* uncomp_data = mz_zip_reader_extract_to_heap(&zip_archive, file_index, &uncomp_size, 0);
    if (!uncomp_data) {
        mz_zip_reader_end(&zip_archive);
        return meshes;
    }
	
	//rapidxml seems to add non printable characters to XML string, sometimes...
	std::string uc = std::string((char *) uncomp_data);
	if (uc.size() > uncomp_size)
		uc.resize(uncomp_size);

    //printf("Successfully extracted '%s'. Size: %zu bytes.\n", model_file_name, uncomp_size);
	//std::string uc = std::string((char *) uncomp_data);
	//std::cout << uc << std::endl << uc.size() << " characters" << std::endl;
	//std::cout << std::string((char *) uncomp_data) << std::endl << "end of text, " << std::string((char *) uncomp_data).size() << " characters." << std::endl;
	//std::cout << uncomp_data << std::endl << "end of text." << std::endl;
	//fflush(stdout);

	rapidxml::xml_document<> doc;    // character type defaults to char
	//doc.parse<0>((char *) uncomp_data);    // 0 means default parse flags
	doc.parse<0>((char *) uc.c_str());  // use resized string, if it was bigger than uncomp_size
	
	rapidxml::xml_node<>* root_node = doc.first_node("model");
	
	if (root_node) {
		
		rapidxml::xml_node<>* obj = doc.first_node("model")->first_node("resources")->first_node("object");
		
		while(obj) {
			//printf("Object:\n");
			
			rapidxml::xml_node<>* mesh = obj->first_node("mesh");
			
			while(mesh) {
			
				rapidxml::xml_node<>* vertices = mesh->first_node("vertices");
			
				manifold::MeshGL m;
		
				rapidxml::xml_node<>* vert = vertices->first_node();
				int vcount = 0;
				while(vert) {
					float v[3];
					v[0] = atof(vert->first_attribute("x")->value()); 
					v[1] = atof(vert->first_attribute("y")->value()); 
					v[2] = atof(vert->first_attribute("z")->value()); 
					m.vertProperties.insert(m.vertProperties.end(), {v[0], v[1], v[2]});
					vcount++;
					vert = vert->next_sibling();
				}
				//printf("\tVertices: %d\n", vcount);

				rapidxml::xml_node<>* triangles = mesh->first_node("triangles");
	
				rapidxml::xml_node<>* tri = triangles->first_node();
				int tcount = 0;
				while(tri) {
					uint32_t t[3];
					t[0] = atoi(tri->first_attribute("v1")->value()); 
					t[1] = atoi(tri->first_attribute("v2")->value()); 
					t[2] = atoi(tri->first_attribute("v3")->value()); 
					m.triVerts.insert(m.triVerts.end(), {t[0], t[1], t[2]});
					tcount++;
					tri = tri->next_sibling();
				}
				//printf("\tFaces: %d\n", tcount); fflush(stdout);
			
				meshes.push_back(m);
				mesh = mesh->next_sibling();
				
			}
			
			obj = obj->next_sibling();
		}
	}

    mz_free(uncomp_data);
    mz_zip_reader_end(&zip_archive);
	
	return meshes;
}

bool ExportMesh3MF(const std::string& filename, const manifold::MeshGL& mesh)
{
	std::string comment = "";
	std::string unit = "millimeter";
	remove(filename.c_str());
	
	if(!mz_zip_add_mem_to_archive_file_in_place(filename.c_str(), "_rels/", NULL, 0, comment.c_str(), comment.size(), MZ_BEST_COMPRESSION)) return false;
	
	if(!mz_zip_add_mem_to_archive_file_in_place(filename.c_str(), "3D/", NULL, 0, comment.c_str(), comment.size(), MZ_BEST_COMPRESSION)) return false;
	
	std::ostringstream buffer;
	
	buffer << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	buffer << "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">" << std::endl;
	buffer << "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\" />" << std::endl;
	buffer << "<Default Extension=\"model\" ContentType=\"application/vnd.ms-package.3dmanufacturing-3dmodel+xml\" />" << std::endl;
	buffer << "</Types>" << std::endl;
	if (!mz_zip_add_mem_to_archive_file_in_place(filename.c_str(), "[Content_Types].xml", buffer.str().c_str(), buffer.str().size(), comment.c_str(), comment.size(), MZ_BEST_COMPRESSION)) return false;
	
	buffer.str("");
	buffer.clear();

	buffer << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	buffer << "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" << std::endl;
	buffer << "<Relationship Target=\"/3D/3dmodel.model\" Id=\"rel0\" Type=\"http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel\" />" << std::endl;
	buffer << "</Relationships>" << std::endl;
	if(!mz_zip_add_mem_to_archive_file_in_place(filename.c_str(), "_rels/.rels", buffer.str().c_str(), buffer.str().size(), comment.c_str(), comment.size(), MZ_BEST_COMPRESSION)) return false;
	
	buffer.str("");
	buffer.clear();
	
	buffer << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	buffer << "  <model unit=\"" << unit << "\" xmlns=\"http://schemas.microsoft.com/3dmanufacturing/2013/01\">" << std::endl;
	buffer << "    <resources>" << std::endl;
	buffer << "      <object id=\"0\" type=\"model\">" << std::endl;
	buffer << "        <mesh>" << std::endl;
	buffer << "          <vertices>" << std::endl;

	buffer << std::setiosflags(std::ios_base::fixed);
	buffer << std::setprecision(18);
	
	for (size_t i = 0; i < mesh.NumVert(); ++i) {
		float v[3];
		for (int j : {0, 1, 2}) v[j] = mesh.vertProperties[i * mesh.numProp + j];
		buffer << "            <vertex x=\"" << v[0] << "\" y=\"" << v[1] << "\" z=\"" << v[2] << "\" />" << std::endl;
	}

	buffer << "          </vertices>" << std::endl;
	buffer << "          <triangles>" << std::endl;
	
	for (size_t i = 0; i < mesh.NumTri(); ++i) {
		uint32_t t[3];
		for (int j : {0, 1, 2}) t[j] = mesh.triVerts[3 * i + j];
		buffer << "            <triangle v1=\"" << t[0] << "\" v2=\"" << t[1] << "\" v3=\"" << t[2] << "\" />" << std::endl;
	}

	buffer << "          </triangles>" << std::endl;
	buffer << "        </mesh>" << std::endl;
	buffer << "      </object>" << std::endl;
	buffer << "    </resources>" << std::endl;
	buffer << "    <build>" << std::endl;
	buffer << "      <item objectid=\"0\" />" << std::endl;
	buffer << "    </build>" << std::endl;
	buffer << "  </model>" << std::endl;
	
	if (!mz_zip_add_mem_to_archive_file_in_place(filename.c_str(), "3D/3dmodel.model", buffer.str().c_str(), buffer.str().size(), comment.c_str(), comment.size(), MZ_BEST_COMPRESSION)) return false;
	
	return true;
	
}
				
bool ExportMeshes3MF(const std::string& filename, const std::vector<manifold::MeshGL> meshes)
{
	std::string comment = "";
	std::string unit = "millimeter";
	remove(filename.c_str());
	
	if(!mz_zip_add_mem_to_archive_file_in_place(filename.c_str(), "_rels/", NULL, 0, comment.c_str(), comment.size(), MZ_BEST_COMPRESSION)) return false;
	
	if(!mz_zip_add_mem_to_archive_file_in_place(filename.c_str(), "3D/", NULL, 0, comment.c_str(), comment.size(), MZ_BEST_COMPRESSION)) return false;
	
	std::ostringstream buffer;
	
	buffer << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	buffer << "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">" << std::endl;
	buffer << "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\" />" << std::endl;
	buffer << "<Default Extension=\"model\" ContentType=\"application/vnd.ms-package.3dmanufacturing-3dmodel+xml\" />" << std::endl;
	buffer << "</Types>" << std::endl;
	if (!mz_zip_add_mem_to_archive_file_in_place(filename.c_str(), "[Content_Types].xml", buffer.str().c_str(), buffer.str().size(), comment.c_str(), comment.size(), MZ_BEST_COMPRESSION)) return false;
	
	buffer.str("");
	buffer.clear();

	buffer << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	buffer << "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" << std::endl;
	buffer << "<Relationship Target=\"/3D/3dmodel.model\" Id=\"rel0\" Type=\"http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel\" />" << std::endl;
	buffer << "</Relationships>" << std::endl;
	if(!mz_zip_add_mem_to_archive_file_in_place(filename.c_str(), "_rels/.rels", buffer.str().c_str(), buffer.str().size(), comment.c_str(), comment.size(), MZ_BEST_COMPRESSION)) return false;
	
	buffer.str("");
	buffer.clear();
	
	buffer << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	buffer << "  <model unit=\"" << unit << "\" xmlns=\"http://schemas.microsoft.com/3dmanufacturing/2013/01\">" << std::endl;
	buffer << "    <resources>" << std::endl;
	
	int id = 0;
	for (auto m : meshes) {
		buffer << "      <object id=\"" << id << "\" type=\"model\">" << std::endl;
		buffer << "        <mesh>" << std::endl;
		buffer << "          <vertices>" << std::endl;

		buffer << std::setiosflags(std::ios_base::fixed);
		buffer << std::setprecision(18);

		for (size_t i = 0; i < m.NumVert(); ++i) {
			float v[3];
			for (int j : {0, 1, 2}) v[j] = m.vertProperties[i * m.numProp + j];
			buffer << "            <vertex x=\"" << v[0] << "\" y=\"" << v[1] << "\" z=\"" << v[2] << "\" />" << std::endl;
		}

		buffer << "          </vertices>" << std::endl;
		buffer << "          <triangles>" << std::endl;

		for (size_t i = 0; i < m.NumTri(); ++i) {
			uint32_t t[3];
			for (int j : {0, 1, 2}) t[j] = m.triVerts[3 * i + j];
			buffer << "            <triangle v1=\"" << t[0] << "\" v2=\"" << t[1] << "\" v3=\"" << t[2] << "\" />" << std::endl;
		}

		buffer << "          </triangles>" << std::endl;
		buffer << "        </mesh>" << std::endl;
		buffer << "      </object>" << std::endl;

		id++;
	}
	
	buffer << "    </resources>" << std::endl;
	buffer << "    <build>" << std::endl;
	
	for (int i=0; i<id; i++) 
		buffer << "      <item objectid=\"" << i << "\" />" << std::endl;
	
	buffer << "    </build>" << std::endl;
	buffer << "  </model>" << std::endl;
	
	if (!mz_zip_add_mem_to_archive_file_in_place(filename.c_str(), "3D/3dmodel.model", buffer.str().c_str(), buffer.str().size(), comment.c_str(), comment.size(), MZ_BEST_COMPRESSION)) return false;
	return true;
}


//STL

//Binary/ASCII STL loader with weld-tolerance dedup courtesy of Microsoft CoPilot:
#include <fstream>
#include <sstream>
#include <unordered_map>

//using namespace manifold;

// -----------------------------
// Hashable Vec3 for dedup
// -----------------------------
struct Vec3 {
	float x, y, z;
	bool operator==(const Vec3& o) const {
		return x == o.x && y == o.y && z == o.z;
	}
};

struct Vec3Hash {
	std::size_t operator()(const Vec3& v) const noexcept {
		const uint32_t* p = reinterpret_cast<const uint32_t*>(&v);
		return (size_t)p[0] * 73856093u ^
				(size_t)p[1] * 19349663u ^
				(size_t)p[2] * 83492791u;
	}
};

struct QuantizedVec3 {
	int x, y, z;
	bool operator==(const QuantizedVec3& o) const {
		return x == o.x && y == o.y && z == o.z;
	}
};

struct QuantizedHash {
	size_t operator()(const QuantizedVec3& q) const noexcept {
		return (size_t)q.x * 73856093u ^
				(size_t)q.y * 19349663u ^
				(size_t)q.z * 83492791u;
	}
};

inline QuantizedVec3 Quantize(const Vec3& v, float eps) {
	return {
		int(std::floor(v.x / eps)),
		int(std::floor(v.y / eps)),
		int(std::floor(v.z / eps))
	};
}



// -----------------------------
// Helpers
// -----------------------------
bool IsASCII_STL(const std::string& filename) {
	std::ifstream f(filename, std::ios::binary);
	if (!f) return false;

	char header[80];
	f.read(header, 80);

	// ASCII STL starts with "solid"
	std::string s(header, header + 5);
	return s == "solid";
}

// -----------------------------
// ASCII STL Loader
// -----------------------------
manifold::MeshGL LoadASCII_STL(const std::string& filename) {
	std::ifstream file(filename);
	if (!file) throw std::runtime_error("Cannot open ASCII STL");

	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<uint32_t> indices;

	float weldEpsilon = 1e-5f;   // tweak as needed
	std::unordered_map<QuantizedVec3, uint32_t, QuantizedHash> vertMap;

	auto addVertex = [&](const Vec3& v, const Vec3& n) {
		QuantizedVec3 key = Quantize(v, weldEpsilon);

		auto it = vertMap.find(key);
		if (it != vertMap.end()) {
			return it->second;   // welded match
		}

		uint32_t idx = positions.size() / 3;

		positions.push_back(v.x);
		positions.push_back(v.y);
		positions.push_back(v.z);

		normals.push_back(n.x);
		normals.push_back(n.y);
		normals.push_back(n.z);

		vertMap[key] = idx;
		return idx;
	};


	std::string line;
	Vec3 normal;

	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string word;
		ss >> word;

		if (word == "facet") {
			ss >> word; // "normal"
			ss >> normal.x >> normal.y >> normal.z;
		}
		else if (word == "vertex") {
			Vec3 v;
			ss >> v.x >> v.y >> v.z;
			uint32_t idx = addVertex(v, normal);
			indices.push_back(idx);
		}
	}

	manifold::MeshGL mesh;
	mesh.vertProperties = std::move(positions);
	//mesh.vertNormal = std::move(normals);
	mesh.triVerts = std::move(indices);
	return mesh;
}

// -----------------------------
// Binary STL Loader
// -----------------------------
manifold::MeshGL LoadBinary_STL(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary);
	if (!file) throw std::runtime_error("Cannot open binary STL");

	file.seekg(80); // skip header

	uint32_t triCount;
	file.read(reinterpret_cast<char*>(&triCount), 4);

	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<uint32_t> indices;

	positions.reserve(triCount * 9);
	normals.reserve(triCount * 9);
	indices.reserve(triCount * 3);
	
	float weldEpsilon = 1e-5f;   // tweak as needed
	std::unordered_map<QuantizedVec3, uint32_t, QuantizedHash> vertMap;

	auto addVertex = [&](const Vec3& v, const Vec3& n) {
		QuantizedVec3 key = Quantize(v, weldEpsilon);

		auto it = vertMap.find(key);
		if (it != vertMap.end()) {
			return it->second;   // welded match
		}

		uint32_t idx = positions.size() / 3;

		positions.push_back(v.x);
		positions.push_back(v.y);
		positions.push_back(v.z);

		normals.push_back(n.x);
		normals.push_back(n.y);
		normals.push_back(n.z);

		vertMap[key] = idx;
		return idx;
	};


	for (uint32_t i = 0; i < triCount; i++) {
		Vec3 n, v1, v2, v3;
		file.read((char*)&n, 12);
		file.read((char*)&v1, 12);
		file.read((char*)&v2, 12);
		file.read((char*)&v3, 12);

		uint32_t i1 = addVertex(v1, n);
		uint32_t i2 = addVertex(v2, n);
		uint32_t i3 = addVertex(v3, n);

		indices.push_back(i1);
		indices.push_back(i2);
		indices.push_back(i3);

		file.seekg(2, std::ios::cur); // skip attribute bytes
	}

	manifold::MeshGL mesh;
	mesh.vertProperties = std::move(positions);
	//mesh.vertNormal = std::move(normals);
	mesh.triVerts = std::move(indices);
	return mesh;
}

// -----------------------------
// Unified Loader
// -----------------------------
manifold::MeshGL LoadSTL_Dedup(const std::string& filename) {
	if (IsASCII_STL(filename))
		return LoadASCII_STL(filename);
	else
		return LoadBinary_STL(filename);
}

manifold::MeshGL ImportMeshSTL(const std::string& filename)
{
	manifold::MeshGL mesh = LoadSTL_Dedup(filename);
	return mesh;
}

bool ExportMeshSTL(const std::string& filename, const manifold::MeshGL& mesh)
{
	const uint64_t numBytes = mesh.NumTri() * 50 + 84;
    char *dst = (char *)calloc(numBytes, 1);

	const uint32_t count = mesh.NumTri();
    memcpy(dst + 80, &count, 4);
	
	for (size_t i = 0; i < mesh.NumTri(); ++i) {
		uint32_t t[3];
		for (int j : {0, 1, 2}) t[j] = mesh.triVerts[3 * i + j];
		float p0[3];
		for (int j : {0, 1, 2}) p0[j] = mesh.vertProperties[t[0] * mesh.numProp + j];
		float p1[3];
		for (int j : {0, 1, 2}) p1[j] = mesh.vertProperties[t[1] * mesh.numProp + j];
		float p2[3];
		for (int j : {0, 1, 2}) p2[j] = mesh.vertProperties[t[2] * mesh.numProp + j];
		
		float n[3], u[3], v[3];
	
		u[0] = p1[0] - p0[0];
		u[1] = p1[1] - p0[1];
		u[2] = p1[2] - p0[2];
	
		v[0] = p2[0] - p0[0];
		v[1] = p2[1] - p0[1];
		v[2] = p2[2] - p0[2];
	
		n[0] = u[1]*v[2] - u[2]*v[1];
		n[1] = u[2]*v[0] - u[0]*v[2];
		n[2] = u[0]*v[1] - u[1]*v[0];
	
		double l = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
		n[0] = n[0]/l;
		n[1] = n[1]/l;
		n[2] = n[2]/l;
		
		float normal[3] = {n[0], n[1], n[2]};

        const uint64_t idx = 84 + i * 50;
        memcpy(dst + idx, &normal, 12);
        memcpy(dst + idx + 12, &p0, 12);
        memcpy(dst + idx + 24, &p1, 12);
        memcpy(dst + idx + 36, &p2, 12);
    }

    std::fstream file(filename, std::ios::out | std::ios::binary);
    file.write(dst, numBytes);
    file.close();

    free(dst);
	return true;
}
