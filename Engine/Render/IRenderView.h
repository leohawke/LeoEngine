/*! \file Engine\Render\IFormat.hpp
\ingroup Engine
\brief GPU数据结构View。
*/
#ifndef LE_RENDER_IRenderView_h
#define LE_RENDER_IRenderView_h 1


namespace platform {
	namespace Render {
		class FrameBuffer;

		class RenderTargetView  {
		public:
			virtual ~RenderTargetView();

			
		};

		class DepthStencilView {
		public:
			virtual ~DepthStencilView();

			
		};

		class UnorderedAccessView  {
		public:
			virtual ~UnorderedAccessView();

			
		};
	}
}

#endif