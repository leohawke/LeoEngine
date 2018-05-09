/*! \file Engine\Asset\Mesh.h
\ingroup Engine
\brief Mesh IO ...
*/
#ifndef LE_ASSET_MESH_X_H
#define LE_ASSET_MESH_X_H 1


#include "../Core/Mesh.h"
#include <experimental/filesystem>
#include <string_view>
namespace platform {
	namespace X {
		using path = std::experimental::filesystem::path;

		std::shared_ptr<asset::MeshAsset> LoadMeshAsset(path const& meshpath);
		std::shared_ptr<Mesh> LoadMesh(path const& meshpath,const std::string& name);
	}
}
#endif