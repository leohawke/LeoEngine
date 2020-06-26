#include "IGPUResourceView.h"
#include "IFormat.hpp"

namespace platform::Render {

	const ClearValueBinding ClearValueBinding::Black {};
	const ClearValueBinding ClearValueBinding::DepthOne{ 1.0f, 0 };

	UnorderedAccessView::~UnorderedAccessView() = default;
	ShaderResourceView::~ShaderResourceView() = default;
}