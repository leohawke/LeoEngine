#include "IGPUResourceView.h"

namespace platform::Render {

	GPUView::GPUView(uint16 width_, uint16 height_, EFormat format_)
		:width(width_),height(height_),format(format_)
	{
	}

	GPUView::~GPUView() = default;

	uint16 GPUView::Width() const
	{
		return width;
	}

	uint16 GPUView::Height() const
	{
		return height;
	}

	EFormat GPUView::Format() const
	{
		return format;
	}

	RenderTargetView::~RenderTargetView() = default;
	DepthStencilView::~DepthStencilView() = default;
	UnorderedAccessView::~UnorderedAccessView() = default;
}