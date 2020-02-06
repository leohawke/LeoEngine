/*! \file Core\Mesh.h
\ingroup Engine
\brief 提供渲染所需的InputLayout。
*/
#ifndef LE_Core_Mesh_H
#define LE_Core_Mesh_H 1

#include "../Asset/MeshAsset.h"
#include "Resource.h"
#include "../Render/InputLayout.hpp"
#include "ResourcesHolder.h"
#include "../Render/IRayTracingGeometry.h"

namespace platform {

	class Mesh :public Resource {
	public:
		Mesh(const asset::MeshAsset& asset, const std::string& name);

		const asset::MeshAsset::SubMeshDescrption::LodDescription& GetSubMeshCurretnLodDescription(int submesh_index);
		const asset::MeshAsset::SubMeshDescrption::LodDescription& GetSubMeshLodDescription(int submesh_index, int lod_index);
		leo::uint8 GetSubMeshMaterialIndex(int submesh_index);

		DefGetter(const lnothrow, leo::uint8, SubMeshCount, static_cast<leo::uint8>(sub_meshes.size()))
			DefGetter(const lnothrow, const std::vector<asset::MeshAsset::SubMeshDescrption>&, SubMeshDesces, sub_meshes)
			DefGetter(const lnothrow, leo::uint8, GeometryLod, mesh_lod)
			DefSetter(lnothrow, leo::uint8, GeometryLod, mesh_lod)

			DefGetter(const lnothrow, const Render::InputLayout&, InputLayout, *input_layout);

		const std::string& GetName() const lnothrow;
	private:
		std::unique_ptr<Render::InputLayout> input_layout;
		std::vector<asset::MeshAsset::SubMeshDescrption> sub_meshes;
		std::unique_ptr<Render::RayTracingGeometry> tracing_geometry;
		leo::uint8 mesh_lod = 0;
		std::string name;
	};

	class MeshesHolder :ResourcesHolder<Mesh> {
	public:
		MeshesHolder();
		~MeshesHolder();

		std::shared_ptr<void> FindResource(const leo::any& key) override;
		std::shared_ptr<Mesh> FindResource(const std::shared_ptr<asset::MeshAsset>& asset, const std::string& name);

		void Connect(const std::shared_ptr<asset::MeshAsset>& asset, const std::shared_ptr<Mesh>& mesh);

		static MeshesHolder& Instance();
	private:
		std::pmr::vector<std::pair<std::weak_ptr<asset::MeshAsset>, std::shared_ptr<Mesh>>> loaded_meshes;
	};

}

#endif