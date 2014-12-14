#include "..\d3dx11.hpp"
#include "Camera.hpp"
#include "ShadowMap.hpp"
#include "EffectShadowMap.hpp"
#include "..\exception.hpp"
namespace leo {
	class ShadowMapDelegate :CONCRETE(ShadowMap), public Singleton<ShadowMapDelegate>
	{
	public:
		ShadowMapDelegate(ID3D11Device* device, std::pair<uint16, uint16> resolution)
			:mResloution(resolution){
			mViewPort.TopLeftX = 0.f;
			mViewPort.TopLeftY = 0.f;
			mViewPort.Width = mResloution.first;
			mViewPort.Height = mResloution.second;
			mViewPort.MinDepth = 0.f;
			mViewPort.MaxDepth = 1.0f;

			// Use typeless format because the DSV is going to interpret 
			// the bits as DXGI_FORMAT_D24_UNORM_S8_UINT, whereas the SRV is going 
			// to interpret the bits as DXGI_FORMAT_R24_UNORM_X8_TYPELESS. 
			D3D11_TEXTURE2D_DESC texDesc; 
			texDesc.Width =static_cast<UINT>(mViewPort.Width);
			texDesc.Height = static_cast<UINT>(mViewPort.Height);
			texDesc.MipLevels = 1; 
			texDesc.ArraySize = 1; 
			texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; 
			texDesc.SampleDesc.Count = 1; 
			texDesc.SampleDesc.Quality = 0; 
			texDesc.Usage = D3D11_USAGE_DEFAULT; 
			texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; 
			texDesc.CPUAccessFlags = 0; texDesc.MiscFlags = 0; 
			ID3D11Texture2D* depthMap = 0;
			try {
				device->CreateTexture2D(&texDesc, 0, &depthMap);
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
				dsvDesc.Flags = 0; 
				dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; 
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; 
				dsvDesc.Texture2D.MipSlice = 0;
				device->CreateDepthStencilView(depthMap, &dsvDesc, &mDepthDSV); 
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
				srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; 
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D; 
				srvDesc.Texture2D.MipLevels = texDesc.MipLevels; 
				srvDesc.Texture2D.MostDetailedMip = 0; 
				device->CreateShaderResourceView(depthMap, &srvDesc, &mDepthSRV);
				// View saves a reference to the texture so we can release our 
				// reference. 
				leo::win::ReleaseCOM(depthMap);
			}
			Catch_DX_Exception

		}


		~ShadowMapDelegate() {
			leo::win::ReleaseCOM(mDepthDSV);
			leo::win::ReleaseCOM(mDepthSRV);
			leo::win::ReleaseCOM(mPreRTV);
			leo::win::ReleaseCOM(mPrevDSV);
		}

		ID3D11ShaderResourceView* GetDepthSRV() {
			return mDepthSRV;
		}
		void BeginShadowMap(ID3D11DeviceContext* context, const CastShadowCamera& camera) {
			UINT numVP = 1;
			context->RSGetViewports(&numVP, &mPrevViewPort);
			context->OMGetRenderTargets(1, &mPreRTV, &mPrevDSV);

			context->RSSetViewports(1, &mViewPort);
			ID3D11RenderTargetView* renderTargets[1] = { nullptr };
			context->OMSetRenderTargets(1, renderTargets, mDepthDSV);
			context->ClearDepthStencilView(mDepthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

			EffectShadowMap::GetInstance()->ViewProjMatrix(camera.ViewProj());
			EffectShadowMap::GetInstance()->Apply(context);
		}
		void EndShadowMap(ID3D11DeviceContext* context) {
			context->RSSetViewports(1, &mPrevViewPort);
			context->OMSetRenderTargets(1, &mPreRTV, mPrevDSV);
		}
	private:
		std::pair<uint16, uint16> mResloution;
		ID3D11ShaderResourceView* mDepthSRV = nullptr;
		ID3D11DepthStencilView* mDepthDSV = nullptr;
		D3D11_VIEWPORT mViewPort;

		D3D11_VIEWPORT mPrevViewPort;
		ID3D11DepthStencilView* mPrevDSV = nullptr;
		ID3D11RenderTargetView* mPreRTV = nullptr;
	};

	ShadowMap& ShadowMap::GetInstance(ID3D11Device* device, std::pair<uint16, uint16> resolution) {
		static ShadowMapDelegate Instance{ device,resolution };
		return Instance;
	}

	ID3D11ShaderResourceView* ShadowMap::GetDepthSRV() {
		lassume(dynamic_cast<ShadowMapDelegate *>(this));

		return ((ShadowMapDelegate *)this)->GetDepthSRV(
			);
	}
	void ShadowMap::BeginShadowMap(ID3D11DeviceContext* context, const CastShadowCamera& camera) {
		lassume(dynamic_cast<ShadowMapDelegate *>(this));

		return ((ShadowMapDelegate *)this)->BeginShadowMap(context,camera
			);
	}
	void ShadowMap::EndShadowMap(ID3D11DeviceContext* context) {
		lassume(dynamic_cast<ShadowMapDelegate *>(this));

		return ((ShadowMapDelegate *)this)->EndShadowMap(context
			);
	}
}