#include "EffectQuad.hpp"
#include "Camera.hpp"
#include "FileSearch.h"
#include "EngineConfig.h"
#include <RenderSystem\ShaderMgr.h>
using namespace leo;

struct QUADVertex {
	leo::float4 PosH;//POSITION;
	leo::float3 ToFarPlane;//TEXCOORD0;
	leo::float2 Tex;//TEXCOORD1;
};

extern const D3D11_INPUT_ELEMENT_DESC QUADIA[3]
=
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, loffsetof(QUADVertex, PosH), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, loffsetof(QUADVertex, ToFarPlane), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, loffsetof(QUADVertex, Tex), D3D11_INPUT_PER_VERTEX_DATA, 0 },
};


static QUADVertex vertexs[4] = {
	{ float4(+1.f, +1.f, 0.9999999f, 1.f),float3(0.f,0.f,0.f),float2(1.f,0.f) },
	{ float4(+1.f, -1.f, 0.9999999f, 1.f),float3(0.f,0.f,0.f),float2(1.f,1.f) },
	{ float4(-1.f, +1.f, 0.9999999f, 1.f),float3(0.f,0.f,0.f),float2(0.f,0.f) },
	{ float4(-1.f, -1.f, 0.9999999f, 1.f),float3(0.f,0.f,0.f),float2(0.f,1.f) }
};

class EffectQuadDelegate : CONCRETE(EffectQuad), public leo::Singleton < EffectQuadDelegate >
{
public:
	EffectQuadDelegate(ID3D11Device* device)		
	{
		using namespace leo;
		ShaderMgr sm;
		mQUADVS = sm.CreateVertexShader(
			FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"quad", D3D11_VERTEX_SHADER)),
			nullptr,
			QUADIA, arrlen(QUADIA),
			&mQUADIALayout);
	}
	~EffectQuadDelegate()
	{}
public:
	void Apply(ID3D11DeviceContext* context)
	{
		UINT strides[] = { sizeof(QUADVertex) };
		UINT offsets[] = { 0 };
		context->IASetVertexBuffers(0, 1, &mQUADVB, strides, offsets);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		context->IASetInputLayout(mQUADIALayout);
		context->RSSetState(nullptr);
		context->VSSetShader(mQUADVS, nullptr, 0);
	}

	void SetFrustum(ID3D11Device* device, const leo::CameraFrustum & frustum) {
		auto aspect = frustum.GetAspect();
		auto farZ = frustum.mFar;
		auto halfHeight = farZ*tanf(0.5f*frustum.GetFov());
		auto halfWidth = aspect*halfHeight;

		vertexs[0].ToFarPlane = float3(+halfWidth, +halfHeight, farZ);
		vertexs[1].ToFarPlane = float3(+halfWidth, -halfHeight, farZ);
		vertexs[2].ToFarPlane = float3(-halfWidth, +halfHeight, farZ);
		vertexs[3].ToFarPlane = float3(-halfWidth, -halfHeight, farZ);


		D3D11_BUFFER_DESC vbDesc;
		vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.CPUAccessFlags = 0;
		vbDesc.MiscFlags = 0;
		vbDesc.StructureByteStride = 0;
		vbDesc.ByteWidth = static_cast<win::UINT> (sizeof(QUADVertex)*arrlen(vertexs));

		D3D11_SUBRESOURCE_DATA resDesc;
		resDesc.pSysMem = &vertexs[0];

		try {
			mQUADVB ? mQUADVB->Release():0;
			dxcall(device->CreateBuffer(&vbDesc, &resDesc, &mQUADVB));
			dx::DebugCOM(mQUADVB, "QUADVertexBuffer");
		}
		Catch_DX_Exception
	}

	void Draw(ID3D11DeviceContext * context) {
		context->Draw(4, 0);
	}

private:
	//DeferredVS.cso
	ID3D11VertexShader* mQUADVS = nullptr;

	win::unique_com<ID3D11Buffer> mQUADVB = nullptr;
	ID3D11InputLayout* mQUADIALayout = nullptr;
};

void leo::EffectQuad::Apply(ID3D11DeviceContext * context)
{
	lassume(dynamic_cast<EffectQuadDelegate*>(this));

	return ((EffectQuadDelegate*)this)->Apply(
		context
		);
}

void leo::EffectQuad::Draw(ID3D11DeviceContext * context)
{
	lassume(dynamic_cast<EffectQuadDelegate*>(this));

	return ((EffectQuadDelegate*)this)->Draw(
		context
		);
}

void leo::EffectQuad::SetFrustum(ID3D11Device* device, const leo::CameraFrustum & frustum)
{
	lassume(dynamic_cast<EffectQuadDelegate*>(this));

	return ((EffectQuadDelegate*)this)->SetFrustum(
		device,frustum
		);
}

EffectQuad & leo::EffectQuad::GetInstance(ID3D11Device * device)
{
	//TODO:fix that
	static EffectQuadDelegate mInstances{ device};
	return mInstances;
}
