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
		MeshAsset() = default;

		DefGetter(const lnothrow, const std::vector<platform::Render::Vertex::Element>&, VertexElements, vertex_elements)
			DefGetter(lnothrow, std::vector<platform::Render::Vertex::Element>&, VertexElementsRef, vertex_elements)
	private:
		std::vector<platform::Render::Vertex::Element> vertex_elements;
	};
}


#endif