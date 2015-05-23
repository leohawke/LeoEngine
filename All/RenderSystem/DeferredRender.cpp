#include "DeferredRender.hpp"
#include <platform.h>
#include <Core\COM.hpp>
#include <Core\FileSearch.h>
#include <Core\Camera.hpp>
#include <Core\BilateralFilter.hpp>
#include <Core\EffectQuad.hpp>
#include "d3dx11.hpp"
#include "ShaderMgr.h"
#include "RenderStates.hpp"
#include <leomathutility.hpp>
#include <exception.hpp>

#include <DirectXPackedVector.h>
//TODO :Support MSAA

class LinearizeDepthImpl;

class leo::DeferredRender::DeferredResImpl {
public:
	/*
	RT0:R8G8B8A8_UNORM<normal,specmono>{
				normal(12bit float2->8bit float3)<>R8G8B8,
				specpow(Material)<>A8
				}
	RT1:R8G8B8A8 <diffuse,pad>{
				tex.Sample()<>R8G8B8,
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
	DeferredResImpl(ID3D11Device* device, std::pair<uint16, uint16> size) {

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
			depthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

			auto mDepthTex = win::make_scope_com<ID3D11Texture2D>();
			device->CreateTexture2D(&depthTexDesc, nullptr, &mDepthTex);

			device->CreateShaderResourceView(mDepthTex, nullptr, &mDepthSRV);
			device->CreateRenderTargetView(mDepthTex, nullptr, &mDepthRTV);
		}
		//mLightSRV
		{
			CD3D11_TEXTURE2D_DESC lightTexDesc{ DXGI_FORMAT_R8G8B8A8_UNORM,size.first,size.second };
			lightTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

			auto mLightTex = win::make_scope_com<ID3D11Texture2D>();
			device->CreateTexture2D(&lightTexDesc, nullptr, &mLightTex);

			device->CreateShaderResourceView(mLightTex, nullptr, &mLightSRV);
			device->CreateRenderTargetView(mLightTex, nullptr, &mLightRTV);
		}
	}
};

class leo::DeferredRender::DeferredStateImpl {
public:
	DeferredStateImpl(ID3D11Device* device) {
		//light :stencil-ref : 0x10
		//no-light:stencil-ref: 0x01
		CD3D11_DEPTH_STENCIL_DESC gBufferPassDSDesc{ D3D11_DEFAULT };
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

		CD3D11_BLEND_DESC lightPassBDesc{ D3D11_DEFAULT };
		lightPassBDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		lightPassBDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		device->CreateBlendState(&lightPassBDesc, &mLightPassBlendState);

		ShaderMgr sm;
		mShaderPS = sm.CreatePixelShader(FileSearch::Search(L"ShaderPS.cso"));
	}

	win::unique_com<ID3D11DepthStencilState> mGBufferPassDepthStenciState = nullptr;
	win::unique_com<ID3D11DepthStencilState> mLightPassDepthStenciState = nullptr;
	win::unique_com<ID3D11DepthStencilState> mShaderPassDepthStenciState = nullptr;

	//着色阶段
	ID3D11PixelShader* mShaderPS = nullptr;

	//光照阶段BlendState
	win::unique_com<ID3D11BlendState> mLightPassBlendState = nullptr;
};

class LinearizeDepthImpl : public leo::Singleton<LinearizeDepthImpl, false>
{
public:
	LinearizeDepthImpl(ID3D11Device* device) {
		using	namespace leo;

		CD3D11_BUFFER_DESC cbDesc{ sizeof(leo::float4),D3D11_BIND_CONSTANT_BUFFER };
		device->CreateBuffer(&cbDesc, nullptr, &mPSCB);

		ShaderMgr sm;
		mLinearizeDepthPS= sm.CreatePixelShader(FileSearch::Search(L"LinearizeDepthPS.cso"));

		RenderStates ss;
		mSamPoint = ss.GetSamplerState(L"NearestClamp");
	}
	~LinearizeDepthImpl() {

	}

	void Apply(ID3D11DeviceContext * context, float near_z, float far_z) {
		float Q = far_z / (far_z - near_z);
		float Mul = near_z*Q;

		leo::float2 MulQ{ Mul,Q };

		context->UpdateSubresource(mPSCB, 0, nullptr, &MulQ, 0, 0);

		context->PSSetShader(mLinearizeDepthPS, nullptr, 0);
		context->PSSetConstantBuffers(0, 1, &mPSCB);
		context->PSSetSamplers(0, 1, &mSamPoint);
	}

	ID3D11PixelShader* mLinearizeDepthPS = nullptr;
	ID3D11SamplerState* mSamPoint = nullptr;

	leo::win::unique_com<ID3D11Buffer> mPSCB = nullptr;

	static LinearizeDepthImpl& GetInstance(ID3D11Device* device = nullptr) {
		static LinearizeDepthImpl mInstance{ device };
		return mInstance;
	}
};

leo::DeferredRender::DeferredRender(ID3D11Device * device, size_type size)
	:pResImpl(std::make_unique<DeferredResImpl>(device, size)),
	pStateImpl(std::make_unique<DeferredStateImpl>(device))
{
	LinearizeDepthImpl::GetInstance(device);
}

leo::DeferredRender::~DeferredRender() {
	LinearizeDepthImpl::GetInstance().~LinearizeDepthImpl();
}

void leo::DeferredRender::OMSet(ID3D11DeviceContext * context, DepthStencil& depthstencil) noexcept
{
	ID3D11RenderTargetView* mRTVs[] = { pResImpl->mGBuffRTVs[0], pResImpl->mGBuffRTVs[1] };
	context->OMSetRenderTargets(arrlen(pResImpl->mGBuffRTVs), mRTVs, depthstencil);
	context->OMSetDepthStencilState(pStateImpl->mGBufferPassDepthStenciState, 0x10);

	const static float rgba[] = { 0.f,0.f,0.f,0.f };

	context->ClearRenderTargetView(pResImpl->mGBuffRTVs[0], rgba);
	context->ClearRenderTargetView(pResImpl->mGBuffRTVs[1], rgba);
	context->ClearDepthStencilView(depthstencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

}

void leo::DeferredRender::UnBind(ID3D11DeviceContext* context, DepthStencil& depthstencil) noexcept {
	ID3D11RenderTargetView* mRTVs[] = { nullptr,nullptr };
	context->OMSetRenderTargets(arrlen(mRTVs), mRTVs, depthstencil);

}


void leo::DeferredRender::ReSize(ID3D11Device * device, size_type size) noexcept
{
	pResImpl.reset(nullptr);
	pResImpl = std::make_unique<DeferredResImpl>(device, size);
}

void leo::DeferredRender::LinearizeDepth(ID3D11DeviceContext * context, DepthStencil& depthstencil, float near_z, float far_z) noexcept
{
	auto & effectQuad = leo::EffectQuad::GetInstance();

	effectQuad.Apply(context);
	LinearizeDepthImpl::GetInstance().Apply(context, near_z, far_z);

	context->OMSetRenderTargets(1, &pResImpl->mDepthRTV, nullptr);
	auto srv = depthstencil.GetDepthSRV();
	context->PSSetShaderResources(0, 1, &srv);

	const static float rgba[] = {near_z,1.f,1.f,1.f};

	context->ClearRenderTargetView(pResImpl->mDepthRTV,rgba );

	effectQuad.Draw(context);
	srv = nullptr;
	context->PSSetShaderResources(0, 1, &srv);
}

void leo::DeferredRender::ShadingPass(ID3D11DeviceContext * context, ID3D11RenderTargetView * finally_rtv) noexcept
{

	auto & effectQuad = leo::EffectQuad::GetInstance();


	static const float rgba[4] = { 0.0f, 0.25f, 0.25f, 0.8f };
	context->OMSetRenderTargets(1, &finally_rtv, nullptr);
	context->ClearRenderTargetView(finally_rtv, rgba);

	effectQuad.Apply(context);
	context->PSSetShader(pStateImpl->mShaderPS, nullptr, 0);

	ID3D11ShaderResourceView* srvs[] = {
		pResImpl->mNormalSpecPowSRV,
		pResImpl->mLightSRV ,
		pResImpl->mDiffuseSpecSRV
	};

	context->PSSetShaderResources(0,arrlen(srvs),srvs);
	context->PSSetSamplers(0, 1, &(LinearizeDepthImpl::GetInstance().mSamPoint));

	effectQuad.Draw(context);

	for (auto & s : srvs)
		s = nullptr;
	context->PSSetShaderResources(0, arrlen(srvs), srvs);
}

void leo::DeferredRender::SetSSAOParams(bool enable, uint8 level) noexcept
{
}

void leo::DeferredRender::ApplyLightPass(ID3D11DeviceContext * context) noexcept
{
	const static float factor[] = { 0.f,0.f,0.f,0.f };
	context->OMSetBlendState(pStateImpl->mLightPassBlendState,factor, 0xffffffff);

	context->OMSetRenderTargets(1, &pResImpl->mLightRTV, nullptr);
	//忽略模板测试,忽略模板
	ID3D11ShaderResourceView* srvs[] = { GetLinearDepthSRV(),GetNormalAlphaSRV() };
	context->PSSetSamplers(0, 1, &LinearizeDepthImpl::GetInstance().mSamPoint);

	context->PSSetShaderResources(0, 2, srvs);
}

ID3D11ShaderResourceView * leo::DeferredRender::GetLinearDepthSRV() const noexcept
{
	return pResImpl->mDepthSRV;
}

ID3D11RenderTargetView * leo::DeferredRender::GetLightRTV() const noexcept
{
	return pResImpl->mLightRTV;
}

ID3D11ShaderResourceView * leo::DeferredRender::GetNormalAlphaSRV() const noexcept
{
	return pResImpl->mNormalSpecPowSRV;
}

leo::DeferredRender::SSAO::~SSAO()
{
}

class leo::DeferredRender::SSAO::DeferredSSAOImpl {
	ID3D11PixelShader* mSSAOPS = nullptr;

	win::unique_com<ID3D11Buffer> mSSAOPSCB = nullptr;
	win::unique_com< ID3D11ShaderResourceView> mSSAORandomVec = nullptr;

	win::unique_com<ID3D11ShaderResourceView> mBlurSSAOSRV = nullptr;
	win::unique_com<ID3D11UnorderedAccessView> mBlurSSAOUAV = nullptr;

	win::unique_com<ID3D11ShaderResourceView> mBlurSwapSSAOSRV = nullptr;
	win::unique_com<ID3D11UnorderedAccessView> mBlurSwapSSAOUAV = nullptr;

	ID3D11ComputeShader* mBlurSSAOCS = nullptr;

	ID3D11ComputeShader* mBlurVerSSAOCS = nullptr;
	ID3D11ComputeShader* mBlurHorSSAOCS = nullptr;

	struct SSAO {
		leo::float4x4 gProj;
		leo::float4 gOffsetVectors[14];

		float    gOcclusionRadius = 5.5f;
		float    gOcclusionFadeStart = 2.0f;
		float    gOcclusionFadeEnd = 20.0f;
		float    gSurfaceEpsilon = 0.55f;
	};
	SSAO ssao;

public:
	~DeferredSSAOImpl() {

	}
	//SSAO Dependent
	DeferredSSAOImpl(ID3D11Device* device, size_type size, const Camera& camera) {
		//SSAO ,GPU资源
		leo::ShaderMgr sm;
		auto  mPSBlob = sm.CreateBlob(leo::FileSearch::Search(L"SSAOPS.cso"));
		mSSAOPS = sm.CreatePixelShader(mPSBlob);



		D3D11_BUFFER_DESC Desc;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = 0;
		Desc.MiscFlags = 0;
		Desc.StructureByteStride = 0;
		Desc.ByteWidth = sizeof(SSAO);

		D3D11_SUBRESOURCE_DATA subData;
		subData.pSysMem = &ssao;
		subData.SysMemPitch = 0;
		subData.SysMemSlicePitch = 0;

		// 8 cube corners
		ssao.gOffsetVectors[0] = leo::float4(+1.0f, +1.0f, +1.0f, 0.0f);
		ssao.gOffsetVectors[1] = leo::float4(-1.0f, -1.0f, -1.0f, 0.0f);

		ssao.gOffsetVectors[2] = leo::float4(-1.0f, +1.0f, +1.0f, 0.0f);
		ssao.gOffsetVectors[3] = leo::float4(+1.0f, -1.0f, -1.0f, 0.0f);

		ssao.gOffsetVectors[4] = leo::float4(+1.0f, +1.0f, -1.0f, 0.0f);
		ssao.gOffsetVectors[5] = leo::float4(-1.0f, -1.0f, +1.0f, 0.0f);

		ssao.gOffsetVectors[6] = leo::float4(-1.0f, +1.0f, -1.0f, 0.0f);
		ssao.gOffsetVectors[7] = leo::float4(+1.0f, -1.0f, +1.0f, 0.0f);

		// 6 centers of cube faces
		ssao.gOffsetVectors[8] = leo::float4(-1.0f, 0.0f, 0.0f, 0.0f);
		ssao.gOffsetVectors[9] = leo::float4(+1.0f, 0.0f, 0.0f, 0.0f);

		ssao.gOffsetVectors[10] = leo::float4(0.0f, -1.0f, 0.0f, 0.0f);
		ssao.gOffsetVectors[11] = leo::float4(0.0f, +1.0f, 0.0f, 0.0f);

		ssao.gOffsetVectors[12] = leo::float4(0.0f, 0.0f, -1.0f, 0.0f);
		ssao.gOffsetVectors[13] = leo::float4(0.0f, 0.0f, +1.0f, 0.0f);

		for (auto & v : ssao.gOffsetVectors) {
			float s = (rand()*1.f / RAND_MAX)*0.75f + 0.25f;
			leo::save(v, leo::Multiply(load(v), s));
		}

		float data[] = { 0.5f,0.f,0.f,0.f,
			0.f,-0.5f,0.f,0.f,
			0.f,0.f,1.f,0.f,
			0.5f,0.5f,0.f,1.f };
		leo::float4x4 toTex{ data };

		leo::save(ssao.gProj,
			leo::Transpose(
				leo::Multiply(
					leo::load(camera.Proj()),
					load(toTex))));


		leo::dxcall(device->CreateBuffer(&Desc, &subData, &mSSAOPSCB));

		//mSSAORandomVec
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = 256;
		texDesc.Height = 256;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_IMMUTABLE;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData = { 0 };
		initData.SysMemPitch = 256 * sizeof(DirectX::PackedVector::XMCOLOR);

		DirectX::PackedVector::XMCOLOR color[256 * 256];
		for (int i = 0; i < 256; ++i)
		{
			for (int j = 0; j < 256; ++j)
			{
				leo::float3 v(rand()*1.f / RAND_MAX, rand()*1.f / RAND_MAX, rand()*1.f / RAND_MAX);

				color[i * 256 + j] = DirectX::PackedVector::XMCOLOR(v.x, v.y, v.z, 0.0f);
			}
		}

		initData.pSysMem = color;

		ID3D11Texture2D* tex = 0;
		leo::dxcall(device->CreateTexture2D(&texDesc, &initData, &tex));

		leo::dxcall(device->CreateShaderResourceView(tex, 0, &mSSAORandomVec));

		// view saves a reference.
		leo::win::ReleaseCOM(tex);

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

		SSAOTexDesc.Width = size.first / 2;
		SSAOTexDesc.Height = size.second / 2;

		SSAOTexDesc.Usage = D3D11_USAGE_DEFAULT;
		SSAOTexDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		SSAOTexDesc.CPUAccessFlags = 0;
		SSAOTexDesc.MiscFlags = 0;

		using leo::win::make_scope_com;
		{
			auto mTex = make_scope_com<ID3D11Texture2D>();
			leo::dxcall(device->CreateTexture2D(&SSAOTexDesc, nullptr, &mTex));
			leo::dxcall(device->CreateShaderResourceView(mTex, nullptr, &mBlurSSAOSRV));
			leo::dxcall(device->CreateUnorderedAccessView(mTex, nullptr, &mBlurSSAOUAV));
		}
		{
			auto mTex = make_scope_com<ID3D11Texture2D>();
			leo::dxcall(device->CreateTexture2D(&SSAOTexDesc, nullptr, &mTex));
			leo::dxcall(device->CreateShaderResourceView(mTex, nullptr, &mBlurSwapSSAOSRV));
			leo::dxcall(device->CreateUnorderedAccessView(mTex, nullptr, &mBlurSwapSSAOUAV));
		}

		CompilerBilaterCS(7, L"BilateralFilterCS.cso");
		CompilerBilaterCS(7, size, L"BilateralFilterVerCS.cso", L"BilateralFilterHorCS.cso");
		auto mBlurCSBlob = sm.CreateBlob(leo::FileSearch::Search(L"BilateralFilterCS.cso"));

		mBlurSSAOCS = sm.CreateComputeShader(mBlurCSBlob);

		mBlurHorSSAOCS = sm.CreateComputeShader(leo::FileSearch::Search(L"BilateralFilterHorCS.cso"));
		mBlurVerSSAOCS = sm.CreateComputeShader(leo::FileSearch::Search(L"BilateralFilterVerCS.cso"));


	}

	void ReSize(ID3D11Device* device,size_type size) {
		//改变AO资源的大小
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

		SSAOTexDesc.Width = size.first / 2;
		SSAOTexDesc.Height = size.second / 2;

		SSAOTexDesc.Usage = D3D11_USAGE_DEFAULT;
		SSAOTexDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		SSAOTexDesc.CPUAccessFlags = 0;
		SSAOTexDesc.MiscFlags = 0;

		using leo::win::make_scope_com;
		{
			auto mTex = make_scope_com<ID3D11Texture2D>();
			leo::dxcall(device->CreateTexture2D(&SSAOTexDesc, nullptr, &mTex));
			mBlurSSAOSRV->Release();
			mBlurSSAOUAV->Release();
			leo::dxcall(device->CreateShaderResourceView(mTex, nullptr, &mBlurSSAOSRV));
			leo::dxcall(device->CreateUnorderedAccessView(mTex, nullptr, &mBlurSSAOUAV));
		}
		{
			auto mTex = make_scope_com<ID3D11Texture2D>();
			leo::dxcall(device->CreateTexture2D(&SSAOTexDesc, nullptr, &mTex));
			mBlurSwapSSAOSRV->Release();
			mBlurSwapSSAOUAV->Release();
			leo::dxcall(device->CreateShaderResourceView(mTex, nullptr, &mBlurSwapSSAOSRV));
			leo::dxcall(device->CreateUnorderedAccessView(mTex, nullptr, &mBlurSwapSSAOUAV));
		}
	}

	void Compute(ID3D11DeviceContext* context) {
		D3D11_VIEWPORT prevVP;
		UINT num = 1;
		context->RSGetViewports(&num, &prevVP);
		D3D11_VIEWPORT currvp = prevVP;
		currvp.Height = prevVP.Height / 2;
		currvp.Width = prevVP.Width / 2;
		context->RSSetViewports(1, &currvp);

		ID3D11RenderTargetView* mMRTs[] = { nullptr,nullptr };
		context->OMSetRenderTargets(2, mMRTs, nullptr);
		float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.8f };
		context->ClearRenderTargetView(mMRTs[0], ClearColor);

		context->PSSetShader(mSSAOPS, nullptr, 0);

		context->PSSetConstantBuffers(0, 1, &mSSAOPSCB);
		context->PSSetShaderResources(2, 1, &mSSAORandomVec);

		context->Draw(4, 0);
	}

	void Blur(ID3D11DeviceContext* context, size_type size) {
		ID3D11ShaderResourceView* srv = nullptr;

		context->CSSetShader(mBlurHorSSAOCS, nullptr, 0);
		context->CSSetShaderResources(0, 1, &srv);
		context->CSSetUnorderedAccessViews(0, 1, &mBlurSwapSSAOUAV, nullptr);//swapUAV

		context->Dispatch(size.first, 1, 1);

		ID3D11UnorderedAccessView* mUAV = nullptr;
		ID3D11ShaderResourceView* mSRV = nullptr;
		context->CSSetUnorderedAccessViews(0, 1, &mUAV, nullptr);
		context->CSSetShaderResources(0, 1, &mSRV);

		context->CSSetShader(mBlurVerSSAOCS, nullptr, 0);
		context->CSSetShaderResources(0, 1, &mBlurSwapSSAOSRV);//swapSRV
		context->CSSetUnorderedAccessViews(0, 1, &mBlurSSAOUAV, nullptr);
		context->Dispatch(1, size.second, 1);
		context->CSSetUnorderedAccessViews(0, 1, &mUAV, nullptr);
		context->CSSetShaderResources(0, 1, &mSRV);



		using leo::win::make_scope_com;

		auto mSSAORes = make_scope_com<ID3D11Resource>();
		auto mBlurSSAORes = make_scope_com<ID3D11Resource>();
		srv->GetResource(&mSSAORes);
		mBlurSSAOSRV->GetResource(&mBlurSSAORes);
		context->CopyResource(mSSAORes, mBlurSSAORes);
	}
};
