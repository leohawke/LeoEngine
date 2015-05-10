#include "..\DeviceMgr.h"
#include "..\d3dx11.hpp"
#include "Deferred.h"
#include "..\ShaderMgr.h"
#include "..\RenderStates.hpp"
#include "leomathutility.hpp"
#include "..\Core\FileSearch.h"
#include "..\Core\EngineConfig.h"
#include "..\Core\Vertex.hpp"
#include "..\Core\Camera.hpp"
#include "..\exception.hpp"

namespace {
	ID3D11ShaderResourceView* mSRVs[2] = { nullptr,nullptr };
	ID3D11RenderTargetView* mMRTs[2] = { nullptr,nullptr };
	ID3D11VertexShader* mIAVS = nullptr;
	ID3D11SamplerState* trilinearSampler = nullptr;
	ID3D11SamplerState* normalDepthSampler = nullptr;

	ID3D11ShaderResourceView* mSSAOSRV = nullptr;
	ID3D11RenderTargetView* mSSAPRTV = nullptr;

	ID3D11Buffer* mIAVB = nullptr;
	ID3D11InputLayout* mIALayout = nullptr;

	struct GBufferIAVertex {
		leo::float4 PosH;//POSITION;
		leo::float3 ToFarPlane;//TEXCOORD0;
		leo::float2 Tex;//TEXCOORD1;
	};

	extern const D3D11_INPUT_ELEMENT_DESC GBufferIA[3]
		=
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, loffsetof(GBufferIAVertex, PosH), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, loffsetof(GBufferIAVertex, ToFarPlane), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, loffsetof(GBufferIAVertex, Tex), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	using size_type = std::pair<leo::uint16, leo::uint16>;
}

using namespace leo;


DeferredResources::DeferredResources() noexcept {
	DeviceMgr mgr;
	auto size = mgr.GetClientSize();
	ReSize(size);

	leo::ShaderMgr sm;
	mIAVS = sm.CreateVertexShader(
		FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"deferred", D3D11_VERTEX_SHADER)), 
		nullptr,
		GBufferIA,arrlen(GBufferIA), 
		&mIALayout);

	leo::RenderStates ss;
	trilinearSampler = ss.GetSamplerState(L"trilinearSampler");
	normalDepthSampler = ss.GetSamplerState(L"DepthMap");

}

ID3D11RenderTargetView** DeferredResources::GetMRTs() const {
	return mMRTs;
}
ID3D11ShaderResourceView** DeferredResources::GetSRVs() const {
	return mSRVs;

}

void DeferredResources::ReSize(const size_type& size) noexcept {

	DeviceMgr mgr;
	auto device = mgr.GetDevice();

	D3D11_TEXTURE2D_DESC GBufferDesc;
	GBufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	GBufferDesc.ArraySize = 1;
	GBufferDesc.MipLevels = 1;

	GBufferDesc.SampleDesc.Count = 1;
	GBufferDesc.SampleDesc.Quality = 0;

	GBufferDesc.Width = size.first;
	GBufferDesc.Height = size.second;

	GBufferDesc.Usage = D3D11_USAGE_DEFAULT	;
	GBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	GBufferDesc.CPUAccessFlags = 0;
	GBufferDesc.MiscFlags = 0;

	char debugSRVName[] = "DeferredSRVS";
	char debugMRTName[] = "DeferredMRTS";

	for (auto i = 0u; i != 2; ++i) {
		if (mSRVs[i])
			mSRVs[i]->Release();
		if (mMRTs[i])
			mMRTs[i]->Release();

		ID3D11Texture2D* mTex;
		device->CreateTexture2D(&GBufferDesc, nullptr, &mTex);
		device->CreateShaderResourceView(mTex, nullptr, mSRVs+i);
		device->CreateRenderTargetView(mTex, nullptr, mMRTs + i);
		debugSRVName[arrlen(debugSRVName) - 2] = i + '0';
		debugMRTName[arrlen(debugMRTName) - 2] = i + '0';
		dx::DebugCOM(mSRVs[i],debugSRVName);
		dx::DebugCOM(mMRTs[i],debugMRTName);

		mTex->Release();
	}


	D3D11_TEXTURE2D_DESC SSAOTexDesc;
#ifdef DEBUG
	SSAOTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
#else
	SSAOTexDesc.Format = DXGI_FORMAT_R32_FLOAT;;
#endif
	SSAOTexDesc.ArraySize = 1;
	SSAOTexDesc.MipLevels = 1;

	SSAOTexDesc.SampleDesc.Count = 1;
	SSAOTexDesc.SampleDesc.Quality = 0;

	SSAOTexDesc.Width = size.first/2;
	SSAOTexDesc.Height = size.second/2;

	SSAOTexDesc.Usage = D3D11_USAGE_DEFAULT;
	SSAOTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	SSAOTexDesc.CPUAccessFlags = 0;
	SSAOTexDesc.MiscFlags = 0;

	using leo::win::make_scope_com;
	auto mTex = make_scope_com<ID3D11Texture2D>();
	device->CreateTexture2D(&SSAOTexDesc, nullptr, &mTex);
	mSSAOSRV? mSSAOSRV->Release():0;
	mSSAPRTV? mSSAPRTV->Release():0;
	device->CreateShaderResourceView(mTex, nullptr, &mSSAOSRV);
	device->CreateRenderTargetView(mTex, nullptr, &mSSAPRTV);
	mTex->Release();
}

