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
	leo::observer_ptr<GPUView> FrameBuffer::Attached(Attachment which) const
	{
		switch (which)	
		{
		case platform::Render::FrameBuffer::DepthStencil:
			return leo::make_observer(ds_view.get());
		default:
			return leo::make_observer(clr_views[which].get());
			break;
		}
		throw std::invalid_argument("don't support enum argument");
	}
}


