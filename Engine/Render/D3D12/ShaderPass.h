#pragma once

#include <LBase/lmemory.hpp>
#include "../Shader.h"
#include "../Effect/Effect.hpp"

namespace platform_ex::Windows::D3D12
{
	class RootSignature;


	//ShaderCompose->ShaderPass
	class GraphicsShaderPass : platform::Render::ShaderPass
	{
	private:
		using ShaderBlob = platform::Render::ShaderBlob;

		leo::observer_ptr<RootSignature> root_signature;
	public:
		ShaderBlob VertexShader;
		ShaderBlob PixelShader;

		std::optional<ShaderBlob> GeometryShader;

		std::optional<ShaderBlob> HullShader;
		std::optional<ShaderBlob> DomainShader;
	public:
		RootSignature* RootSignature() const;
	};
}
