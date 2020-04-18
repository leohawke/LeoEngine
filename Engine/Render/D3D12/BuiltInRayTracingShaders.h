#pragma once

#include "../BuiltInRayTracingShader.h"
#include <list>

namespace platform_ex::Windows::D3D12 {

	using namespace platform::Render::Shader;

	template<typename ShaderType>
	inline platform::Render::RayTracingShader* GetBuildInRayTracingShader()
	{
		auto ShaderMap = GetGlobalShaderMap();

		auto Shader = ShaderMap->GetShader<ShaderType>();

		auto RayTracingShader = Shader->GetRayTracingShader();

		return RayTracingShader;
	}

	class DefaultCHS :public BuiltInRayTracingShader
	{
		EXPORTED_SHADER_TYPE(DefaultCHS);
	};

	class DefaultMS :public BuiltInRayTracingShader
	{
		EXPORTED_SHADER_TYPE(DefaultMS);
	};

	class ShadowRG : public BuiltInRayTracingShader
	{
		EXPORTED_SHADER_TYPE(ShadowRG);
	};
}
