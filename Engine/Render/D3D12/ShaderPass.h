#pragma once

#include <LBase/lmemory.hpp>
#include "../IShader.h"
#include "../Effect/Effect.hpp"

namespace platform_ex::Windows::D3D12
{
	class RootSignature;


	//ShaderCompose->ShaderPass
	class GraphicsShaderPass : platform::Render::ShaderPass
	{
	private:
		using ShaderBlob = platform::Render::ShaderBlob;

		ShaderBlob VertexShader;
		ShaderBlob PixelShader;

		leo::observer_ptr<RootSignature> root_signature;

	public:
		RootSignature* RootSignature() const;
	};
}
