#ifndef LE_GRAPHICSVIEW_H
#define LE_GRAPHICSVIEW_H 1

#pragma once

#include "IGraphicsView.h"
#include "ShadingPipeline.h"
#include "../Core/Threading/ThreadSafeWorkerContainer.hpp"

namespace LeoEngine::GraphicsPipeline {
	using ShadingItem = LeoEngine::Render::ShadingItem;
	class GraphicsView : public IGraphicsView {
	public:
		GraphicsView(ViewType view_type);

		void SetFrameID(leo::uint32 frameId) final;
		leo::uint32 GetFrameID() const final;


		void SetFrameTime(TimeValue time) final;
		TimeValue GetFrameTime() const final;

		void SetCamera(const Camera& camera) final;
		void SetPreviousFrameCamera(const Camera& camera) final;

		void SwitchUsageMode(UsageMode mode) final;

		void SetViewPort(const ViewPort& viewport) final;

		const ViewPort& GetViewPort() const final;

		LightIndex  AddLight(const  DirectLight& light) final;
		DirectLight& GetLight(LightIndex light_id) final;

		void AddShadingObject(leo::observer_ptr<ShadingElement> pShadingElement, leo::observer_ptr<ShadingObject> pShadingObject,const EffectItem& effect_item) final;
	public:
		//TODO! lockfree_add_vector
		using ShadingItems = leo::vector<Render::ShadingItem>;
		using DirectgLightList = leo::list<DirectLight>;

	private:
		ViewType view_type;
		UsageMode usage_mode;

		leo::uint32 frame_id;
		TimeValue frame_time;

		leo::observer_ptr<GraphicsView> parent_view;

		ShadingItems shading_items[Render::EFLIST_COUNT];
		DirectgLightList lights;

		ViewPort view_port;

		Camera camera;
		Camera previous_camera;

		//Temporary shading objects storage
		struct {
			Worker::ThreadSafeWorkerContainer<ShadingObject*> objects;
		};
	};
}

#endif