#include "d3dx11.hpp"
#include "RenderStates.hpp"

#include "DepthStencil.hpp"

using namespace leo;

DepthStencil::DepthStencil(std::pair<uint16, uint16> size,ID3D11Device * device)
	:DepthStencil(size,device,{1,0})
{
}

DepthStencil::DepthStencil(std::pair<uint16, uint16> size,ID3D11Device * device, DXGI_SAMPLE_DESC sampleDesc)
{
	CD3D11_TEXTURE2D_DESC depthTexDesc{ DXGI_FORMAT_R24G8_TYPELESS,size.first,size.second };

	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthTexDesc.SampleDesc = sampleDesc;

	auto mTex = win::make_scope_com<ID3D11Texture2D>();

	device->CreateTexture2D(&depthTexDesc, nullptr, &mTex);
	dx::DebugCOM(mTex, "DepthStencil::temp Tex");

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthDsvDesc{ D3D11_DSV_DIMENSION_TEXTURE2D,DXGI_FORMAT_D24_UNORM_S8_UINT };
	//depthDsvDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
	device->CreateDepthStencilView(mTex, &depthDsvDesc, &mDepthStencilView);
	dx::DebugCOM(mDepthStencilView, "DepthStencil::mDepthStencilView");

	CD3D11_SHADER_RESOURCE_VIEW_DESC depthSrvDesc{ D3D11_SRV_DIMENSION_TEXTURE2D,DXGI_FORMAT_R24_UNORM_X8_TYPELESS,0,1 };
	device->CreateShaderResourceView(mTex, &depthSrvDesc, &mDepthSRV);
	dx::DebugCOM(mDepthSRV, "DepthStencil::mDepthSRV");

}

DepthStencil::~DepthStencil()
{
}

DepthStencil::operator ID3D11DepthStencilView*() const noexcept
{
	return mDepthStencilView;
}

ID3D11ShaderResourceView * DepthStencil::GetDepthSRV() const noexcept
{
	return mDepthSRV;
}

void DepthStencil::ReSize(std::pair<uint16, uint16> size, ID3D11Device* device)
{
	CD3D11_TEXTURE2D_DESC depthTexDesc;
	CD3D11_DEPTH_STENCIL_VIEW_DESC depthDsvDesc;
	CD3D11_SHADER_RESOURCE_VIEW_DESC depthSrvDesc;
	{
		auto mRes = win::make_scope_com<ID3D11Resource>();
		mDepthSRV->GetResource(&mRes);

		auto mTex = win::make_scope_com<ID3D11Texture2D>();
		mRes->QueryInterface(&mTex);

		mTex->GetDesc(&depthTexDesc);

		if (depthTexDesc.Width == size.first && depthTexDesc.Height == size.second)
			return;

		mDepthSRV->GetDesc(&depthSrvDesc);
		mDepthStencilView->GetDesc(&depthDsvDesc);

		mDepthSRV->Release();
		mDepthStencilView->Release();
	}

	depthTexDesc.Width = size.first;
	depthTexDesc.Height = size.second;

	auto mTex = win::make_scope_com<ID3D11Texture2D>();
	device->CreateTexture2D(&depthTexDesc, nullptr, &mTex);
	device->CreateDepthStencilView(mTex, &depthDsvDesc, &mDepthStencilView);
	device->CreateShaderResourceView(mTex, &depthSrvDesc, &mDepthSRV);
	dx::DebugCOM(mTex, "DepthStencil::temp Tex");
	dx::DebugCOM(mDepthStencilView, "DepthStencil::mDepthStencilView");
	dx::DebugCOM(mDepthSRV, "DepthStencil::mDepthSRV");


}
