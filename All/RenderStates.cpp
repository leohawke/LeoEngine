#include "RenderStates.hpp"
#include "Mgr.hpp"
#include "COM.hpp"
#include "exception.hpp"

namespace leo
{
	std::map<std::size_t, RenderStates::RenderState> RenderStates::globalRenderStates;

	RenderStates::Delegate::Delegate()
	{
		auto p = reinterpret_cast<RenderStates*>(0);

		// WireframeRS
		CD3D11_RASTERIZER_DESC wireframeDesc(D3D11_DEFAULT);
		wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
		wireframeDesc.CullMode = D3D11_CULL_BACK;
		wireframeDesc.FrontCounterClockwise = false;
		wireframeDesc.DepthClipEnable = true;
		p->CreateRasterizerState(L"WireframeRS", wireframeDesc);

		// NoCullRS
		CD3D11_RASTERIZER_DESC noCullDesc(D3D11_DEFAULT);
		noCullDesc.FillMode = D3D11_FILL_SOLID;
		noCullDesc.CullMode = D3D11_CULL_NONE;
		noCullDesc.FrontCounterClockwise = false;
		noCullDesc.DepthClipEnable = true;
		p->CreateRasterizerState(L"NoCullRS", noCullDesc);

		//FrontCullRS
		CD3D11_RASTERIZER_DESC frontCullDesc(D3D11_DEFAULT);
		frontCullDesc.FillMode = D3D11_FILL_SOLID;
		frontCullDesc.CullMode = D3D11_CULL_FRONT;
		frontCullDesc.FrontCounterClockwise = false;
		frontCullDesc.DepthClipEnable = true;
		p->CreateRasterizerState(L"FrontCullRS", frontCullDesc);

		// EqualDSS
		CD3D11_DEPTH_STENCIL_DESC equalsDesc(D3D11_DEFAULT);
		equalsDesc.DepthEnable = true;
		equalsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		equalsDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
		p->CreateDepthStencilState(L"EqualDSS", equalsDesc);

		equalsDesc.DepthEnable = false;
		equalsDesc.StencilEnable = false;
		equalsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		p->CreateDepthStencilState(L"NoDepthDSS", equalsDesc);

		// DefaultDSS
		CD3D11_DEPTH_STENCIL_DESC defaultDesc(D3D11_DEFAULT);
		p->CreateDepthStencilState(L"DefaultDSS", defaultDesc);

		// AlphaToCoverageBS
		CD3D11_BLEND_DESC alphaToCoverageDesc(D3D11_DEFAULT);
		alphaToCoverageDesc.AlphaToCoverageEnable = true;
		alphaToCoverageDesc.IndependentBlendEnable = false;
		alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
		alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		p->CreateBlendState(L"AlphaToCoverageBS", alphaToCoverageDesc);

		// TransparentBS
		CD3D11_BLEND_DESC transparentDesc(D3D11_DEFAULT);
		transparentDesc.AlphaToCoverageEnable = false;
		transparentDesc.IndependentBlendEnable = false;
		transparentDesc.RenderTarget[0].BlendEnable = true;
		transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		p->CreateBlendState(L"TransparentBS", transparentDesc);

		// LinearRepeat
		CD3D11_SAMPLER_DESC linearRepeatDesc(D3D11_DEFAULT);
		linearRepeatDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		linearRepeatDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		linearRepeatDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		p->CreateSamplerState(L"LinearRepeat", linearRepeatDesc);

		// DepthMap
		CD3D11_SAMPLER_DESC depthMapDesc(D3D11_DEFAULT);
		depthMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		depthMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		depthMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		depthMapDesc.BorderColor[0] = 0.f;
		depthMapDesc.BorderColor[1] = 0.f;
		depthMapDesc.BorderColor[2] = 0.f;
		depthMapDesc.BorderColor[3] = 1e5f;

		p->CreateSamplerState(L"DepthMap", depthMapDesc);


		CD3D11_SAMPLER_DESC anisoSampler(D3D11_DEFAULT);
		anisoSampler.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		anisoSampler.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		anisoSampler.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		anisoSampler.BorderColor[0] = 0.f;
		anisoSampler.BorderColor[1] = 0.f;
		anisoSampler.BorderColor[2] = 0.f;
		anisoSampler.BorderColor[3] = 0.f;
		p->CreateSamplerState(L"anisoSampler", anisoSampler);

		CD3D11_SAMPLER_DESC trilinearSampler(D3D11_DEFAULT);
		trilinearSampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		trilinearSampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		trilinearSampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		p->CreateSamplerState(L"trilinearSampler", trilinearSampler);

		// NearestClamp
		linearRepeatDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		linearRepeatDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		linearRepeatDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		linearRepeatDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		p->CreateSamplerState(L"NearestClamp", linearRepeatDesc);

		// NearestRepeat
		linearRepeatDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		linearRepeatDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		linearRepeatDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		p->CreateSamplerState(L"NearestRepeat", linearRepeatDesc);

		// LinearClamp
		linearRepeatDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		linearRepeatDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		linearRepeatDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		linearRepeatDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		p->CreateSamplerState(L"LinearClamp", linearRepeatDesc);


		// samShadow
		CD3D11_SAMPLER_DESC samShadow(D3D11_DEFAULT);
		samShadow.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		samShadow.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samShadow.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samShadow.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samShadow.BorderColor[0] = 0.f; samShadow.BorderColor[1] = 0.f;
		samShadow.BorderColor[2] = 0.f; samShadow.BorderColor[3] = 0.f;
		samShadow.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		p->CreateSamplerState(L"samShadow", samShadow);

		// NoDoubleDSS
		CD3D11_DEPTH_STENCIL_DESC noDoubleDesc(D3D11_DEFAULT);
		noDoubleDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		noDoubleDesc.StencilEnable = true;
		D3D11_DEPTH_STENCILOP_DESC frontFace;
		frontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		frontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		frontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		frontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
		noDoubleDesc.FrontFace = frontFace;
		p->CreateDepthStencilState(L"NoDoubleDSS", noDoubleDesc);

		// ShadowBS
		CD3D11_BLEND_DESC shadowDesc(D3D11_DEFAULT);
		shadowDesc.RenderTarget[0].BlendEnable = true;
		shadowDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
		shadowDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		p->CreateBlendState(L"ShadowBS", shadowDesc);

		// MirrorBS
		CD3D11_BLEND_DESC mirrorDesc(D3D11_DEFAULT);
		mirrorDesc.RenderTarget[0].RenderTargetWriteMask = 0;
		p->CreateBlendState(L"MirrorBS", mirrorDesc);
		// MirrorDSS
		noDoubleDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		noDoubleDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		p->CreateDepthStencilState(L"MirrorDSS", noDoubleDesc);

		//ReflectDSS
		noDoubleDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		noDoubleDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		p->CreateDepthStencilState(L"ReflectDSS", noDoubleDesc);
		//ReflectRS
		noCullDesc.FrontCounterClockwise = true;
		p->CreateRasterizerState(L"ReflectRS", noCullDesc);

		//Bias = DepthBias *r(24-bit r = 1/2^24) + SlopeScaledDepthBias*MaxDepthSlope
		CD3D11_RASTERIZER_DESC ShadowMapRS(D3D11_DEFAULT);
		ShadowMapRS.DepthBias = 100000;
		//=>0.006
		ShadowMapRS.DepthBiasClamp = 0.f;
		ShadowMapRS.SlopeScaledDepthBias = 1.f;
		p->CreateRasterizerState(L"ShadowMapRS", ShadowMapRS);
	}

