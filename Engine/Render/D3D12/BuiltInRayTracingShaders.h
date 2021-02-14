#pragma once

#include "../BuiltInRayTracingShader.h"
#include "Engine/Render/ShaderParamterTraits.hpp"
#include "Engine/Render/ShaderTextureTraits.hpp"
#include "Engine/Render/ShaderParameterStruct.h"
#include <list>

namespace platform_ex::Windows::D3D12 {

	using namespace platform::Render::Shader;

	template<typename ShaderType>
	inline platform::Render::RayTracingShader* GetBuildInRayTracingShader()
	{
		auto ShaderMap = GetBuiltInShaderMap();

		auto Shader = ShaderMap->GetShader<ShaderType>();

		auto RayTracingShader = Shader->GetRayTracingShader();

		return RayTracingShader;
	}

	class DefaultCHS :public BuiltInRayTracingShader
	{
		EXPORTED_RAYTRACING_SHADER(DefaultCHS);
	};

	class DefaultMS :public BuiltInRayTracingShader
	{
		EXPORTED_RAYTRACING_SHADER(DefaultMS);
	};

	class ShadowRG : public BuiltInRayTracingShader
	{
	public:
		BEGIN_SHADER_PARAMETER_STRUCT(Parameters)
			SHADER_PARAMETER(leo::math::float4x4, SVPositionToWorld)
			SHADER_PARAMETER(leo::math::float3, WorldCameraOrigin)
			SHADER_PARAMETER(leo::math::float4, BufferSizeAndInvSize)
			SHADER_PARAMETER(float, NormalBias)
			SHADER_PARAMETER_TEXTURE(platform::Render::Texture2D, WorldNormalBuffer)
			END_SHADER_PARAMETER_STRUCT();

		EXPORTED_RAYTRACING_SHADER(ShadowRG);
	};
}
