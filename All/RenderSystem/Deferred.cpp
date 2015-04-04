#include "..\DeviceMgr.h"
#include "..\d3dx11.hpp"
#include "Deferred.h"

using namespace leo;

namespace {
	ID3D11ShaderResourceView* mSRVs[2] = { nullptr,nullptr };
	ID3D11RenderTargetView* mMRTs[2] = { nullptr,nullptr };
}

DeferredResources::DeferredResources() noexcept {
	DeviceMgr mgr;
	auto size = mgr.GetClientSize();
	ReSize(size);
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
}

DeferredResources::~DeferredResources() {
	for (auto i = 0u; i != 2; ++i) {
		if (mSRVs[i])
			leo::win::ReleaseCOM(mSRVs[i]);
		if (mMRTs[i])
			leo::win::ReleaseCOM(mMRTs[i]);
	}
}