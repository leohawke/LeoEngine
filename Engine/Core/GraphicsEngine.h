/*! \file Core\GraphicsEngine.h
\ingroup GraphicsEngine
\brief Í¼ÐÎÒýÇæ¡£
*/
#ifndef LE_Core_Graphics_Engine_H
#define LE_Core_Graphics_Engine_H 1

#include <LBase/lmemory.hpp>
#include "../GraphicsPipeline/IGraphicsView.h"


namespace LeoEngine::GraphicsEngine {
	using namespace LeoEngine::GraphicsPipeline;

	struct GraphicsPassInfo {


		std::shared_ptr<IGraphicsView> graphics_view;

		leo::uint32 frame_id;

		void SetGraphicsView(IGraphicsView::ViewType type);


		static GraphicsPassInfo CreateGeneralPassGraphicsInfo(const Camera& camera);

	private:
		GraphicsPassInfo() {
			//TODO!
		}


		void SetGraphicsView(IGraphicsView* pGraphicsView);
		void SetCamera(const Camera& camera);

		leo::uint8 thread_id = 0;
		loe::uint32 frame_id = 0;

		const Camera* pCamera = nullptr;
	};
}

#endif