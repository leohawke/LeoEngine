#include <atomic>
#include <thread>
#include <mutex>
#include <array>
#include <platform.h>
#include <Core\COM.hpp>
#include <Core\FileSearch.h>
#include <Core\Camera.hpp>
#include <Core\BilateralFilter.hpp>
#include <Core\EffectQuad.hpp>
#include <exception.hpp>
#include "d3dx11.hpp"
#include "PostProcess.hpp"
#include "RenderStates.hpp"

//NOTE: ifndef NO_GENMIP,the GPU must support SINGLE_CHANNEL_FLOAT,that's mean 
//ifdef NO_SINGLE_CHANNEL_FLOAT ,then NO_GENMIP will be defined
//but not mean ifdef NO_GENMIP,the NO_SINGLE_CHANNEL_FLOAT will be defined
#ifdef NO_SINGLE_CHANNEL_FLOAT
#ifndef NO_GENMIP
#define NO_GENMIP
#endif
#endif
class HDRLuminanceImpl :public leo::PostProcess
{
public:
	HDRLuminanceImpl(ID3D11Device* device)
		:PostProcess(device)
	{
		D3D11_BUFFER_DESC Desc;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = 0;
		Desc.MiscFlags = 0;
		Desc.StructureByteStride = 0;
		Desc.ByteWidth = sizeof(mOffsets);

		leo::dxcall(device->CreateBuffer(&Desc, nullptr, &mOffsetsBuffer));

		CD3D11_TEXTURE2D_DESC texDesc{ 
			DXGI_FORMAT_R32_FLOAT,//FORMAT
			1,1,//SIZE
			1,1,//ARRAT & MIP
			D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE  //BindFlag
		};
		
		#ifdef NO_GENMIP
		//tone_level_count res
		#ifdef NO_SINGLE_CHANNEL_FLOAT
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		BindProcess(device, "Shader/LumIterativePS_multiple.cso");
		mLumIterativePS = mPixelShader;
		BindProcess(device, "Shader/LumLogInitialPS_multiple.cso");
		#else
		BindProcess(device, L"Shader/LumIterativePS_single.cso");
		mLumIterativePS = mPixelShader;
		BindProcess(device, L"Shader/LumLogInitialPS_single.cso");
		#endif

		mToneViewPort[0].TopLeftX = 0;
		mToneViewPort[0].TopLeftY = 0;
		mToneViewPort[0].MaxDepth = 1.0f;
		mToneViewPort[0].MinDepth = 0.0f;

		for (auto i = 0u; i < tone_level_count;++i) {
			texDesc.Height = texDesc.Width = 1 << (2 * i)<<2;
			mToneSize[i] = std::make_pair(texDesc.Width, texDesc.Height);
			auto tempTex = leo::win::make_scope_com<ID3D11Texture2D>();
			leo::dxcall(device->CreateTexture2D(&texDesc, nullptr, &tempTex));
			leo::dxcall(device->CreateRenderTargetView(tempTex, nullptr, &mRTVToneMap[i]));
			leo::dxcall(device->CreateShaderResourceView(tempTex, nullptr, &mSRVToneMap[i]));

			mToneViewPort[i] = mToneViewPort[0];
			mToneViewPort[i].Width = static_cast<float>(texDesc.Width);
			mToneViewPort[i].Height =static_cast<float>(texDesc.Height);
			if (i == 0) {
				mToneTex_0 = leo::win::unique_com<ID3D11Texture2D>(tempTex.release());
			}
		}
		texDesc.Height = texDesc.Width = mToneSize[0].first;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		texDesc.Usage = D3D11_USAGE_STAGING;
		texDesc.BindFlags = 0;
		leo::dxcall(device->CreateTexture2D(&texDesc, nullptr, &mLastToneMap));
		#else
		//one res
		#endif
	}

	bool Apply(ID3D11DeviceContext * context) override
	{
		return PostProcess::Apply(context);
	}

