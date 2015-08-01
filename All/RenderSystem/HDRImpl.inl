#include <atomic>
#include <thread>
#include <mutex>
#include <platform.h>
#include <Core\COM.hpp>
#include <Core\FileSearch.h>
#include <Core\Camera.hpp>
#include <Core\BilateralFilter.hpp>
#include <Core\EffectQuad.hpp>
#include <exception.hpp>
#include "d3dx11.hpp"

class HDRImpl {
public:
	HDRImpl(ID3D11Device* create, ID3D11Texture2D* src, ID3D11RenderTargetView* dst) {
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
		context->ExecuteCommandList(mCommandList, false);
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
		}

		//some resourecr don't need resize
		static std::once_flag once_flag;
		std::call_once(once_flag, &HDRImpl::once_method, this, create);

		ready = true;
		mutex.unlock();
	}

	void once_method(ID3D11Device* create) {
		ID3D11DeviceContext* deferredcontext;
		leo::dxcall(create->CreateDeferredContext(0, &deferredcontext));

		deferredcontext->CopyResource(mSrcCopyTex, mSrcPtr);

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
};
