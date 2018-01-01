/*! \file Engine\Asset\MeshAsset.h
\ingroup Engine
\brief Mesh Binary From KlaGE...
*/
#ifndef LE_ASSET_MESH_ASSET_H
#define LE_ASSET_MESH_ASSET_H 1

#include <LBase/sutility.h>
#include <LBase/lmathtype.hpp>

#include "../Render/InputLayout.hpp"

namespace asset {
	class MeshAsset :leo::noncopyable {
	public:
		struct SubMeshDescrption
		{
			leo::uint8 MaterialIndex;
			struct LodDescription {
				leo::uint32 VertexNum;
				leo::uint32 VertexBase;
				leo::uint32 IndexNum;
				leo::uint32 IndexBase;
			};
			std::vector<LodDescription> LodsDescription;
		};

		MeshAsset() = default;

		DefGetter(const lnothrow, const std::vector<platform::Render::Vertex::Element>&, VertexElements, vertex_elements)
			DefGetter(lnothrow, std::vector<platform::Render::Vertex::Element>&, VertexElementsRef, vertex_elements)

			DefGetter(const lnothrow, platform::Render::EFormat, IndexFormat, index_format)
			DefSetter(lnothrow, platform::Render::EFormat, IndexFormat, index_format)

			DefGetter(lnothrow, std::vector<std::unique_ptr<stdex::byte[]>>&, VertexStreamsRef, vertex_streams)
			DefGetter(const lnothrow,const std::vector<std::unique_ptr<stdex::byte[]>>&, VertexStreams, vertex_streams)
	
			DefGetter(lnothrow, std::unique_ptr<stdex::byte[]>&, IndexStreamsRef, index_stream)
			DefGetter(const lnothrow, leo::observer_ptr<stdex::byte>, IndexStreams,leo::make_observer(index_stream.get()))

			DefGetter(const lnothrow, const std::vector<SubMeshDescrption>&, SubMeshDesces, sub_meshes)
			DefGetter(lnothrow, std::vector<SubMeshDescrption>&, SubMeshDescesRef, sub_meshes)
	private:
		std::vector<platform::Render::Vertex::Element> vertex_elements;
		platform::Render::EFormat index_format;
		std::vector<std::unique_ptr<stdex::byte[]>> vertex_streams;
		std::unique_ptr<stdex::byte[]> index_stream;
		std::vector<SubMeshDescrption> sub_meshes;
	};
}


#endif