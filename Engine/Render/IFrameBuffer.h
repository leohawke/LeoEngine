/*! \file Engine\Render\IFrameBuffer.h
\ingroup Engine
\brief ‰÷»æµΩŒ∆¿Ì°£
*/
#ifndef LE_RENDER_IFrameBuffer_h
#define LE_RENDER_IFrameBuffer_h 1

#include "IRenderView.h"
#include <vector>
#include <memory>

namespace platform {
	namespace Render {

		class FrameBuffer {
		public:
			~FrameBuffer();

		private:
			std::vector<std::shared_ptr<RenderTargetView>> clr_views;
			std::shared_ptr<DepthStencilView> ds_view;
			std::vector<std::shared_ptr<UnorderedAccessView>> uav_views;
		};
	}
}

#endif