#include "LBase/smart_ptr.hpp"
#include "CommonRenderResources.h"
#include "IContext.h"

using namespace platform::Render;

VertexDeclarationElements platform::Render::GFilterVertexDeclaration()
{
	static VertexDeclarationElements Elements =
	{ 
		CtorVertexElement(0,loffsetof(FilterVertex,Position),Render::Vertex::Usage::Position,0,Render::EF_ABGR32F,sizeof(FilterVertex)),
		CtorVertexElement(0,loffsetof(FilterVertex,UV),Render::Vertex::Usage::TextureCoord,0,Render::EF_GR32F,sizeof(FilterVertex)),
	};

    return Elements;
}

std::shared_ptr<GraphicsBuffer> platform::Render::GScreenRectangleVertexBuffer()
{
	static struct ScreenRectangleVertexBuffer
	{
		ScreenRectangleVertexBuffer()
		{
			FilterVertex Vertices[6];
			Vertices[0].Position = leo::math::float4(1, 1, 0, 1);
			Vertices[0].UV = leo::math::float2(1, 1);

			Vertices[1].Position = leo::math::float4(0, 1, 0, 1);
			Vertices[1].UV = leo::math::float2(0, 1);

			Vertices[2].Position = leo::math::float4(1, 0, 0, 1);
			Vertices[2].UV = leo::math::float2(1, 0);

			Vertices[3].Position = leo::math::float4(0, 0, 0, 1);
			Vertices[3].UV = leo::math::float2(0, 0);

			//The final two vertices are used for the triangle optimization (a single triangle spans the entire viewport )
			Vertices[4].Position = leo::math::float4(-1, 1, 0, 1);
			Vertices[4].UV = leo::math::float2(-1, 1);

			Vertices[5].Position = leo::math::float4(1, -1, 0, 1);
			Vertices[5].UV = leo::math::float2(1, -1);

			VertexBuffer = leo::share_raw(Render::Context::Instance().GetDevice().CreateVertexBuffer(Render::Buffer::Usage::Static,
				Render::EAccessHint::EA_GPURead | Render::EAccessHint::EA_Immutable,
				sizeof(Vertices),
				Render::EF_Unknown, Vertices));
		}

		std::shared_ptr<Render::GraphicsBuffer> VertexBuffer;
	} Buffer;

	return Buffer.VertexBuffer;
}

std::shared_ptr<GraphicsBuffer> platform::Render::GScreenRectangleIndexBuffer()
{
	static struct ScreenRectangleIndexBuffer
	{
		ScreenRectangleIndexBuffer()
		{
			// Indices 0 - 5 are used for rendering a quad. Indices 6 - 8 are used for triangle optimization.
			const uint16 Indices[] = { 0, 1, 2, 2, 1, 3, 0, 4, 5 };

			IndexBuffer = leo::share_raw(Render::Context::Instance().GetDevice().CreateIndexBuffer(Render::Buffer::Usage::Static,
				Render::EAccessHint::EA_GPURead | Render::EAccessHint::EA_Immutable,
				sizeof(Indices),
				Render::EF_R16UI, Indices));
		}

		std::shared_ptr<Render::GraphicsBuffer> IndexBuffer;
	} Buffer;

	return Buffer.IndexBuffer;
}
