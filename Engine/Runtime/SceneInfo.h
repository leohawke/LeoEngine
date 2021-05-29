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

		SceneMatrices()
		{}
	private:
		lm::float4x4 ViewMatrix;
		lm::float4x4 ProjectionMatrix;

		lm::float3 ViewOrigin;

		lm::float4x4 InvViewMatrix;
		lm::float4x4 InvProjectionMatrix;
		lm::float4x4 InvViewProjectionMatrix;
		lm::float4x4 ViewProjectionMatrix;
	public:
		SceneMatrices(const Initializer& initializer);

		inline const lm::float4x4& GetInvViewProjectionMatrix() const
		{
			return InvViewProjectionMatrix;
		}

		inline const lm::float4x4& GetProjectionMatrix() const
		{
			return ProjectionMatrix;
		}

		inline const lm::float4x4& GetInvProjectionMatrix() const
		{
			return InvProjectionMatrix;
		}

		inline const lm::float4x4& GetViewMatrix() const
		{
			return ViewMatrix;
		}

		lm::float3 GetViewOrigin() const
		{
			return ViewOrigin;
		}
	};

	inline lm::float4 CreateInvDeviceZToWorldZTransform(const lm::float4x4& ProjMatrix)
	{
		// The perspective depth projection comes from the the following projection matrix:
		//
		// | 1  0  0  0 |
		// | 0  1  0  0 |
		// | 0  0  A  1 |
		// | 0  0  B  0 |
		//
		// Z' = (Z * A + B) / Z
		// Z' = A + B / Z
		//
		// So to get Z from Z' is just:
		// Z = B / (Z' - A)
		//
		// Note a reversed Z projection matrix will have A=0.
		//
		// Done in shader as:
		// Z = 1 / (Z' * C1 - C2)   --- Where C1 = 1/B, C2 = A/B
		//

		float DepthMul = ProjMatrix[2][2];
		float DepthAdd = ProjMatrix[3][2];

		if (DepthAdd == 0.f)
		{
			// Avoid dividing by 0 in this case
			DepthAdd = 0.00000001f;
		}

		// perspective
		// SceneDepth = 1.0f / (DeviceZ / ProjMatrix[3][2] - ProjMatrix[2][2] / ProjMatrix[3][2])

		// ortho
		// SceneDepth = DeviceZ / ProjMatrix[2][2] - ProjMatrix[3][2] / ProjMatrix[2][2];

		bool bIsPerspectiveProjection = ProjMatrix[3][3] < 1.0f;

		if (bIsPerspectiveProjection)
		{
			float SubtractValue = DepthMul / DepthAdd;

			// Subtract a tiny number to avoid divide by 0 errors in the shader when a very far distance is decided from the depth buffer.
			// This fixes fog not being applied to the black background in the editor.
			SubtractValue -= 0.00000001f;

			return lm::float4(
				0.0f,
				0.0f,
				1.0f / DepthAdd,
				SubtractValue
			);
		}
		else
		{
			return lm::float4(
				1.0f / ProjMatrix[2][2],
				-ProjMatrix[3][2] / ProjMatrix[2][2] + 1.0f,
				0.0f,
				1.0f
			);
		}
	}

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