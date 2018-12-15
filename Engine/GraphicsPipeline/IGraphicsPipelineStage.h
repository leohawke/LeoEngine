#ifndef LE_GRAPHICSPIPELINE_H
#define LE_GRAPHICSPIPELINE_H 1

#pragma once

#include <LBase/linttype.hpp>
#include <LBase/memory.hpp>

namespace platform::GraphicsPipeline {
	class GraphicsView;
	class GraphicsPipeline;

	class GraphicsPipelineStage {
	public:
		virtual ~GraphicsPipelineStage();

		virtual void Update();

		virtual void Resize(std::pair<leo::uint16,leo::uint16> size);
	public:
		void WatchGraphics(leo::observer_ptr<GraphicsView> pGraphicsView) {
			graphics_view = pGraphicsView;
		}
	protected:
		friend class GraphicsPipeline;

		leo::uint32 stage_id;
	private	:
		leo::observer_ptr<GraphicsPipeline> owner_graphics_pipeline = nullptr;
		leo::observer_ptr<GraphicsView>  graphics_view = nullptr;
	};
}

#endif
