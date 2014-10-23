#include <core\Camera.hpp>
#include <Core\EffectLine.hpp>
#include <Core\Vertex.hpp>
#include <ShaderMgr.h>
#include <d3dx11.hpp>
#include <exception.hpp>

#include "Axis.hpp"
namespace leo
{
	Axis::Axis(ID3D11Device* device)
	{
		CD3D11_BUFFER_DESC vbDesc(sizeof(float4)*6,D3D11_BIND_VERTEX_BUFFER,D3D11_USAGE_IMMUTABLE);

		static const float4 vertexdata[] = 
		{
			float4(-100.f, 0.f, 0.f, 1.f),
			float4(100.f, 0.f, 0.f, 1.f),
			float4(0.f, -100.f, 0.f, 1.f),
			float4(0.f, 100.f, 0.f, 1.f),
			float4(0.f, 0.f, -100.f, 1.f),
			float4(0.f, 0.f, 100.f, 1.f)
		};
		
		D3D11_SUBRESOURCE_DATA vbResData;
		vbResData.pSysMem = vertexdata;

		try
		{
			leo::EffectLine::GetInstance(device);
			dxcall(device->CreateBuffer(&vbDesc, &vbResData, &mVertexBuffer));
		}
		Catch_DX_Exception
	}

	Axis::~Axis()
	{
		win::ReleaseCOM(mVertexBuffer);
	}

	void Axis::Render(ID3D11DeviceContext* context, const Camera& camera)
	{

		UINT strides[] = { sizeof(float4) };
		UINT offsets[] = { 0 };

		context->IASetVertexBuffers(0, 1, &mVertexBuffer, strides, offsets);
		context->IASetInputLayout(ShaderMgr().CreateInputLayout(InputLayoutDesc::Sky));
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		auto& pEffect = EffectLine::GetInstance();

		pEffect->ViewProj(camera.ViewProj());
		pEffect->Color(float4(1.f, 0.f, 0.f, 1.f));
		pEffect->Apply(context);

		context->Draw(2, 0);
		pEffect->Color(float4(0.f, 1.f, 0.f, 1.f),context);
		context->Draw(2, 2);
		pEffect->Color(float4(0.f, 0.f, 1.f, 1.f), context);
		context->Draw(2, 4);
	}

	const std::unique_ptr<Axis>& Axis::GetInstance(ID3D11Device* device)
	{
		static auto mInstance = unique_raw(new Axis(device));
		return mInstance;
	}
}