	RenderStates::Delegate::~Delegate()
	{
		for (auto state : globalRenderStates)
		{
			switch (state.second.type)
			{
			case RenderState::RS_TYPE::Blend:
				win::ReleaseCOM(state.second.blend_state);
				break;
			case RenderState::RS_TYPE::DepthStencil:
				win::ReleaseCOM(state.second.depthstencil_state);
				break;
			case RenderState::RS_TYPE::Rasterizer:
				win::ReleaseCOM(state.second.rasterizer_state);
				break;
			case RenderState::RS_TYPE::Sampler:
				win::ReleaseCOM(state.second.sampler_state);
				break;
			default:
				break;
			}
		}
	}

	ID3D11BlendState* RenderStates::CreateBlendState(const wchar_t* name, const D3D11_BLEND_DESC& desc)
	{
		auto sid = hash(name);
		ID3D11BlendState* blendstate = nullptr;
		if (globalRenderStates.find(sid) != globalRenderStates.end())
		{
			auto & rs = globalRenderStates[sid];
			if (rs.type != RenderState::RS_TYPE::Blend)
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "内存里的值错误");
			blendstate = rs.blend_state;
		}
		else
		{
			global::globalD3DDevice->CreateBlendState(&desc, &blendstate);
			dx::DebugCOM(blendstate, name);
			RenderState rs;
			rs.blend_state = blendstate;
			rs.type = RenderState::RS_TYPE::Blend;
			globalRenderStates[sid] = rs;
		}
		return blendstate;
	}
	
	ID3D11DepthStencilState* RenderStates::CreateDepthStencilState(const wchar_t* name, const D3D11_DEPTH_STENCIL_DESC& desc)
	{
		auto sid = hash(name);
		ID3D11DepthStencilState* depthstate = nullptr;
		if (globalRenderStates.find(sid) != globalRenderStates.end())
		{
			auto & rs = globalRenderStates[sid];
			if (rs.type != RenderState::RS_TYPE::DepthStencil)
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "内存里的值错误");
			depthstate = rs.depthstencil_state;
		}
		else
		{
			global::globalD3DDevice->CreateDepthStencilState(&desc, &depthstate);
			dx::DebugCOM(depthstate, name);
			RenderState rs;
			rs.depthstencil_state = depthstate;
			rs.type = RenderState::RS_TYPE::DepthStencil;
			globalRenderStates[sid] = rs;
		}
		return depthstate;
	}

	ID3D11RasterizerState* RenderStates::CreateRasterizerState(const wchar_t* name, const D3D11_RASTERIZER_DESC& desc)
	{
		auto sid = hash(name);
		ID3D11RasterizerState* rasterizerstate = nullptr;
		if (globalRenderStates.find(sid) != globalRenderStates.end())
		{
			auto & rs = globalRenderStates[sid];
			if (rs.type != RenderState::RS_TYPE::Rasterizer)
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "内存里的值错误");
			rasterizerstate = rs.rasterizer_state;
		}
		else
		{
			global::globalD3DDevice->CreateRasterizerState(&desc, &rasterizerstate);
			dx::DebugCOM(rasterizerstate, name);
			RenderState rs;
			rs.rasterizer_state = rasterizerstate;
			rs.type = RenderState::RS_TYPE::Rasterizer;
			globalRenderStates[sid] = rs;
		}
		return rasterizerstate;
	}

	ID3D11SamplerState* RenderStates::CreateSamplerState(const wchar_t* name, const D3D11_SAMPLER_DESC& desc)
	{
		auto sid = hash(name);
		ID3D11SamplerState* samplestate = nullptr;
		if (globalRenderStates.find(sid) != globalRenderStates.end())
		{
			auto & rs = globalRenderStates[sid];
			if (rs.type != RenderState::RS_TYPE::Sampler)
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "内存里的值错误");
			samplestate = rs.sampler_state;
		}
		else
		{
			global::globalD3DDevice->CreateSamplerState(&desc, &samplestate);
			dx::DebugCOM(samplestate,name);
			RenderState rs;
			rs.sampler_state = samplestate;
			rs.type = RenderState::RS_TYPE::Sampler;
			globalRenderStates[sid] = rs;
		}
		return samplestate;
	}

	ID3D11BlendState* RenderStates::GetBlendState(const wchar_t * name)
	{
		return globalRenderStates[hash(name)].blend_state;
	}

	ID3D11DepthStencilState* RenderStates::GetDepthStencilState(const wchar_t * name)
	{
		return globalRenderStates[hash(name)].depthstencil_state;
	}

	ID3D11RasterizerState* RenderStates::GetRasterizerState(const wchar_t * name)
	{
		return globalRenderStates[hash(name)].rasterizer_state;
	}

	ID3D11SamplerState* RenderStates::GetSamplerState(const wchar_t * name)
	{
		return globalRenderStates[hash(name)].sampler_state;
	}
}