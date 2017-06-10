/*! \file Engine\Render\D3D12\FrameBuffer.h
\ingroup Engine\Render\D3D12
\brief ‰÷»æµΩŒ∆¿Ì°£
*/
#ifndef LE_RENDER_D3D12_FrameBuffer_h
#define LE_RENDER_D3D12_FrameBuffer_h 1

#include "..\IFrameBuffer.h"

namespace platform_ex::Windows::D3D12 {
	class FrameBuffer :public platform::Render::FrameBuffer {
	public:
		~FrameBuffer();

		void OnBind() override;
	public:
		void SetRenderTargets();
		void BindBarrier();
		void UnBindBarrier();
	};
}

#endif