	void Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) override
	{
		{
			auto tex = leo::win::make_scope_com<ID3D11Texture2D>(nullptr);
			auto res = leo::win::make_scope_com<ID3D11Resource>(nullptr);
			src->GetResource(&res);
			leo::dxcall(res->QueryInterface(&tex));
			D3D11_TEXTURE2D_DESC desc;
			tex->GetDesc(&desc);

			// Initialize the sample offsets for the initial luminance pass.
			float tU, tV;
			tU = 1.0f / (3.0f * desc.Width);
			tV = 1.0f / (3.0f * desc.Height);

			leo::float2 avSampleOffsets[10];

			auto index = 0;
			for (auto x = -1; x <= 1; x++)
			{
				for (auto y = -1; y <= 1; y++)
				{
					avSampleOffsets[index].x = x * tU;
					avSampleOffsets[index].y = y * tV;

					index++;
				}
			}

			for (auto i = 0; i < index+1;){
				mOffsets[i/2].x = avSampleOffsets[i].x;
				mOffsets[i/2].y = avSampleOffsets[i].y;
				++i;
				mOffsets[i/2].z = avSampleOffsets[i].x;
				mOffsets[i/2].w = avSampleOffsets[i].y;
				++i;
			}

			context->UpdateSubresource(mOffsetsBuffer, 0, nullptr, mOffsets.data(), 0, 0);
			context->PSSetConstantBuffers(0, 1, &mOffsetsBuffer);
		}
		#ifdef NO_GENMIP
		auto curr_level = tone_level_count - 1;
		context->RSSetViewports(1, &mToneViewPort[curr_level]);
		PostProcess::Draw(context, src, mRTVToneMap[curr_level]);
		#else
		PostProcess::Draw(context, src, mRTVToneMap);
		#endif

#ifdef NO_GENMIP
		curr_level--;
		context->PSSetShader(mLumIterativePS, nullptr, 0);
		while (curr_level > -1) {
			CalcSampleOffset(mToneSize[curr_level + 1]);
			context->RSSetViewports(1, &mToneViewPort[curr_level]);
			context->UpdateSubresource(mOffsetsBuffer, 0, nullptr, mOffsets.data(), 0, 0);
			PostProcess::Draw(context, mSRVToneMap[curr_level + 1], mRTVToneMap[curr_level]);
			curr_level--;
		}
#else
#endif
	}

	float GetLumFactor(ID3D11DeviceContext* context, float lumAdapt,float dt)
	{
		D3D11_MAPPED_SUBRESOURCE subRes;
		context->CopyResource(mLastToneMap, mToneTex_0);
		context->Map(mLastToneMap, 0, D3D11_MAP_READ, 0, &subRes);
		float* texData = reinterpret_cast<float*>(subRes.pData);
		#ifndef NO_SINGLE_CHANNEL_FLOAT
		float fResampleSum = 0.f;
		for (auto iSample = 0u; iSample < 16;++iSample)
		{
			fResampleSum += texData[iSample*subRes.RowPitch / 4];
		}
		fResampleSum = exp(fResampleSum / 16.f);
		#else
		float fResampleSum = 0.f;
		for (auto iSample = 0u; iSample < 16;++iSample)
		{
			byte* xyzw = (byte*)texData[iSample*subRes.RowPitch / 4];

			float rgba[4];

			rgba[0] = xyzw[0] / 255.f * 1.f;
			rgba[1] = xyzw[1] / 255.f * 1 / 255.f;
			rgba[2] = xyzw[2] / 255.f * 1 / 65025.f;
			rgba[3] = xyzw[3] / 255.f * 1 / 16581375.f;

			fResampleSum += rgba[0];
			fResampleSum += rgba[1];
			fResampleSum += rgba[2];
			fResampleSum += rgba[3];
		}
		fResampleSum = exp(fResampleSum / 16.f);
		#endif
		context->Unmap(mLastToneMap, 0);

		auto mCurrLum = fResampleSum;
		return lumAdapt + (mCurrLum - lumAdapt) * (1 - pow(0.98f, 30 * dt));
	}
