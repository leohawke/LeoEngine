#include "DeferredRender.hpp"
#include <platform.h>
#include <Core\COM.hpp>
#include "d3dx11.hpp"
using namespace leo;

//TODO :Support MSAA

class DeferredRender::DeferredResImpl {
public:
	/*
	RT0:R8G8B8A8_UNORM<normal,specmono>{
				normal(12bit float2->8bit float3)<>R8G8B8,
				specpow(Material)<>A8
				}
	RT1:R8G8B8A8 <diffuse,pad>{
				tex.Sample()<>R18G8B8,
				specmono(dot(specular,float3(0.2126f,0.7152f,0.0722f)))<>A8
				}
	*/
	win::unique_com<ID3D11RenderTargetView> mGBuffRTVs[2];
	//R32_FLOAT,linear depth
	win::unique_com<ID3D11RenderTargetView> mDepthRTV = nullptr;
	//R8G8B8A8<,light RT,diffuse,specPow
	win::unique_com<ID3D11RenderTargetView> mLightRTV = nullptr;
	//RT0
	win::unique_com<ID3D11ShaderResourceView> mNormalSpecPowSRV = nullptr;
	//RT1
	win::unique_com<ID3D11ShaderResourceView> mDiffuseSpecSRV = nullptr;
	//depth RT
	win::unique_com<ID3D11ShaderResourceView> mDepthSRV = nullptr;
	//light RT
	win::unique_com<ID3D11ShaderResourceView> mLightSRV = nullptr;

	//TODO:格式检查支持,替换格式
	DeferredResImpl(ID3D11Device* device,std::pair<uint16,uint16> size) {

		CreateRes(device, size);
	}

	~DeferredResImpl() = default;
private:
	void CreateRes(ID3D11Device* device, std::pair<uint16, uint16> size) {
		CD3D11_TEXTURE2D_DESC gbuffTexDesc{ DXGI_FORMAT_R8G8B8A8_UNORM,size.first,size.second };
		gbuffTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		//mNormalSpecPowSRV
		{
			auto mGBufferTex = win::make_scope_com<ID3D11Texture2D>();
			device->CreateTexture2D(&gbuffTexDesc, nullptr, &mGBufferTex);

			device->CreateShaderResourceView(mGBufferTex, nullptr, &mNormalSpecPowSRV);
			device->CreateRenderTargetView(mGBufferTex, nullptr, &mGBuffRTVs[0]);
		}
		//mDiffuseSpecSRV
		{
			auto mGBufferTex = win::make_scope_com<ID3D11Texture2D>();
			device->CreateTexture2D(&gbuffTexDesc, nullptr, &mGBufferTex);

			device->CreateShaderResourceView(mGBufferTex, nullptr, &mDiffuseSpecSRV);
			device->CreateRenderTargetView(mGBufferTex, nullptr, &mGBuffRTVs[1]);
		}
		//mDepthSRV
		{
			CD3D11_TEXTURE2D_DESC depthTexDesc{ DXGI_FORMAT_R32_FLOAT,size.first,size.second };

			auto mDepthTex = win::make_scope_com<ID3D11Texture2D>();
			device->CreateTexture2D(&depthTexDesc, nullptr, &mDepthTex);

			device->CreateShaderResourceView(mDepthTex, nullptr, &mDepthSRV);
			device->CreateRenderTargetView(mDepthTex, nullptr, &mDepthRTV);
		}
		//mLightSRV
		{
			CD3D11_TEXTURE2D_DESC lightTexDesc{ DXGI_FORMAT_R8G8B8A8_UNORM,size.first,size.second };

			auto mLightTex = win::make_scope_com<ID3D11Texture2D>();
			device->CreateTexture2D(&lightTexDesc, nullptr, &mLightTex);

			device->CreateShaderResourceView(mLightTex, nullptr, &mLightSRV);
			device->CreateRenderTargetView(mLightTex, nullptr, &mLightRTV);
		}
	}
};

class DeferredRender::DeferredStateImpl {
public:
	DeferredStateImpl(ID3D11Device* device) {
		//light :stencil-ref : 0x10
		//no-light:stencil-ref: 0x01
		CD3D11_DEPTH_STENCIL_DESC gBufferPassDSDesc{D3D11_DEFAULT};
		gBufferPassDSDesc.StencilEnable = true;
		gBufferPassDSDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		device->CreateDepthStencilState(&gBufferPassDSDesc, &mGBufferPassDepthStenciState);

		CD3D11_DEPTH_STENCIL_DESC lightPassDSDesc{ D3D11_DEFAULT };
		lightPassDSDesc.DepthEnable = false;
		lightPassDSDesc.StencilEnable = true;
		lightPassDSDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		lightPassDSDesc.BackFace = lightPassDSDesc.FrontFace;
		device->CreateDepthStencilState(&lightPassDSDesc, &mLightPassDepthStenciState);

		CD3D11_DEPTH_STENCIL_DESC shaderPassDesc{ D3D11_DEFAULT };
		shaderPassDesc.DepthEnable = false;
		shaderPassDesc.StencilEnable = true;
		shaderPassDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		shaderPassDesc.StencilReadMask = 0x11;
		shaderPassDesc.BackFace = shaderPassDesc.FrontFace;
		device->CreateDepthStencilState(&shaderPassDesc, &mShaderPassDepthStenciState);

	}

	win::unique_com<ID3D11DepthStencilState> mGBufferPassDepthStenciState = nullptr;
	win::unique_com<ID3D11DepthStencilState> mLightPassDepthStenciState = nullptr;
	win::unique_com<ID3D11DepthStencilState> mShaderPassDepthStenciState = nullptr;
};

leo::DeferredRender::DeferredRender(ID3D11Device * device, size_type size)
	:pResImpl(std::make_unique<DeferredResImpl>(device,size)),
	pStateImpl(std::make_unique<DeferredStateImpl>(device))
{
}

void leo::DeferredRender::OMSet(ID3D11DeviceContext * context, DepthStencil& depthstencil) noexcept
{
	ID3D11RenderTargetView* mRTVs[] = { pResImpl->mGBuffRTVs[0], pResImpl->mGBuffRTVs[1] };
	context->OMSetRenderTargets(arrlen(pResImpl->mGBuffRTVs), mRTVs, depthstencil);
	context->OMSetDepthStencilState(pStateImpl->mGBufferPassDepthStenciState, 0x10);
}

void leo::DeferredRender::ReSize(ID3D11Device * device, size_type size) noexcept
{
	pResImpl.reset(nullptr);
	pResImpl = std::make_unique<DeferredResImpl>(device, size);
}
