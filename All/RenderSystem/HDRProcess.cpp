#include <Core\EngineConfig.h>
#include <Core\FileSearch.h>

#include "HDRProcess.h"
#include "ShaderMgr.h"
#include "CopyProcess.hpp"

ID3D11Buffer* leo::HDRProcess::HDRCommon::mGpuParams = nullptr;//sizeof = float4[2]
ID3D11VertexShader* leo::HDRProcess::HDRCommon::mLumVS = nullptr;

ID3D11SamplerState* leo::HDRProcess::HDRCommon::src_sampler = nullptr;
ID3D11SamplerState* leo::HDRProcess::HDRCommon::last_lum_sampler = nullptr;
std::size_t leo::HDRProcess::HDRCommon::mRefCount = 0;

leo::HDRProcess::HDRCommon::HDRCommon(ID3D11Device * create)
	:PostProcess(create), mTexDesc(DXGI_FORMAT_R32_FLOAT,//FORMAT
		1, 1,//SIZE
		1, 1,//ARRAT & MIP
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE  //BindFlag)
		)
{
	++mRefCount;

#ifdef NO_SINGLE_CHANNEL_FLOAT
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
#endif

	if (!mGpuParams) {
		leo::dx::CreateGPUCBuffer<decltype(mCpuParams)>(create, mGpuParams, "tex_coord_offset");
	}

	if (!mLumVS) {
		mLumVS = ShaderMgr().CreateVertexShader(
			FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"tonemapping", D3D11_VERTEX_SHADER))
			);
	}

	if (!src_sampler) {
		CD3D11_SAMPLER_DESC mSampleDesc{ D3D11_DEFAULT };
		mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		src_sampler = leo::RenderStates().CreateSamplerState(L"src_sampler", mSampleDesc);
	}

	if (!last_lum_sampler) {
		CD3D11_SAMPLER_DESC mSampleDesc{ D3D11_DEFAULT };
		mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		last_lum_sampler = leo::RenderStates().CreateSamplerState(L"last_lum_sampler", mSampleDesc);
	}

	mViewPort.TopLeftX = 0;
	mViewPort.TopLeftY = 0;
	mViewPort.MaxDepth = 1.0f;
	mViewPort.MinDepth = 0.0f;
}

leo::HDRProcess::HDRCommon::~HDRCommon()
{
	--mRefCount;
	if (!mRefCount)
		win::ReleaseCOM(mGpuParams);
}

void leo::HDRProcess::HDRCommon::Input(UINT width, UINT height)
{
	mWidth = width;
	mHeight = height;
}

void leo::HDRProcess::HDRCommon::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
	context->VSSetShader(mLumVS, nullptr, 0);
	context->VSSetConstantBuffers(0, 1, &mGpuParams);
	ID3D11SamplerState* sss[] = { src_sampler,last_lum_sampler };
	context->PSSetSamplers(0, 2, sss);
}

void leo::HDRProcess::HDRCommon::GetSampleOffset(UINT width, UINT height)
{
	float const tu = 1.0f / width;
	float const tv = 1.0f / height;

	// Sample from the 16 surrounding points.
	int index = 0;
	for (int y = -1; y <= 2; y += 2)
	{
		for (int x = -1; x <= 2; x += 4)
		{
			mCpuParams[index].x = (x + 0) * tu;
			mCpuParams[index].y = y * tv;
			mCpuParams[index].z = (x + 2) * tu;
			mCpuParams[index].w = y * tv;

			++index;
		}
	}
}

leo::HDRProcess::LumLogProcess::LumLogProcess(ID3D11Device * create, unsigned level)
	:HDRCommon(create)
{
#ifdef NO_SINGLE_CHANNEL_FLOAT
	BindProcess(create, "Shader/LumLogInitialPS_multiple.cso");
#else
	BindProcess(create, L"Shader/LumLogInitialPS_single.cso");
#endif

	mTexDesc.Width = mTexDesc.Height = 1 << (2 * level);
	mViewPort.Width = mViewPort.Height = static_cast<float>(mTexDesc.Width);

	auto tempTex = leo::win::make_scope_com<ID3D11Texture2D>();
	leo::dxcall(create->CreateTexture2D(&mTexDesc, nullptr, &tempTex));
	leo::dxcall(create->CreateRenderTargetView(tempTex, nullptr, &mLumLogRTV));
	leo::dxcall(create->CreateShaderResourceView(tempTex, nullptr, &mLumLogOutput));
}

