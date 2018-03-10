#include "Mesh.h"
#include "../Render/IContext.h"

namespace platform {
	using namespace Render;

	Mesh::Mesh(const asset::MeshAsset & asset, const std::string & name)
		:Resources(name),sub_meshes(asset.GetSubMeshDesces())
	{
		auto& device = Context::Instance().GetDevice();
		input_layout = unique_raw(device.CreateInputLayout());
		
		for (std::size_t i = 0; i != asset.GetVertexElements().size(); ++i) {
			auto& element = asset.GetVertexElements()[i];
			auto& stream = asset.GetVertexStreams()[i];
			auto vertex_stream =leo::share_raw(
				device.CreateVertexBuffer(
					Buffer::Usage::Static,
					EAccessHint::EA_GPURead|EAccessHint::EA_Immutable, 
					element.GetElementSize()*asset.GetVertexCount(),
					element.format, stream.get()));
			input_layout->BindVertexStream(vertex_stream, { element });
		}

		auto index_stream = leo::share_raw(
			device.CreateIndexBuffer(
				Buffer::Usage::Static, EAccessHint::EA_GPURead | EAccessHint::EA_Immutable,
				NumFormatBytes(asset.GetIndexFormat())*asset.GetIndexCount(),
				asset.GetIndexFormat(), asset.GetIndexStreams().get()));
		input_layout->BindIndexStream(index_stream, asset.GetIndexFormat());

		//Topo
		input_layout->SetTopoType(InputLayout::TriangleList);
	}
	const asset::MeshAsset::SubMeshDescrption::LodDescription & Mesh::GetSubMeshCurretnLodDescription(int submesh_index)
	{
		return GetSubMeshLodDescription(submesh_index, mesh_lod);
	}
	const asset::MeshAsset::SubMeshDescrption::LodDescription & Mesh::GetSubMeshLodDescription(int submesh_index, int lod_index)
	{
		return sub_meshes[submesh_index].LodsDescription[mesh_lod];
	}
	leo::uint8 Mesh::GetSubMeshMaterialIndex(int submesh_index)
	{
		return sub_meshes[submesh_index].MaterialIndex;
	}
}
