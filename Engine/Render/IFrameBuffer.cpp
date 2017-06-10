#include "IFrameBuffer.h"
#include "D3D12\FrameBuffer.h"

namespace platform::Render {
	RenderTargetView::~RenderTargetView() = default;
	DepthStencilView::~DepthStencilView() = default;
	UnorderedAccessView::~UnorderedAccessView() = default;


	FrameBuffer::~FrameBuffer() = default;

	void FrameBuffer::OnBind()
	{
	}

	

	void FrameBuffer::OnUnBind()
	{
	}
}


