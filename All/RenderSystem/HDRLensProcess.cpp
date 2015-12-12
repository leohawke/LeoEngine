#include <Core\EngineConfig.h>
#include <Core\FileSearch.h>

#include "HDRProcess.h"
#include "ShaderMgr.h"
#include "CopyProcess.hpp"
#include "BlurProcess.h"
leo::HDRProcess::SqrBright::SqrBright(ID3D11Device * create)
	:PostProcess(create)
{
	BindProcess(create, FileSearch::Search(L"SqrBright.cso")); 

	CD3D11_SAMPLER_DESC mSampleDesc{ D3D11_DEFAULT };
	mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	bilinear_sampler = leo::RenderStates().CreateSamplerState(L"bilinear_sampler", mSampleDesc);
}


void leo::HDRProcess::SqrBright::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
	context->PSSetSamplers(0, 1, &bilinear_sampler);
}


leo::HDRProcess::GlowMerger::GlowMerger(ID3D11Device * create)
	:PostProcess(create)
{
	BindProcess(create, FileSearch::Search(L"GlowMerger.cso"));

	CD3D11_SAMPLER_DESC mSampleDesc{ D3D11_DEFAULT };
	mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	bilinear_sampler = leo::RenderStates().CreateSamplerState(L"bilinear_sampler", mSampleDesc);
}

void leo::HDRProcess::GlowMerger::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
	context->PSSetSamplers(0, 1, &bilinear_sampler);
}


leo::HDRProcess::HDRLensProcess::HDRLensProcess(ID3D11Device * create, ID3D11Texture2D * tex)
	:bright_pass_downsampler(std::make_unique<SqrBright>(create)),glow_merger(std::make_unique<GlowMerger>(create))
{
	for (auto i = 0; i != 2; ++i) {
		downsamplers[i] = leo::Make_CopyProcess(create, leo::bilinear_process);
		blurs[i] = std::make_shared<leo::BlurPorcess<leo::SeparableGaussianFilterProcess>>(create, 8,1.f);
	}
	blurs[2] = std::make_shared<leo::BlurPorcess<leo::SeparableGaussianFilterProcess>>(create, 8, 1.f);

	//bright_pass_input
	Input(create, tex);

	D3D11_TEXTURE2D_DESC desc;
	tex->GetDesc(&desc);
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	auto width = desc.Width;
	auto height = desc.Height;
	//bring_pass_output & blur_inputs[0]
	//downsampler_output[0] & blur_inputs[1]
	//downsampler_output[1] & blur_inputs[2]
	{
		leo::win::unique_com<ID3D11Texture2D> downsample_tex[3];

		for (size_t i = 0; i != 3; ++i) {
			//width /(2<<i),height(2<<i);
			desc.Width = width / (2 << i);
			desc.Height = height / (2 << i);
			leo::dxcall(create->CreateTexture2D(&desc, nullptr, &downsample_tex[i]));
		}

		bright_pass_output_size.first = width / 2;
		bright_pass_output_size.second = height / 2;
		leo::dxcall(create->CreateRenderTargetView(downsample_tex[0], nullptr, &bring_pass_output));
		leo::dxcall(create->CreateShaderResourceView(downsample_tex[0], nullptr, &blur_inputs[0]));

		for (size_t i = 0; i != 2; ++i) {
			leo::dxcall(create->CreateRenderTargetView(downsample_tex[i+1], nullptr, &downsampler_output[i]));
			leo::dxcall(create->CreateShaderResourceView(downsample_tex[i+1], nullptr, &blur_inputs[i+1]));
		}
	}
	//blur_outputs[i] & glow_input[i]
	{
		leo::win::unique_com<ID3D11Texture2D> glow_tex[3];

		for (size_t i = 0; i != 3; ++i) {
			//width /(2<<i),height(2<<i);
			desc.Width = width / (2 << i);
			desc.Height = height / (2 << i);
			leo::dxcall(create->CreateTexture2D(&desc, nullptr, &glow_tex[i]));

			blurs[i]->OutputPin(create,0, desc.Width, desc.Height, desc.Format); 
			blurs[i]->OutputPin(create,1, desc.Width, desc.Height, desc.Format);

			auto context = leo::win::make_scope_com<ID3D11DeviceContext>();
			create->GetImmediateContext(&context);
			blurs[i]->InputPin(context, desc.Width, desc.Height, desc.Format);
		}

		for (size_t i = 0; i != 3; ++i) {
			leo::dxcall(create->CreateRenderTargetView(glow_tex[i], nullptr, &blur_outputs[i]));
			leo::dxcall(create->CreateShaderResourceView(glow_tex[i], nullptr, &glow_input[i]));
		}
	}

	{
		leo::win::unique_com<ID3D11Texture2D> lens_tex;
		leo::dxcall(create->CreateTexture2D(&desc, nullptr, &lens_tex));
		leo::dxcall(create->CreateRenderTargetView(lens_tex, nullptr, &glow_output));
		leo::dxcall(create->CreateShaderResourceView(lens_tex, nullptr,&lens_output));
	}
}

leo::HDRProcess::IHDRLensPorcess::~IHDRLensPorcess()
{
}

void leo::HDRProcess::HDRLensProcess::Apply(ID3D11DeviceContext * context)
{
	//note,the downsamplers use some bilinear_sampler
	bright_pass_downsampler->Apply(context);
	leo::dx::SetViewPort(context, bright_pass_output_size.first, bright_pass_output_size.second);
	bright_pass_downsampler->Draw(context, bright_pass_input, bring_pass_output);
	leo::dx::SetViewPort(context, bright_pass_output_size.first/2, bright_pass_output_size.second/2);
	downsamplers[0]->Draw(context, blur_inputs[0], downsampler_output[0]);
	leo::dx::SetViewPort(context, bright_pass_output_size.first/4, bright_pass_output_size.second/4);
	downsamplers[1]->Draw(context, blur_inputs[1], downsampler_output[1]);

	blurs[0]->Apply(context, blur_inputs[0],blur_outputs[0]);
	blurs[1]->Apply(context, blur_inputs[1], blur_outputs[1]);
	blurs[2]->Apply(context, blur_inputs[2], blur_outputs[2]);

	glow_merger->Apply(context);
	ID3D11RenderTargetView* rt = nullptr;
	context->OMSetRenderTargets(1, &rt, nullptr);
	context->PSSetShaderResources(1, 1, &glow_input[1]);
	context->PSSetShaderResources(2, 1, &glow_input[2]);
	glow_merger->Draw(context, glow_input[0], glow_output);
	context->OMSetRenderTargets(1, &rt, nullptr);
}

ID3D11ShaderResourceView * leo::HDRProcess::HDRLensProcess::Output()
{
	return lens_output.get();
}

void leo::HDRProcess::HDRLensProcess::Input(ID3D11Device * create, ID3D11Texture2D * tex)
{
	leo::win::unique_com<ID3D11ShaderResourceView> bright_pass_input_temp;
	leo::dxcall(create->CreateShaderResourceView(tex,nullptr,&bright_pass_input_temp));
	bright_pass_input.swap(bright_pass_input_temp);
}