private:
	void CalcSampleOffset(std::pair<UINT,UINT> size)
	{
		float du = 1.f / size.first;
		float dv = 1.f / size.second;

		auto index = 0u;
		for (auto y = 0u; y != 2; ++y)
			for (auto x = 0u;x != 2 / 2;++x)
			{
				mOffsets[index].x = (x * 2 - 1.5f)*du;
				mOffsets[index].y = (y - 1.5f)*dv;

				mOffsets[index].z = mOffsets[index].x + du;
				mOffsets[index].w = mOffsets[index].y;
				++index;
			}
	}
private:
	#ifdef NO_GENMIP
	enum tone_level :leo::uint8 {
		tone_level_two = 0,
		tone_level_three = 1,
		tone_level_four = 2,
		tone_level_count
	};
	leo::win::unique_com<ID3D11ShaderResourceView> mSRVToneMap[tone_level_count];
	leo::win::unique_com<ID3D11Texture2D> mLastToneMap;
	leo::win::unique_com<ID3D11Texture2D> mToneTex_0;
	leo::win::unique_com<ID3D11RenderTargetView> mRTVToneMap[tone_level_count];
	std::pair<UINT, UINT> mToneSize[tone_level_count];
	
	D3D11_VIEWPORT mToneViewPort[tone_level_count];
	#else
	leo::win::unique_com<ID3D11ShaderResourceView> mSRVToneMap;
	leo::win::unique_com<ID3D11RenderTargetView> mRTVToneMap;
	#endif
	leo::win::unique_com<ID3D11Buffer> mOffsetsBuffer = nullptr;
	std::array<leo::float4, 32> mOffsets;


	ID3D11PixelShader* mLumIterativePS = nullptr;
};

//this class can control
// Star Effect and Bloom Effect
// BlueShift and ToneMap impl in GPU ,can't be control
class HDRToneImpl :public leo::PostProcess {

public:
	HDRToneImpl(ID3D11Device* device,ID3D11ShaderResourceView* mScale,float lumAdapt,bool use_bloom = true,bool use_star= false)
		:PostProcess(device),mEffectControl(use_bloom,use_star),mParams(lumAdapt,0.18f,0.5f,1.f)
	{
		D3D11_BUFFER_DESC Desc;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = 0;
		Desc.MiscFlags = 0;
		Desc.StructureByteStride = 0;
		Desc.ByteWidth = sizeof(mParams);

		leo::dxcall(device->CreateBuffer(&Desc, nullptr, &mParamsBuffer));

		BindProcess(device, L"Shader/HDRFinalPS.cso");

		CD3D11_SAMPLER_DESC mSampleDesc{ D3D11_DEFAULT };
		mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;;
		mLinearSS = leo::RenderStates().CreateSamplerState(L"HDRToneImpls1", mSampleDesc);
		
		mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		mPointSS = leo::RenderStates().CreateSamplerState(L"HDRToneImpls0", mSampleDesc);

		mViewPort.TopLeftX = 0;
		mViewPort.TopLeftY = 0;
		mViewPort.MaxDepth = 1.0f;
		mViewPort.MinDepth = 0.0f;
	}

	//NOTE : this function will be create/destory res in another thread
	std::pair<bool, bool> ControlEffect(bool use_bloom, bool use_star)
	{
		if (mEffectControl.first || mEffectControl.second) {

		}

		//Todo
		//Star effect
		if (mEffectControl.second) {


			//Todo
			//because star effect can change bloom behaviour
			if (mEffectControl.first) {

			}
		}
		//Bloom effect
		if (mEffectControl.first) {

		}
	}

	void SetLumAdapt(float lumAdapt) {
		mParams.x = lumAdapt;
	}

	bool Apply(ID3D11DeviceContext * context) override {
		PostProcess::Apply(context);
		context->PSSetConstantBuffers(0, 1, &mParamsBuffer);
		context->UpdateSubresource(mParamsBuffer, 0, nullptr, &mParams, 0, 0);
		ID3D11ShaderResourceView* srvs[] = { mStarResAsIn,mBloomResAsIn };
		context->PSSetShaderResources(1, 2, srvs);
		ID3D11SamplerState* sss[] = { mPointSS,mLinearSS };
		context->PSSetSamplers(0, 2, sss);
		return true;
	}

