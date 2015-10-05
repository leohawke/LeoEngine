#include <Core\EngineConfig.h>
#include <Core\FileSearch.h>

#include "HDRProcess.h"
#include "ShaderMgr.h"

 ID3D11Buffer* leo::HDRProcess::HDRCommon::mGpuParams = nullptr;//sizeof = float4[2]
 ID3D11VertexShader* leo::HDRProcess::HDRCommon::mLumVS = nullptr;

 ID3D11SamplerState* leo::HDRProcess::HDRCommon::src_sampler = nullptr;
 ID3D11SamplerState* leo::HDRProcess::HDRCommon::last_lum_sampler = nullptr;

leo::HDRProcess::HDRCommon::HDRCommon(ID3D11Device * create)
	:PostProcess(create),mTexDesc(DXGI_FORMAT_R32_FLOAT,//FORMAT
		1, 1,//SIZE
		1, 1,//ARRAT & MIP
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE  //BindFlag)
		)
{
	#ifdef NO_SINGLE_CHANNEL_FLOAT
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	#endif

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

ID3D11ShaderResourceView * leo::HDRProcess::LumLogProcess::Output() const
{
	return mLumLogOutput;
}

void leo::HDRProcess::LumLogProcess::Draw(ID3D11DeviceContext * context, ID3D11ShaderResourceView * src, ID3D11RenderTargetView * dst)
{
	GetSampleOffset(mTexDesc.Width, mTexDesc.Height);
	context->UpdateSubresource(mGpuParams, 0, nullptr, &mCpuParams, 0, 0);
	PostProcess::Draw(context, src, mLumLogRTV);
}

leo::HDRProcess::LumIterativeProcess::LumIterativeProcess(ID3D11Device * create, unsigned level)
	:HDRCommon(create)
{
}

void leo::HDRProcess::LumIterativeProcess::Draw(ID3D11DeviceContext * context, ID3D11ShaderResourceView * src, ID3D11RenderTargetView * dst)
{
}

leo::HDRProcess::LumAdaptedProcess::LumAdaptedProcess(ID3D11Device * create)
	:HDRCommon(create)
{
}

void leo::HDRProcess::LumAdaptedProcess::Draw(ID3D11DeviceContext * context, ID3D11ShaderResourceView * src, ID3D11RenderTargetView * dst)
{
}
