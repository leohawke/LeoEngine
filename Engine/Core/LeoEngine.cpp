#include "LeoEngine.h"
#include "../GraphicsPipeline/GraphicsView.h"

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
