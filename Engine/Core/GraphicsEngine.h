/*! \file Core\GraphicsEngine.h
\ingroup GraphicsEngine
\brief Í¼ÐÎÒýÇæ¡£
*/
#ifndef LE_Core_Graphics_Engine_H
#define LE_Core_Graphics_Engine_H 1

#include <LBase/lmemory.hpp>
#include "../GraphicsPipeline/IGraphicsView.h"

namespace LeoEngine::GraphicsPipeline {
	class GraphicsView;
}


namespace LeoEngine::GraphicsEngine {
	using namespace LeoEngine::GraphicsPipeline;

	struct GraphicsPassInfo {
		void SetGraphicsView(IGraphicsView::ViewType type);

		static GraphicsPassInfo CreateGeneralPassGraphicsInfo(const Camera& camera);


		leo::uint32 GetFrameID() { return frame_id; }
	private:
		GraphicsPassInfo() {
			//TODO!
		}


		void SetGraphicsView(IGraphicsView* pGraphicsView);
		void SetCamera(const Camera& camera);
		leo::observer_ptr<IGraphicsView> graphics_view;


		leo::uint8 thread_id = 0;
		leo::uint32 frame_id = 0;

		const Camera* pCamera = nullptr;
	};

	inline namespace Interface {
		//! Main interface to the renderer implementation, wrapping the graphics API
		class ILeoEngine {
		public:
			virtual GraphicsView* GetOrCreateRenderView(IGraphicsView::ViewType type) = 0;
		};
	}
}

#endif