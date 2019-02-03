#include "LeoEngine.h"
#include "../GraphicsPipeline/GraphicsView.h"
#include "LeoEngine/ShadingElements/SEMeshImpl.h"

using namespace LeoEngine;
using namespace LeoEngine::GraphicsPipeline;

GraphicsView* GraphicsEngine::LeoEngine::GetOrCreateRenderView(IGraphicsView::ViewType type) {
	//todo pool
	return new GraphicsView(type);
}

void GraphicsEngine::LeoEngine::ReturnRenderView(GraphicsView* pRenderView)
{
	delete this;
}

ShadingElement* LeoEngine::GraphicsEngine::LeoEngine::CreateShadingElement(Render::ShadingElementDataType sdt)
{
	switch (sdt)
	{
	case Render::SED_Mesh:
		return new SEMeshImpl();
	}

	return nullptr;
}
