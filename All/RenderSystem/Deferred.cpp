#include "..\DeviceMgr.h"
#include "..\d3dx11.hpp"
#include "Deferred.h"
#include "..\ShaderMgr.h"
#include "..\RenderStates.hpp"
#include "..\leomath.hpp"
#include "..\Core\FileSearch.h"
#include "..\Core\EngineConfig.h"

namespace {
	ID3D11ShaderResourceView* mSRVs[2] = { nullptr,nullptr };
	ID3D11RenderTargetView* mMRTs[2] = { nullptr,nullptr };
	ID3D11VertexShader* mIAVS = nullptr;
	ID3D11SamplerState* trilinearSampler = nullptr;
	ID3D11SamplerState* normalDepthSampler = nullptr;

	ID3D11ShaderResourceView* mSSAOSRV = nullptr;
	ID3D11RenderTargetView* mSSAPRTV = nullptr;
	ID3D11PixelShader* mSSAOPS = nullptr;

	ID3D11InputLayout* mIALayout = nullptr;

	struct GBufferIAVertex {
		leo::float4 PosH;//POSITION;
		leo::float3 ToFarPlane;//TEXCOORD0;
		leo::float2 Tex;//TEXCOORD1;
	};

	extern const D3D11_INPUT_ELEMENT_DESC GBufferIA[4]
		=
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, loffsetof(GBufferIAVertex, PosH), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, loffsetof(GBufferIAVertex, ToFarPlane), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, loffsetof(GBufferIAVertex, Tex), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
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

	mSSAOPS = sm.CreatePixelShader(
		FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"ssao", D3D11_PIXEL_SHADER))
		);
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
	GBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

	for (auto i = 0u; i != 2; ++i) {
		if (mSRVs[i])
			mSRVs[i]->Release();
		if (mMRTs[i])
			mMRTs[i]->Release();

		ID3D11Texture2D* mTex;
		device->CreateTexture2D(&GBufferDesc, nullptr, &mTex);
		device->CreateShaderResourceView(mTex, nullptr, mSRVs+i);
		device->CreateRenderTargetView(mTex, nullptr, mMRTs + i);

		mTex->Release();
	}


	D3D11_TEXTURE2D_DESC SSAOTexDesc;
	SSAOTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

	ID3D11Texture2D* mTex = nullptr;
	device->CreateTexture2D(&SSAOTexDesc, nullptr, &mTex);
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
	leo::win::ReleaseCOM(mIAVS);
}

void DeferredResources::SetFrustum(const CameraFrustum& frustum) noexcept {
}

void DeferredResources::OMSet() noexcept {
}
void DeferredResources::IASet() noexcept {
}

void DeferredResources::ComputerSSAO() noexcept {
}
