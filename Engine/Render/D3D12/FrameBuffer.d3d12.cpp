#include "FrameBuffer.h"
#include "RenderView.h"

namespace platform_ex::Windows::D3D12 {
	FrameBuffer::~FrameBuffer() = default;

	void FrameBuffer::SetRenderTargets()
	{
	}
	void FrameBuffer::BindBarrier()
	{
	}

	void FrameBuffer::UnBindBarrier()
	{
	}

	void FrameBuffer::OnBind() {
		SetRenderTargets();

		for (auto& uav_view : uav_views) {
			static_cast<UnorderedAccessView*>(uav_view.get())->ResetInitCount();
		}
	}
}