ID3D11ShaderResourceView * leo::HDRProcess::LumLogProcess::Output(UINT& width, UINT& height) const
{
	width = mTexDesc.Width;
	height = mTexDesc.Height;
	return mLumLogOutput;
}

void leo::HDRProcess::LumLogProcess::Draw(ID3D11DeviceContext * context, ID3D11ShaderResourceView * src, ID3D11RenderTargetView *)
{
	context->RSSetViewports(1, &mViewPort);
	GetSampleOffset(mWidth, mHeight);
	context->UpdateSubresource(mGpuParams, 0, nullptr, &mCpuParams, 0, 0);
	PostProcess::Draw(context, src, mLumLogRTV);
}

leo::HDRProcess::LumIterativeProcess::LumIterativeProcess(ID3D11Device * create, unsigned level)
	:HDRCommon(create)
{
#ifdef NO_SINGLE_CHANNEL_FLOAT
	BindProcess(create, "Shader/LumIterativePS_multiple.cso");
#else
	BindProcess(create, L"Shader/LumIterativePS_single.cso");
#endif

	mTexDesc.Width = mTexDesc.Height = 1 << (2 * level);
	mViewPort.Width = mViewPort.Height = static_cast<float>(mTexDesc.Width);

	auto tempTex = leo::win::make_scope_com<ID3D11Texture2D>();
	leo::dxcall(create->CreateTexture2D(&mTexDesc, nullptr, &tempTex));
	leo::dxcall(create->CreateRenderTargetView(tempTex, nullptr, &mLumIterRTV));
	leo::dxcall(create->CreateShaderResourceView(tempTex, nullptr, &mLumIterOutput));
}

leo::HDRProcess::LumIterativeProcess::~LumIterativeProcess()
{
}

ID3D11ShaderResourceView * leo::HDRProcess::LumIterativeProcess::Output(UINT& width, UINT& height) const
{
	width = mTexDesc.Width;
	height = mTexDesc.Height;
	return mLumIterOutput;
}

void leo::HDRProcess::LumIterativeProcess::Draw(ID3D11DeviceContext * context, ID3D11ShaderResourceView * src, ID3D11RenderTargetView *)
{
	context->RSSetViewports(1, &mViewPort);
	GetSampleOffset(mWidth, mHeight);
	context->UpdateSubresource(mGpuParams, 0, nullptr, &mCpuParams, 0, 0);
	PostProcess::Draw(context, src, mLumIterRTV);
}

leo::HDRProcess::LumAdaptedProcess::LumAdaptedProcess(ID3D11Device * create)
	:HDRCommon(create)
{
	leo::dx::CreateGPUCBuffer<leo::float4>(create, mDeltaGpuParams, "frame_delta");


#ifdef NO_SINGLE_CHANNEL_FLOAT
	BindProcess(create, "Shader/LumAdaptedPS_multiple.cso");
#else
	BindProcess(create, L"Shader/LumAdaptedPS_single.cso");
#endif

	mTexDesc.Width = mTexDesc.Height = 1;
	mViewPort.Width = mViewPort.Height = static_cast<float>(mTexDesc.Width);


	auto tempTex = leo::win::make_scope_com<ID3D11Texture2D>();
	leo::dxcall(create->CreateTexture2D(&mTexDesc, nullptr, &tempTex));
	leo::dxcall(create->CreateRenderTargetView(tempTex, nullptr, &mLumAdaptedSwapRTV[0]));
	leo::dxcall(create->CreateShaderResourceView(tempTex, nullptr, &mLumAdaptedSwapOutput[0]));

	auto tempTex_1 = leo::win::make_scope_com<ID3D11Texture2D>();
	leo::dxcall(create->CreateTexture2D(&mTexDesc, nullptr, &tempTex_1));
	leo::dxcall(create->CreateRenderTargetView(tempTex_1, nullptr, &mLumAdaptedSwapRTV[1]));
	leo::dxcall(create->CreateShaderResourceView(tempTex_1, nullptr, &mLumAdaptedSwapOutput[1]));
}

ID3D11ShaderResourceView * leo::HDRProcess::LumAdaptedProcess::Output() const
{
	return mLumAdaptedSwapOutput[mIndex];
}

void leo::HDRProcess::LumAdaptedProcess::SetFrameDelta(float dt)
{
	mDeltaCpuParams.x = dt;
}

