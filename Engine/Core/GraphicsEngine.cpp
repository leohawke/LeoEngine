#include "GraphicsEngine.h"

using namespace LeoEngine::GraphicsEngine;

void GraphicsPassInfo::SetGraphicsView(IGraphicsView::ViewType type) {

}

void GraphicsPassInfo::SetGraphicsView(IGraphicsView* pGraphicsView) {

}

static GraphicsPassInfo GraphicsPassInfo::CreateGeneralPassGraphicsInfo(const Camera& camera) {
	GraphicsPassInfo passInfo;

	return passInfo;
}

void GraphicsPassInfo::SetCamera(const Camera& camera) {
	//TODO
	//pCamera = Environment->LeoEngine->GetGraphicsPassCamera(camera);
}
