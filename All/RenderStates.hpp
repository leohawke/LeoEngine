#pragma once
#include "d3dx11.hpp"
#include "IndePlatform\Singleton.hpp"
namespace leo
{
	class RenderStates
	{
	
		class Delegate : public Singleton<Delegate>
		{
		protected:
			Delegate();
		public:
			~Delegate();
		public:
			static const std::unique_ptr<Delegate>& GetInstance()
			{
				static auto mInstance = unique_raw(new Delegate());
				return mInstance;
			}
		};

	public:
		RenderStates()
		{
			Delegate::GetInstance();
		}

		ID3D11BlendState* CreateBlendState(const wchar_t* name,const D3D11_BLEND_DESC& desc);
		ID3D11DepthStencilState* CreateDepthStencilState(const wchar_t* name,const D3D11_DEPTH_STENCIL_DESC& desc);
		ID3D11RasterizerState* CreateRasterizerState(const wchar_t* name, const D3D11_RASTERIZER_DESC& desc);
		ID3D11SamplerState* CreateSamplerState(const wchar_t* name, const D3D11_SAMPLER_DESC& desc);

		ID3D11BlendState* GetBlendState(const wchar_t * name);
		ID3D11DepthStencilState* GetDepthStencilState(const wchar_t * name);
		ID3D11RasterizerState* GetRasterizerState(const wchar_t * name);
		ID3D11SamplerState* GetSamplerState(const wchar_t * name);
	private:
		struct RenderState
		{
			union
			{
				ID3D11BlendState* blend_state;
				ID3D11DepthStencilState* depthstencil_state;
				ID3D11RasterizerState* rasterizer_state;
				ID3D11SamplerState* sampler_state;
			};
			enum class RS_TYPE {Blend,DepthStencil,Rasterizer,Sampler} type = RS_TYPE::Blend;
		};
		static std::map<std::size_t, RenderState> globalRenderStates;
	};
}