	void Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) override
	{
		//Bright-pass filtered
		if (mEffectControl.first || mEffectControl.second) {

		}

		//Todo
		//Star effect
		if (mEffectControl.second) {


			//Todo
			//because star effect can change bloom behaviour
			if (mEffectControl.first) {

			}
		}
		//Bloom effect
		if (mEffectControl.first) {

		}
		{
			auto mRes = leo::win::make_scope_com<ID3D11Resource>();
			dst->GetResource(&mRes);
			auto mTex = leo::win::make_scope_com<ID3D11Texture2D>();
			mRes->QueryInterface(&mTex);
			D3D11_TEXTURE2D_DESC desc;
			mTex->GetDesc(&desc);
			mViewPort.Height = desc.Height *1.f;
			mViewPort.Width = desc.Width *1.f;
		}
		context->RSSetViewports(1, &mViewPort);
		PostProcess::Draw(context, src, dst);
	}
private:
	std::pair<bool, bool> mEffectControl;

	leo::win::unique_com<ID3D11RenderTargetView> mBrightResAsOut;
	leo::win::unique_com<ID3D11ShaderResourceView> mBrightResAsIn;

	//defualt = mBrightResAsOut
	//Todo: if(all(mEffectControl) new res;
	ID3D11ShaderResourceView* mBloomIn;

	//Todo:
	//#define NUM_STAR_TEXTURES 12
	leo::win::unique_com<ID3D11RenderTargetView> mStarResAsOut;
	leo::win::unique_com<ID3D11ShaderResourceView> mStarResAsIn;

	leo::win::unique_com<ID3D11RenderTargetView> mBloomResAsOut;
	leo::win::unique_com<ID3D11ShaderResourceView> mBloomResAsIn;

	//float fAdaptedLum;
	//float g_fMiddleGray;//default=0.18f
	//float g_fStarScale;//default=0.5f
	//float g_fBloomScale;//default=1.f
	leo::float4 mParams;
	leo::win::unique_com<ID3D11Buffer> mParamsBuffer = nullptr;

	ID3D11SamplerState* mLinearSS = nullptr;
	ID3D11SamplerState* mPointSS = nullptr;

	D3D11_VIEWPORT mViewPort;
};

class HDRImpl {
public:
	HDRImpl(ID3D11Device* create, ID3D11Texture2D* src, ID3D11RenderTargetView* dst)
	:mScalerProcess(std::make_unique<leo::ScalaerProcess<4>>(create)),
		mMeasureLumProcess(std::make_unique<HDRLuminanceImpl>(create)),
		mToneProcess(std::make_unique<HDRToneImpl>(create,mSrcCopy,mLumAdapt))
	{
		std::thread create_thread(&HDRImpl::create_method, this, create, src, dst);
		create_thread.detach();
	}

	~HDRImpl() {
	}

	void ReSize(ID3D11Device* create, ID3D11Texture2D* src, ID3D11RenderTargetView* dst) {
		std::thread resize_thread(&HDRImpl::resize_method, this, create, src, dst, true);
		resize_thread.detach();
	}

	void Apply(ID3D11DeviceContext* context) {
		//do nothing
	}

	void Draw(ID3D11DeviceContext* context, float dt) {
		while (!ready)
			;
		//TODO:SUPPORT MSAA
		context->CopyResource(mSrcCopyTex, mSrcPtr);
		context->ExecuteCommandList(mCommandList, false);
		//TODO:do by GPU
		mLumAdapt = mMeasureLumProcess->GetLumFactor(context, mLumAdapt, dt);
		mToneProcess->SetLumAdapt(mLumAdapt);
		/*mScalerProcess->Apply(context);
		mScalerProcess->Draw(context, mSrcCopy, mScaleRT);*/
	}
protected:
	void create_method(ID3D11Device* create, ID3D11Texture2D* src, ID3D11RenderTargetView* dst) {
		resize_method(create, src, dst, false);
	}