void leo::HDRProcess::LumAdaptedProcess::Draw(ID3D11DeviceContext * context, ID3D11ShaderResourceView * src, ID3D11RenderTargetView * dst)
{
	context->RSSetViewports(1, &mViewPort);
	context->UpdateSubresource(mDeltaGpuParams, 0, nullptr, &mDeltaCpuParams, 0, 0);
	context->PSSetConstantBuffers(0, 1, &mDeltaGpuParams);
	context->PSSetShaderResources(1, 1, &mLumAdaptedSwapOutput[mIndex]);
	PostProcess::Draw(context, src, mLumAdaptedSwapRTV[!mIndex]);
	mIndex = !mIndex;

	context->OMSetRenderTargets(1, &dst, nullptr);
}

leo::HDRProcess::HDRStatProcess::HDRStatProcess(ID3D11Device * create, ID3D11Texture2D* tex)
	:mCopyProcess(leo::Make_CopyProcess(create, leo::bilinear_process)), mMaxLevel(4)
{


	mViewPort.Height = mHeight*1.f;
	mViewPort.Width = mWidth*1.f;
	mViewPort.TopLeftX = 0;
	mViewPort.TopLeftY = 0;
	mViewPort.MaxDepth = 1.0f;
	mViewPort.MinDepth = 0.0f;

	pLumFirst = std::make_unique<LumLogProcess>(create, mMaxLevel);

	for (int i = mMaxLevel - 1; i != -1; --i)
		pLumIterVec.emplace_back(std::make_unique<LumIterativeProcess>(create, i));

	pLumFinal = std::make_unique<LumAdaptedProcess>(create);
}

void leo::HDRProcess::HDRStatProcess::Apply(ID3D11DeviceContext * context, float dt)
{
	mCopyProcess->Apply(context);
	context->RSSetViewports(1, &mViewPort);
	mCopyProcess->Draw(context,nullptr, mScaleRT);

	pLumFirst->Apply(context);
	auto width = mWidth;
	auto height = mHeight;

	//Note,this code can do by a for in all prt containter
	//Because this class's base is HDRCommon
	pLumFirst->Input(width, height);
	pLumFirst->Draw(context, mScale, nullptr);
	auto src = pLumFirst->Output(width, height);

	auto i = 0;
	while (i != mMaxLevel) {
		pLumIterVec[i]->Input(width, height);
		pLumIterVec[i]->Draw(context, src, nullptr);
		src = pLumIterVec[i]->Output(width, height);
		++i;
	}
	pLumFinal->SetFrameDelta(dt);
	pLumFinal->Input(width, height);
	pLumFinal->Draw(context, src, nullptr);
}

ID3D11ShaderResourceView * leo::HDRProcess::HDRStatProcess::Output()
{
	return pLumFinal->Output();
}

void leo::HDRProcess::HDRStatProcess::Input(ID3D11Device * create, ID3D11Texture2D * tex)
{

	//first create scale tex res
	D3D11_TEXTURE2D_DESC texDesc;
	tex->GetDesc(&texDesc);

	texDesc.Width /= 4;
	texDesc.Height /= 4;

	texDesc.Width -= (texDesc.Width % 8);
	texDesc.Height -= (texDesc.Height % 8);

	mWidth = texDesc.Width;
	mHeight = texDesc.Height;

	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	auto mScaleTex = leo::win::make_scope_com<ID3D11Texture2D>(nullptr);
	leo::dxcall(create->CreateTexture2D(&texDesc, nullptr, &mScaleTex));
	leo::dxcall(create->CreateShaderResourceView(mScaleTex, nullptr, &mScale));
	leo::dxcall(create->CreateRenderTargetView(mScaleTex, nullptr, &mScaleRT));

}

leo::HDRProcess::HDRProcess(ID3D11Device * create, ID3D11Texture2D* src)
	:PostProcess(create),
	mStatProcess(std::make_unique<HDRStatProcess>(create, src)),
	mToneCpuParams(1.f, 0.25f)
{
#ifdef NO_SINGLE_CHANNEL_FLOAT
	BindProcess(create, "Shader/HDRFinalPS_multiple.cso");
#else
	BindProcess(create, L"Shader/HDRFinalPS_single.cso");
#endif

	leo::dx::CreateGPUCBuffer(create, mToneCpuParams, mToneGpuParams, "tone_params");
}

void leo::HDRProcess::SetFrameDelta(float dt)
{
	mDt = dt;
}

void leo::HDRProcess::Apply(ID3D11DeviceContext * context)
{
	mStatProcess->Apply(context, mDt);
	auto src = mStatProcess->Output();
	context->PSSetShaderResources(2, 1, &src);
	context->PSSetConstantBuffers(0, 1, &mToneGpuParams);
	PostProcess::Apply(context);
}

void leo::HDRProcess::ReSize(ID3D11Device * create, ID3D11Texture2D * src)
{
}
