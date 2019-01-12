#include "GraphicsView.h"

using namespace LeoEngine::GraphicsPipeline;
using namespace LeoEngine::Render;

GraphicsView::GraphicsView(ViewType _view_type)
	:view_type(_view_type)
{}

void GraphicsView::SetFrameID(leo::uint32 frameId)
{
	frame_id = frameId;
}

leo::uint32 GraphicsView::GetFrameID() const
{
	return frame_id;
}

void GraphicsView::SetFrameTime(TimeValue time)
{
	frame_time = time;
}

TimeValue GraphicsView::GetFrameTime() const
{
	return frame_time;
}

void GraphicsView::SetCamera(const Camera & _camera)
{
	camera = _camera;
}
void GraphicsView::SetPreviousFrameCamera(const Camera & camera)
{
	previous_camera = camera;
}

void GraphicsView::SwitchUsageMode(UsageMode mode)
{
	if (mode == usage_mode)
		return;

	if (mode == Writing) {
		//Register PostWrite
	}

	if (mode == Writed) {
		//Notify PostWrite
	}

	if (mode == Reading) {
		//wait write
		//wait postwrite
		//wait sort

		//BeginRendering
	}

	if (mode == Readed) {
		//EndRendering
	}

	usage_mode = mode;
}

void GraphicsView::SetViewPort(const ViewPort & viewport)
{
	view_port = viewport;
}

const ViewPort & GraphicsView::GetViewPort() const
{
	return view_port;
}

LightIndex GraphicsView::AddLight(const DirectLight & light)
{
	LightIndex nLightId =static_cast<LightIndex>(lights.size());

	lights.push_back(light);

	return nLightId;
}

DirectLight & GraphicsView::GetLight(LightIndex light_id)
{
	return *std::next(lights.begin(), light_id);
}

void GraphicsView::AddShadingObject(leo::observer_ptr<ShadingElement> pShadingElement, leo::observer_ptr<ShadingObject> pShadingObject,const EffectItem & effect_item)
{
	ShadingItem item{ pShadingObject.get(),pShadingElement.get(),  effect_item };
	shading_items[EFLIST_GENERAL].emplace_back(std::move(item));
}