	void resize_method(ID3D11Device* create, ID3D11Texture2D* src, ID3D11RenderTargetView* dst, bool release = true) {
		if (!mutex.try_lock())
			return;//无视两个连续的resize调用中的后一个
		ready = false;
		if (release) {
			mSrcCopy->Release();
			mSrcCopyTex->Release();
		}
		UINT height, width;
		{
		//first create scale tex res
			D3D11_TEXTURE2D_DESC texDesc;
			src->GetDesc(&texDesc);

			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			leo::dxcall(create->CreateTexture2D(&texDesc, nullptr, &mSrcCopyTex));
			leo::dxcall(create->CreateShaderResourceView(mSrcCopyTex, nullptr, &mSrcCopy));
			mSrcPtr = src;

			D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
			dst->GetDesc(&rtDesc);
			
			texDesc.Width /= 4;
			texDesc.Height /= 4;

			texDesc.Width -= (texDesc.Width % 8);
			texDesc.Height -= (texDesc.Height % 8);

			height = texDesc.Height;
			width = texDesc.Width;

			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			auto mScaleTex = leo::win::make_scope_com<ID3D11Texture2D>(nullptr);
			leo::dxcall(create->CreateTexture2D(&texDesc, nullptr, &mScaleTex));
			leo::dxcall(create->CreateShaderResourceView(mScaleTex, nullptr, &mScaleCopy));
			leo::dxcall(create->CreateRenderTargetView(mScaleTex, nullptr, &mScaleRT));
		}

		once_method(create, width, height,dst);

		ready = true;
		mutex.unlock();
	}

	void once_method(ID3D11Device* create,UINT width,UINT height, ID3D11RenderTargetView* dst) {
		ID3D11DeviceContext* deferredcontext;
		leo::dxcall(create->CreateDeferredContext(0, &deferredcontext));

		mViewPort.Height = height*1.f;
		mViewPort.Width =  width*1.f;
		mViewPort.TopLeftX = 0;
		mViewPort.TopLeftY = 0;
		mViewPort.MaxDepth = 1.0f;
		mViewPort.MinDepth = 0.0f;
		deferredcontext->RSSetViewports(1, &mViewPort);
		mScalerProcess->Apply(deferredcontext);
		mScalerProcess->Draw(deferredcontext, mSrcCopy, mScaleRT);
		mMeasureLumProcess->Apply(deferredcontext);
		mMeasureLumProcess->Draw(deferredcontext, mScaleCopy, nullptr);

		mToneProcess->Apply(deferredcontext);
		mToneProcess->Draw(deferredcontext, mSrcCopy, dst);
		mCommandList.reset(nullptr);
		leo::dxcall(deferredcontext->FinishCommandList(false, &mCommandList));
		deferredcontext->Release();
	}
private:
	using atomic_bool = std::atomic_bool;
	atomic_bool ready = false;
	std::mutex mutex = {};
	leo::win::unique_com<ID3D11CommandList> mCommandList = nullptr;

	//common
	ID3D11Texture2D* mSrcPtr = nullptr;
	leo::win::unique_com<ID3D11Texture2D> mSrcCopyTex = nullptr;
	leo::win::unique_com<ID3D11ShaderResourceView> mSrcCopy = nullptr;

	leo::win::unique_com<ID3D11ShaderResourceView> mScaleCopy = nullptr;
	leo::win::unique_com<ID3D11RenderTargetView> mScaleRT = nullptr;

	std::unique_ptr<leo::PostProcess> mScalerProcess = nullptr;
	std::unique_ptr<HDRLuminanceImpl> mMeasureLumProcess = nullptr;
	std::unique_ptr<HDRToneImpl> mToneProcess = nullptr;

	//dot(float3(0.0f, 0.25f, 0.25f),float3(0.2125f, 0.7154f, 0.0721f));
	float mLumAdapt = exp(log(0.196875f));

	D3D11_VIEWPORT mViewPort;
};
