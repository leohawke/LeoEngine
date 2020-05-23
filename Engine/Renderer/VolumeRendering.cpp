#include "VolumeRendering.h"
#include <LBase/lmathtype.hpp>
#include <Engine/Render/IDevice.h>
#include <Engine/Render/IContext.h>
#include <LBase/smart_ptr.hpp>

using namespace platform;
using namespace platform::Render::Shader;

IMPLEMENT_SHADER(WriteToSliceVS, "PostProcess/VolumeRendering.lsl", "WriteToSliceMainVS", platform::Render::VertexShader);
IMPLEMENT_SHADER(WriteToSliceGS, "PostProcess/VolumeRendering.lsl", "WriteToSliceMainGS", platform::Render::GeometryShader);


struct ScreenVertex
{
	leo::math::float2 Position;
	leo::math::float2 UV;
};

std::shared_ptr<Render::GraphicsBuffer> platform::GVolumeRasterizeVertexBuffer()
{
	static struct VolumeRasterizeVertexBuffer
	{
		VolumeRasterizeVertexBuffer()
		{
			ScreenVertex DestVertex[4];

			DestVertex[0].Position = leo::math::float2(1, -1);
			DestVertex[0].UV = leo::math::float2(1, 1);

			DestVertex[1].Position = leo::math::float2(1, 1);
			DestVertex[1].UV = leo::math::float2(1, 0);

			DestVertex[2].Position = leo::math::float2(-1, -1);
			DestVertex[2].UV = leo::math::float2(0, 1);

			DestVertex[3].Position = leo::math::float2(-1, 1);
			DestVertex[3].UV = leo::math::float2(0, 0);

			VettexBuffer = leo::share_raw(Render::Context::Instance().GetDevice().CreateVertexBuffer(Render::Buffer::Usage::Static,
				Render::EAccessHint::EA_GPURead | Render::EAccessHint::EA_Immutable,
				sizeof(DestVertex),
				Render::EF_Unknown, DestVertex));
		}

		std::shared_ptr<Render::GraphicsBuffer> VettexBuffer;
	} Buffer;

	return Buffer.VettexBuffer;
}

Render::VertexDeclarationElements platform::GScreenVertexDeclaration()
{
	constexpr std::array<Render::VertexElement, 2> Elements =
	{ {
		{0,loffsetof(ScreenVertex,Position),Render::Vertex::Usage::Position,0,Render::EF_BGR32F,sizeof(ScreenVertex)},
		{0,loffsetof(ScreenVertex,UV),Render::Vertex::Usage::TextureCoord,0,Render::EF_GR32F,sizeof(ScreenVertex)},
	} };

	return { Elements.begin(),Elements.end()};
}

void platform::RasterizeToVolumeTexture(Render::CommandList& CmdList, VolumeBounds VolumeBounds)
{
	CmdList.SetViewport(VolumeBounds.MinX, VolumeBounds.MinY, 0, VolumeBounds.MaxX, VolumeBounds.MaxY, 0);
	CmdList.SetVertexBuffer(0, GVolumeRasterizeVertexBuffer().get());

	const auto NumInstances = VolumeBounds.MaxZ - VolumeBounds.MinZ;

	CmdList.DrawPrimitive(0, 2, NumInstances);
}
