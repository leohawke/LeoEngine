/*! \file Engine\Asset\Mesh.h
\ingroup Engine
\brief Mesh IO ...
*/
#ifndef LE_ASSET_MESH_X_H
#define LE_ASSET_MESH_X_H 1


#include "../Core/Mesh.h"
#include <filesystem>
#include <string_view>
#include "../Core/Coroutine/Task.h"
namespace platform {
	namespace X {
		using path = std::filesystem::path;

		std::shared_ptr<asset::MeshAsset> LoadMeshAsset(path const& meshpath);

		std::shared_ptr<Mesh> LoadMesh(path const& meshpath,const std::string& name);

		leo::coroutine::Task<std::shared_ptr<asset::MeshAsset>> AsyncLoadMeshAsset(path const& meshpath);
		leo::coroutine::Task<std::shared_ptr<Mesh>> AsyncLoadMesh(path const& meshpath, const std::string& name);

	}
}
#endif