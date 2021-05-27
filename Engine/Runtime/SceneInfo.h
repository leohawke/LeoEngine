#pragma once
#include <LBase/lmath.hpp>
#include <LBase/linttype.hpp>
#include "Render/ShaderParamterTraits.hpp"
#include "Engine/Render/ShaderTextureTraits.hpp"

namespace LeoEngine
{
	namespace lm = leo::math;
	using namespace leo::inttype;

	BEGIN_SHADER_PARAMETER_STRUCT(SceneParameters)
		SHADER_PARAMETER(lm::float4x4, ScreenToWorld)
		SHADER_PARAMETER(lm::float4, BufferSizeAndInvSize)
		SHADER_PARAMETER(lm::float4, ScreenPositionScaleBias)
		SHADER_PARAMETER_TEXTURE(platform::Render::Texture2D, SceneDepthTexture)
		SHADER_PARAMETER_SAMPLER(platform::Render::TextureSampleDesc, SceneDepthTextureSampler)
	END_SHADER_PARAMETER_STRUCT();

	class SceneInfo
	{
	public:
		lm::float3 AABBMin;
		lm::float3 AABBMax;

		lm::float4x4 ViewMatrix;
		lm::float4x4 ProjectionMatrix;

		lm::float3 ViewOrigin;

		float NearClippingDistance;

		int32 MaxShadowCascades = 10;

		lm::vector2<int> BufferSize;

		const SceneParameters& GetParameters() const
		{
			return Parameters;
		}
	private:
		SceneParameters Parameters;
	};
}