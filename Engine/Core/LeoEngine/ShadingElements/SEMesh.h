/*! \file Core\LeoEngine\ShadingElements\SEMesh.h
\ingroup LeoEngine
\brief Mesh×ÅÉ«ÔªËØ¡£
*/
#ifndef LE_Core_Leo_Engine_Shading_Elements_SEMesh_H
#define LE_Core_Leo_Engine_Shading_Elements_SEMesh_H 1

#include "../../../Render/ShadingElement.h"
#include <LBase/linttype.hpp>

namespace platform{
	class Mesh;
}

namespace LeoEngine::GraphicsEngine {
	using namespace leo::inttype;



	class SEMesh : public Render::ShadingElement
	{
		using base =  Render::ShadingElement;
	public:
		platform::Mesh* pRenderMesh = nullptr;

		uint32 firstIndexId = 0;
		uint32 numIndices = 0;

		uint32 firstVertId =0;
		uint32 numVerts = 0;

	protected:
		SEMesh()
		:base(Render::SED_Mesh){
		}

		//Impl
		virtual ~SEMesh();
	};
}

#endif