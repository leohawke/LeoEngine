/*! \file Engine\Render\IFrameBuffer.h
\ingroup Engine
\brief ‰÷»æµΩŒ∆¿Ì°£
*/
#ifndef LE_RENDER_IFrameBuffer_h
#define LE_RENDER_IFrameBuffer_h 1

#include "IRenderView.h"
#include "../Core/ViewPort.h"
#include <vector>
#include <memory>

namespace platform {
	namespace Render {

		class FrameBuffer {
		public:
			enum Attachment : uint8{
				Target0,
				Target1,
				Target2,
				Target3,
				Target4,
				Target5,
				Target6,
				Target7,
				DepthStencil,
			};

			virtual ~FrameBuffer();

			virtual void OnBind();
			virtual void OnUnBind();

			leo::observer_ptr<GPUView> Attached(Attachment which) const;
		protected:
			std::vector<std::shared_ptr<RenderTargetView>> clr_views;
			std::shared_ptr<DepthStencilView> ds_view;
			std::vector<std::shared_ptr<UnorderedAccessView>> uav_views;

			ViewPort viewport;
		};
	}
}

#endif