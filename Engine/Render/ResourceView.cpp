#include "IGPUResourceView.h"

namespace platform::Render {

	RenderTargetView::~RenderTargetView() = default;
	DepthStencilView::~DepthStencilView() = default;
	UnorderedAccessView::~UnorderedAccessView() = default;
	ShaderResourceView::~ShaderResourceView() = default;
}