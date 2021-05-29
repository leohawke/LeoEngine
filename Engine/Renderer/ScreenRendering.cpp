#include "ScreenRendering.h"
#include "Render/IContext.h"
#include <LBase/lmath.hpp>
#include <LBase/smart_ptr.hpp>

using namespace LeoEngine;
using namespace platform;

std::shared_ptr<platform::Render::GraphicsBuffer> LeoEngine::GFullScreenVertexBuffer()
{
	leo::math::float4 DestVertex[] =
	{
		leo::math::float4(-1.0f, 1.0f, 0.0f, 1.0f),
		leo::math::float4(1.0f, 1.0f, 0.0f, 1.0f),
		leo::math::float4(-1.0f, -1.0f, 0.0f, 1.0f),
		leo::math::float4(1.0f, -1.0f, 0.0f, 1.0f),
	};

	static std::shared_ptr<platform::Render::GraphicsBuffer> ClearVertexBuffer
		 = leo::share_raw(Render::Context::Instance().GetDevice().CreateVertexBuffer(Render::Buffer::Usage::Static,
			 Render::EAccessHint::EA_GPURead | Render::EAccessHint::EA_Immutable,
			 sizeof(DestVertex),
			 Render::EF_Unknown, DestVertex));

	return ClearVertexBuffer;
}