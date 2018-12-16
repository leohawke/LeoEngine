#include "GraphicsView.h"

using namespace LeoEngine::GraphicsPipeline;

void GraphicsView::SetFrameID(leo::uint32 frameId)
{
}

leo::uint32 GraphicsView::GetFrameID() const
{
	return leo::uint32();
}

void GraphicsView::SetFrameTime(TimeValue time)
{
}

TimeValue GraphicsView::GetFrameTime() const
{
	return TimeValue();
}

void GraphicsView::SetCamera(const Camera & camera)
{
}
void GraphicsView::SetPreviousFrameCamera(const Camera & camera)
{
}

void GraphicsView::SwitchUsageMode(UsageMode mode)
{
}

void GraphicsView::SetViewPort(const ViewPort & viewport)
{
}

const ViewPort & GraphicsView::GetViewPort() const
{
	return view_port;
}

LightIndex GraphicsView::AddLight(const DirectLight & light)
{
	return LightIndex();
}

DirectLight & GraphicsView::GetLight(LightIndex light_id)
{
	return *std::next(lights.begin(), light_id);
}

void GraphicsView::AddShadingObject(leo::observer_ptr<ShadingElement> pShadingElement, leo::observer_ptr<ShadingObject> pShadingObject, EffectItem & effect_item)
{
}