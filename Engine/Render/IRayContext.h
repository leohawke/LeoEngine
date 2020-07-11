/*! \file Engine\Render\IRayContext.h
\ingroup Engine
\brief 射线执行接口类。
*/
#ifndef LE_RENDER_IRayContext_h
#define LE_RENDER_IRayContext_h 1

#include "IRayDevice.h"
#include "IFrameBuffer.h"

namespace platform::Render {

	struct GenShadowConstants
	{
		leo::math::float3 LightDirection;
		float SourceRadius;
		leo::uint32 SamplesPerPixel;
		leo::uint32 StateFrameIndex;
		leo::uint32 padding[2];
		leo::math::float4x4 CameraToWorld;
		leo::math::float2 Resolution;
		leo::math::float2 Padding1;
	};

	static_assert(loffsetof(GenShadowConstants, CameraToWorld) % 16 == 0);

	class RayContext
	{
	public:
		virtual RayDevice& GetDevice() = 0;

		virtual void RayTraceShadow(RayTracingScene* InScene, FrameBuffer* InDepth, UnorderedAccessView* Ouput, GraphicsBuffer* InConstants) =0;
	};
}

#endif
