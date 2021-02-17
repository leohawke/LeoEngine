/*! \file Engine\Render\IRayContext.h
\ingroup Engine
\brief 射线执行接口类。
*/
#ifndef LE_RENDER_IRayContext_h
#define LE_RENDER_IRayContext_h 1

#include "IRayDevice.h"
#include "IFrameBuffer.h"
#include "ShaderParamterTraits.hpp"
#include "ShaderTextureTraits.hpp"

namespace platform::Render {

	BEGIN_SHADER_PARAMETER_STRUCT(ShadowRGParameters)
		SHADER_PARAMETER(leo::math::float4x4, SVPositionToWorld)
		SHADER_PARAMETER(leo::math::float3, WorldCameraOrigin)
		SHADER_PARAMETER(leo::math::float4, BufferSizeAndInvSize)
		SHADER_PARAMETER(float, NormalBias)
		SHADER_PARAMETER(leo::math::float3, LightDirection)
		SHADER_PARAMETER(float, SourceRadius)
		SHADER_PARAMETER(unsigned, SamplesPerPixel)
		SHADER_PARAMETER(unsigned, StateFrameIndex)
		SHADER_PARAMETER_TEXTURE(platform::Render::Texture2D, WorldNormalBuffer)
		SHADER_PARAMETER_TEXTURE(platform::Render::Texture2D, Depth)
		SHADER_PARAMETER_TEXTURE(platform::Render::RWTexture2D, Output)
		END_SHADER_PARAMETER_STRUCT();

	class RayContext
	{
	public:
		virtual RayDevice& GetDevice() = 0;

		virtual void RayTraceShadow(RayTracingScene* InScene,const ShadowRGParameters& InConstants) =0;
	};
}

#endif
