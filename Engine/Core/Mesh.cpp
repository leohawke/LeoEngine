#include "Mesh.h"

namespace platform {

	Mesh::Mesh(const asset::MeshAsset & asset, const std::string & name)
		:Resources(name)
	{
	}
	const asset::MeshAsset::SubMeshDescrption::LodDescription & Mesh::GetSubMeshCurretnLodDescription(int submesh_index)
	{
		// TODO: �ڴ˴����� return ���
	}
	const asset::MeshAsset::SubMeshDescrption::LodDescription & Mesh::GetSubMeshLodDescription(int submesh_index, int lod_index)
	{
		// TODO: �ڴ˴����� return ���
	}
}
