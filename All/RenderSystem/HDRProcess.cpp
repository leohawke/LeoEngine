#include <Core\EngineConfig.h>
#include <Core\FileSearch.h>

#include "HDRProcess.h"
#include "ShaderMgr.h"

 ID3D11Buffer* leo::HDRProcess::HDRCommon::mGpuParams = nullptr;//sizeof = float4[2]
 ID3D11VertexShader* leo::HDRProcess::HDRCommon::mLumVS = nullptr;

 ID3D11SamplerState* leo::HDRProcess::HDRCommon::src_sampler = nullptr;
 ID3D11SamplerState* leo::HDRProcess::HDRCommon::last_lum_sampler = nullptr;

leo::HDRProcess::HDRCommon::HDRCommon(ID3D11Device * create)
	:PostProcess(create)
{
	if (!mGpuParams) {
		D3D11_BUFFER_DESC Desc;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = 0;
		Desc.MiscFlags = 0;
		Desc.StructureByteStride = 0;
		Desc.ByteWidth = sizeof(mCpuParams);

		leo::dxcall(create->CreateBuffer(&Desc, nullptr, &mGpuParams));
	}

	if (!mLumVS) {
		mLumVS = ShaderMgr().CreateVertexShader(
			FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"postprocess", D3D11_VERTEX_SHADER))
			);
	}

	if (!src_sampler) {

	}

	if (!last_lum_sampler) {

	}
}

void leo::HDRProcess::HDRCommon::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
}

void leo::HDRProcess::HDRCommon::GetSampleOffset(UINT width, UINT height)
{
}

void leo::HDRProcess::LumLogProcess::Draw(ID3D11DeviceContext * context, ID3D11ShaderResourceView * src, ID3D11RenderTargetView * dst)
{
}

void leo::HDRProcess::LumIterativeProcess::Draw(ID3D11DeviceContext * context, ID3D11ShaderResourceView * src, ID3D11RenderTargetView * dst)
{
}

void leo::HDRProcess::LumAdaptedProcess::Draw(ID3D11DeviceContext * context, ID3D11ShaderResourceView * src, ID3D11RenderTargetView * dst)
{
}
