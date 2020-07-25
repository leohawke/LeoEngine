#include "PipleState.h"
#include "Convert.h"
#include "FrameBuffer.h"
#include "InputLayout.hpp"
#include "ShaderCompose.h"
#include "Context.h"
#include "Engine/Core/Hash/CityHash.h"

namespace platform_ex::Windows::D3D12 {
	PipleState::PipleState(const base & state)
		:base(state)
	{
		graphics_ps_desc.BlendState = Convert(base::BlendState);
		graphics_ps_desc.RasterizerState = Convert(base::RasterizerState);
		graphics_ps_desc.DepthStencilState = Convert(base::DepthStencilState);
		graphics_ps_desc.SampleMask = base::BlendState.sample_mask;
		graphics_ps_desc.NodeMask = 0;
		graphics_ps_desc.CachedPSO.pCachedBlob = nullptr;
		graphics_ps_desc.CachedPSO.CachedBlobSizeInBytes = 0;
		graphics_ps_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	}
}