DeferredResources::~DeferredResources() {
	for (auto i = 0u; i != 2; ++i) {
		if (mSRVs[i])
			leo::win::ReleaseCOM(mSRVs[i]);
		if (mMRTs[i])
			leo::win::ReleaseCOM(mMRTs[i]);
	}
	leo::win::ReleaseCOM(mSSAOSRV);
	leo::win::ReleaseCOM(mSSAPRTV);
	leo::win::ReleaseCOM(mIAVB);
}

void DeferredResources::SetFrustum(const CameraFrustum& frustum) noexcept {
	static GBufferIAVertex vertexs[4] = {
		{ float4(+1.f, +1.f, 1.f, 1.f),float3(0.f,0.f,0.f),float2(1.f,0.f)},
		{ float4(+1.f, -1.f, 1.f, 1.f),float3(0.f,0.f,0.f),float2(1.f,1.f) },
		{ float4(-1.f, +1.f, 1.f, 1.f),float3(0.f,0.f,0.f),float2(0.f,0.f) },
		{ float4(-1.f, -1.f, 1.f, 1.f),float3(0.f,0.f,0.f),float2(0.f,1.f) }
	};

	auto aspect = frustum.GetAspect();
	auto farZ = frustum.mFar;
	auto halfHeight = farZ*tanf(0.5f*frustum.GetFov());
	auto halfWidth = aspect*halfHeight;

	vertexs[0].ToFarPlane = float3(+halfWidth, +halfHeight, farZ);
	vertexs[1].ToFarPlane = float3(+halfWidth, -halfHeight, farZ);
	vertexs[2].ToFarPlane = float3(-halfWidth, +halfHeight, farZ);
	vertexs[3].ToFarPlane = float3(-halfWidth, -halfHeight, farZ);

	leo::win::ReleaseCOM(mIAVB);
	
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;
	vbDesc.ByteWidth = static_cast<win::uint> (sizeof(GBufferIAVertex)*arrlen(vertexs));

	D3D11_SUBRESOURCE_DATA resDesc;
	resDesc.pSysMem = &vertexs[0];

	leo::DeviceMgr dm;
	try {
		dxcall(dm.GetDevice()->CreateBuffer(&vbDesc, &resDesc, &mIAVB));
		dx::DebugCOM(mIAVB, "GBuFFInputVertexBuffer");
	}
	Catch_DX_Exception
}

void DeferredResources::OMSet() noexcept {
	leo::DeviceMgr().GetDeviceContext()->OMSetRenderTargets(arrlen(mMRTs), mMRTs, leo::DeviceMgr().GetDepthStencilView());

	float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.8f };
	leo::DeviceMgr().GetDeviceContext()->ClearRenderTargetView(mMRTs[0], ClearColor);
	leo::DeviceMgr().GetDeviceContext()->ClearRenderTargetView(mMRTs[1], ClearColor);
	leo::DeviceMgr().GetDeviceContext()->ClearDepthStencilView(leo::DeviceMgr().GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
}

void DeferredResources::IASet() noexcept {
	//ID3D11RenderTargetView* nullMRTs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
	auto context = leo::DeviceMgr().GetDeviceContext();

	//context->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT
	ID3D11RenderTargetView* rtvs[] = { nullptr,nullptr };
	context->OMSetRenderTargets(2, rtvs, nullptr);

	UINT strides[] = { sizeof(GBufferIAVertex) };
	UINT offsets[] = { 0 };
	context->IASetVertexBuffers(0, 1, &mIAVB,strides , offsets);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetInputLayout(mIALayout);

	context->VSSetShader(mIAVS, nullptr, 0);

	ID3D11SamplerState* mpsss[] =
	{trilinearSampler,normalDepthSampler};
	context->PSSetSamplers(0,2,mpsss);
	context->PSSetShaderResources(0, 2, mSRVs);
}


void DeferredResources::UnIASet() noexcept {
	ID3D11ShaderResourceView* pssrvs[] = { nullptr,nullptr};

	leo::DeviceMgr().GetDeviceContext()->PSSetShaderResources(0, arrlen(pssrvs), pssrvs);
}

ID3D11RenderTargetView* DeferredResources::GetSSAORTV() const {
	return mSSAPRTV;
}
ID3D11ShaderResourceView* DeferredResources::GetSSAOSRV() const {
	return mSSAOSRV;
}

