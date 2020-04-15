#include "VolumeRendering.h"
#include <LBase/lmathtype.hpp>
#include <Engine/Render/IDevice.h>
#include <Engine/Render/IContext.h>
#include <LBase/smart_ptr.hpp>

using namespace platform;
using namespace platform::Render::Shader;

struct ScreenVertex
{
	leo::math::float2 Position;
	leo::math::float2 UV;
};

std::shared_ptr<Render::GraphicsBuffer> platform::GVolumeRasterizeVertexBuffer()
{
	ScreenVertex DestVertex[4];

	DestVertex[0].Position = leo::math::float2(1, -1);
	DestVertex[0].UV = leo::math::float2(1, 1);

	DestVertex[1].Position = leo::math::float2(1,1);
	DestVertex[1].UV = leo::math::float2(1, 0);

	DestVertex[2].Position = leo::math::float2(-1, -1);
	DestVertex[2].UV = leo::math::float2(0, 1);

	DestVertex[3].Position = leo::math::float2(-1, 1);
	DestVertex[3].UV = leo::math::float2(0, 0);

	return leo::share_raw(Render::Context::Instance().GetDevice().CreateVertexBuffer(Render::Buffer::Usage::Static,
		Render::EAccessHint::EA_GPURead | Render::EAccessHint::EA_Immutable,
		sizeof(DestVertex),
		Render::EF_Unknown, DestVertex));
}
