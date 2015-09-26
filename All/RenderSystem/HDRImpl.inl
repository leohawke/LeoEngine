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
			1,0,//ARRAT & MIP
			D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE  //BindFlag
		};
		#ifdef NO_GENMIP
		//tone_level_count res
		#ifdef NO_SINGLE_CHANNEL_FLOAT
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		BindProcess(device, "Shader/LumLogIntialPS_multiple.cso");
		#else
		BindProcess(device, "Shader/LumLogIntialPS_single.cso");
		#endif
		for (auto i = 0u; i < tone_level_count;++i) {
			texDesc.Height = texDesc.Width = 1 << (2 * i)<<2;
			auto tempTex = leo::win::make_scope_com<ID3D11Texture2D>();
			leo::dxcall(device->CreateTexture2D(&texDesc, nullptr, &tempTex));
			leo::dxcall(device->CreateRenderTargetView(tempTex, nullptr, &mRTVToneMap[i]));
			leo::dxcall(device->CreateShaderResourceView(tempTex, nullptr, &mSRVToneMap[i]));
			if (i == 0)
				mLastToneMap = leo::win::unique_com<ID3D11Texture2D>(tempTex.release());
		}
		#else
		//one res
		#endif
	}

	bool Apply(ID3D11DeviceContext * context) override
	{
		PostProcess::Apply(context);
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
		PostProcess::Draw(context, src, mRTVToneMap[curr_level]);
		#else
		PostProcess::Draw(context, src, mRTVToneMap);
		#endif

#ifdef NO_GENMIP
		curr_level--;

		while (curr_level > 0) {
			CalcSampleOffset(mToneSize[curr_level + 1]);
			context->UpdateSubresource(mOffsetsBuffer, 0, nullptr, mOffsets.data(), 0, 0);
			PostProcess::Draw(context, mSRVToneMap[curr_level + 1], mRTVToneMap[curr_level]);
			curr_level--;
		}
		D3D11_MAPPED_SUBRESOURCE subRes;
		context->Map(mLastToneMap, 0, D3D11_MAP_READ, 0, &subRes);
		float* texData = subRes.pData;
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
			rgba[1] = xyzw[1] / 255.f * 1/255.f;
			rgba[2] = xyzw[2] / 255.f * 1/65025.f;
			rgba[3] = xyzw[3] / 255.f * 1/ 16581375.f;

			fResampleSum += rgba[0];
			fResampleSum += rgba[1];
			fResampleSum += rgba[2];
			fResampleSum += rgba[3];
		}
		fResampleSum = exp(fResampleSum / 16.f);
		#endif
		context->Unmap(mLastTomeMap, 0);
#else
#endif
		auto mCurrLum = fResampleSum;
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
	leo::win::unique_com<ID3D11RenderTargetView> mRTVToneMap[tone_level_count];
	#else
	leo::win::unique_com<ID3D11ShaderResourceView> mSRVToneMap;
	leo::win::unique_com<ID3D11RenderTargetView> mRTVToneMap;
	#endif
	leo::win::unique_com<ID3D11Buffer> mOffsetsBuffer = nullptr;
	std::array<leo::float4, 32> mOffsets;

	std::pair<UINT, UINT> mToneSize[tone_level_count];

	float mLastLum;
};

class HDRImpl {
public:
	HDRImpl(ID3D11Device* create, ID3D11Texture2D* src, ID3D11RenderTargetView* dst)
	:mScalerProcess(std::make_unique<leo::ScalaerProcess<4>>(create)){
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

	void Draw(ID3D11DeviceContext* context) {
		while (!ready)
			;
		//TODO:SUPPORT MSAA
		context->CopyResource(mSrcCopyTex, mSrcPtr);
		context->ExecuteCommandList(mCommandList, false);
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

		once_method(create, width, height);

		ready = true;
		mutex.unlock();
	}

	void once_method(ID3D11Device* create,UINT width,UINT height) {
		ID3D11DeviceContext* deferredcontext;
		leo::dxcall(create->CreateDeferredContext(0, &deferredcontext));

		D3D11_VIEWPORT vp;
		vp.Height = height*1.f;
		vp.Width =  width*1.f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.MaxDepth = 1.0f;
		vp.MinDepth = 0.0f;
		deferredcontext->RSSetViewports(1, &vp);

		mScalerProcess->Apply(deferredcontext);
		mScalerProcess->Draw(deferredcontext, mSrcCopy, mScaleRT);

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
};
