/*! \file Engine\Render\IFormat.hpp
\ingroup Engine
\brief GPU数据结构View。
*/
#ifndef LE_RENDER_IRenderView_h
#define LE_RENDER_IRenderView_h 1

#include "ITexture.hpp"

namespace platform {
	namespace Render {
		class FrameBuffer;

		class GPUView {
		public:
			GPUView(uint16 width, uint16 height, EFormat format);

			virtual ~GPUView();

			uint16 Width() const;
			uint16 Height() const;

			EFormat Format() const;

		protected:
			uint16 width;
			uint16 height;
			EFormat format;
		};

		class RenderTargetView :public GPUView  {
		public:
			using GPUView::GPUView;

			virtual ~RenderTargetView();
		};

		class DepthStencilView :public GPUView {
		public:
			virtual ~DepthStencilView();

			
		};

		class UnorderedAccessView :public GPUView {
		public:
			virtual ~UnorderedAccessView();

			
		};
	}
}

#endif