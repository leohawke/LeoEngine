#pragma once
#include <LBase/lmath.hpp>
#include <LBase/linttype.hpp>
#include "Math/IntRect.h"
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

	struct SceneMatrices
	{
		struct Initializer
		{
			lm::float4x4 ViewMatrix;
			lm::float4x4 ProjectionMatrix;

			lm::float3 ViewOrigin;
		};

		lm::float4x4 ViewMatrix;
		lm::float4x4 ProjectionMatrix;

		lm::float3 ViewOrigin;

		lm::float4x4 InvViewMatrix;
		lm::float4x4 InvProjectionMatrix;
		lm::float4x4 InvViewProjectionMatrix;
		lm::float4x4 ViewProjectionMatrix;

		SceneMatrices(const Initializer& initializer);

		lm::float4x4 GetInvViewProjectionMatrix() const
		{
			return InvProjectionMatrix;
		}
	};

	class SceneInfo
	{
	public:
		lm::float3 AABBMin;
		lm::float3 AABBMax;

		SceneMatrices Matrices;

		float NearClippingDistance;

		int32 MaxShadowCascades = 10;

		IntRect ViewRect;
		lm::vector2<int> BufferSize;

		platform::Render::Texture2D* SceneDepth;

		const SceneParameters& GetParameters() const
		{
			return Parameters;
		}

		void SetupParameters()
		{
			SetupParameters(BufferSize,Matrices, Parameters);
		}
	private:
		void SetupParameters(
			lm::vector2<int> BufferSize,
			const SceneMatrices& Matrices,
			SceneParameters& Parameters
		);
	private:
		SceneParameters Parameters;
	};
}