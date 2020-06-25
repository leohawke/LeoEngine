/*! \file Engine\Render\D3D12\FrameBuffer.h
\ingroup Engine\Render\D3D12
\brief ‰÷»æµΩŒ∆¿Ì°£
*/
#ifndef LE_RENDER_D3D12_FrameBuffer_h
#define LE_RENDER_D3D12_FrameBuffer_h 1

#include "..\IFrameBuffer.h"
#include "d3d12_dxgi.h"

namespace platform_ex::Windows::D3D12 {
	class DepthStencilView;

	class FrameBuffer :public platform::Render::FrameBuffer {
	public:
		FrameBuffer();
		~FrameBuffer();

		void OnBind() override;
		void OnUnBind() override;

		void Clear(leo::uint32 flags, const leo::math::float4  & clr, float depth, leo::int32 stencil) override;

		DepthStencilView* GetDepthStencilView() const;

		platform::Render::Texture* Attached(FrameBuffer::Attachment) const;
	public:
		void SetRenderTargets();
		void BindBarrier();

		friend class Display;
	private:
		D3D12_VIEWPORT d3d12_viewport;
	};
}

#endif
