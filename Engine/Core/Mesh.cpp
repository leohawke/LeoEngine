#include "Mesh.h"

namespace platform {

	Mesh::Mesh(const asset::MeshAsset & asset, const std::string & name)
		:Resources(name)
	{
	}
	const asset::MeshAsset::SubMeshDescrption::LodDescription & Mesh::GetSubMeshCurretnLodDescription(int submesh_index)
	{
		// TODO: 在此处插入 return 语句
	}
	const asset::MeshAsset::SubMeshDescrption::LodDescription & Mesh::GetSubMeshLodDescription(int submesh_index, int lod_index)
	{
		// TODO: 在此处插入 return 语句
	}
}
