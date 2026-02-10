#pragma once
#include <string>

#include "manifold/manifold.h"

//STL routines:
manifold::MeshGL ImportMeshSTL(const std::string& filename);

bool ExportMeshSTL(const std::string& filename, const manifold::MeshGL& mesh);


//3MF routines:
manifold::MeshGL ImportMesh3MF(const std::string& filename);

std::vector<manifold::MeshGL> ImportMeshes3MF(const std::string& filename);

bool ExportMesh3MF(const std::string& filename, const manifold::MeshGL& mesh);
				
bool ExportMeshes3MF(const std::string& filename, const std::vector<manifold::MeshGL> meshes);

inline std::string manifoldError(const manifold::Manifold::Error& error) {
  switch (error) {
    case manifold::Manifold::Error::NoError:
      return "No Error";
    case manifold::Manifold::Error::NonFiniteVertex:
      return "Non Finite Vertex";
    case manifold::Manifold::Error::NotManifold:
      return "Not Manifold";
    case manifold::Manifold::Error::VertexOutOfBounds:
      return "Vertex Out Of Bounds";
    case manifold::Manifold::Error::PropertiesWrongLength:
      return "Properties Wrong Length";
    case manifold::Manifold::Error::MissingPositionProperties:
      return "Missing Position Properties";
    case manifold::Manifold::Error::MergeVectorsDifferentLengths:
      return "Merge Vectors Different Lengths";
    case manifold::Manifold::Error::MergeIndexOutOfBounds:
      return "Merge Index Out Of Bounds";
    case manifold::Manifold::Error::TransformWrongLength:
      return "Transform Wrong Length";
    case manifold::Manifold::Error::RunIndexWrongLength:
      return "Run Index Wrong Length";
    case manifold::Manifold::Error::FaceIDWrongLength:
      return "Face ID Wrong Length";
    case manifold::Manifold::Error::InvalidConstruction:
      return "Invalid Construction";
    case manifold::Manifold::Error::ResultTooLarge:
      return "Result Too Large";
    default:
      return "Unknown Error";
  };
}

