#include "PipleState.h"
#include "Convert.h"

namespace platform_ex::Windows::D3D12 {
	PipleState::PipleState(const base & state)
		:base(state)
	{
		BlendState = Convert(base::BlendState);
		RasterizerState = Convert(base::RasterizerState);
		DepthStencilState = Convert(base::DepthStencilState);
	}
}