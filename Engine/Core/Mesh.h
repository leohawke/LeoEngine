/*! \file Core\Mesh.h
\ingroup Engine
\brief 提供渲染所需的InputLayout。
*/
#ifndef LE_Core_Mesh_H
#define LE_Core_Mesh_H 1

#include "../Asset/MeshAsset.h"
#include "Resource.h"
#include "../Render/InputLayout.hpp"

namespace platform {

	class Mesh :Resources {
	public:
		Mesh(const asset::MeshAsset& asset, const std::string& name);

		const asset::MeshAsset::SubMeshDescrption::LodDescription& GetSubMeshCurretnLodDescription(int submesh_index);
		const asset::MeshAsset::SubMeshDescrption::LodDescription& GetSubMeshLodDescription(int submesh_index, int lod_index);
		leo::uint8 GetSubMeshMaterialIndex(int submesh_index);
		
		DefGetter(const lnothrow, leo::uint8, SubMeshCount,static_cast<leo::uint8>(sub_meshes.size()))
		DefGetter(const lnothrow, const std::vector<asset::MeshAsset::SubMeshDescrption>&, SubMeshDesces, sub_meshes)
		DefGetter(const lnothrow, leo::uint8, GeometryLod, mesh_lod)
		DefSetter(lnothrow,leo::uint8,GeometryLod, mesh_lod)
	private:
		std::unique_ptr<Render::InputLayout> input_layout;
		std::vector<asset::MeshAsset::SubMeshDescrption> sub_meshes;
		leo::uint8 mesh_lod = 0;
	};
}

#endif