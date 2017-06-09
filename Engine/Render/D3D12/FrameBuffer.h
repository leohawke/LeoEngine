/*! \file Engine\Render\D3D12\FrameBuffer.h
\ingroup Engine\Render\D3D12
\brief ��Ⱦ������
*/
#ifndef LE_RENDER_D3D12_FrameBuffer_h
#define LE_RENDER_D3D12_FrameBuffer_h 1

#include "..\IFrameBuffer.h"

namespace platform_ex::Windows::D3D12 {
	class FrameBuffer : platform::Render::FrameBuffer {
	public:
		~FrameBuffer();
	};
}

#endif
