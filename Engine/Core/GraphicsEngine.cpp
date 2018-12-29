#include "GraphicsEngine.h"
#include "../GraphicsPipeline/GraphicsView.h"
#include "../System/NinthTimer.h"
#include "../System/SystemEnvironment.h"

using namespace LeoEngine::GraphicsEngine;

void GraphicsPassInfo::SetGraphicsView(IGraphicsView::ViewType type) {
	graphics_view = leo::make_observer(Environment->LeoEngine->GetOrCreateRenderView(type));
	SetGraphicsView(graphics_view.get());
}

void GraphicsPassInfo::SetGraphicsView(IGraphicsView* pGraphicsView) {
	pGraphicsView->SetFrameID(GetFrameID());
	pGraphicsView->SetFrameTime(Environment->Timer->GetFrameStartTime());
	pGraphicsView->SetViewPort(ViewPort(0, 0, pCamera->GetFrustumViewWidth(), pCamera->GetFrustumViewHeight()));
}

GraphicsPassInfo GraphicsPassInfo::CreateGeneralPassGraphicsInfo(const Camera& camera) {
	GraphicsPassInfo passInfo;

	passInfo.SetCamera(camera);
	passInfo.SetGraphicsView(IGraphicsView::Default_View);

	return passInfo;
}

void GraphicsPassInfo::SetCamera(const Camera& camera) {
	pCamera = Environment->LeoEngine->GetGraphicsPassCamera(camera);
}
