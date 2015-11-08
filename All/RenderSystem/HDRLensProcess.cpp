#include <Core\EngineConfig.h>
#include <Core\FileSearch.h>

#include "HDRProcess.h"
#include "ShaderMgr.h"
#include "CopyProcess.hpp"

leo::HDRProcess::SqrBright::SqrBright(ID3D11Device * create)
	:PostProcess(create)
{
	BindProcess(create,FileSearch::Search(L"SqrBright.cso"));

	CD3D11_SAMPLER_DESC mSampleDesc{ D3D11_DEFAULT };
	mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	bilinear_sampler = leo::RenderStates().CreateSamplerState(L"bilinear_sampler", mSampleDesc);
}


void leo::HDRProcess::SqrBright::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
	context->PSSetSamplers(0, 1, &bilinear_sampler);
}


void leo::HDRProcess::GlowMerger::Apply(ID3D11DeviceContext * context)
{
}


leo::HDRProcess::HDRLensProcess::HDRLensProcess(ID3D11Device * create, ID3D11Texture2D * tex)
{
	leo::dxcall(create->CreateShaderResourceView(tex, nullptr, &bright_pass_input));

	//bring_pass_output & downsampler_inputs[0]
	//downsampler_output[0] & downsampler_inputs[1]
	{
		leo::win::unique_com<ID3D11Texture2D> downsample_tex[2];

		leo::dxcall(create->CreateRenderTargetView(downsample_tex[0], nullptr, &bring_pass_output));
		leo::dxcall(create->CreateShaderResourceView(downsample_tex[0], nullptr, &downsampler_inputs[0]));

		leo::dxcall(create->CreateRenderTargetView(downsample_tex[1], nullptr, &downsampler_output[0]));
		leo::dxcall(create->CreateShaderResourceView(downsample_tex[1], nullptr, &downsampler_inputs[1]));
	}

	//downsampler_output[1]
	{

	}
}

leo::HDRProcess::IHDRLensPorcess::~IHDRLensPorcess()
{
}

void leo::HDRProcess::HDRLensProcess::Apply(ID3D11DeviceContext * context)
{
	//note,the downsamplers use some bilinear_sampler
	bright_pass_downsampler->Apply(context);

	bright_pass_downsampler->Draw(context, bright_pass_input, bring_pass_output);
	downsamplers[0]->Draw(context, downsampler_inputs[0], downsampler_output[0]);
	downsamplers[1]->Draw(context, downsampler_inputs[1], downsampler_output[1]);

	blurs[0]->Apply(context);
	blurs[0]->Draw(context, blur_inputs[0],blur_outputs[0]);
	blurs[1]->Draw(context, blur_inputs[1], blur_outputs[1]);
	blurs[2]->Draw(context, blur_inputs[2], blur_outputs[2]);

	glow_merger->Apply(context);
	//add set;
	glow_merger->Draw(context, glow_input, glow_output);
}

ID3D11ShaderResourceView * leo::HDRProcess::HDRLensProcess::Output()
{
	return nullptr;
}

void leo::HDRProcess::HDRLensProcess::Input(ID3D11Device *, ID3D11Texture2D * tex)
{
}