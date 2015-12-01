#include <exception.hpp>
#include "BlurProcess.h"
#include "RenderStates.hpp"
#include "ShaderMgr.h"
#include <Core\FileSearch.h>
leo::SeparableGaussianFilterProcess::SeparableGaussianFilterProcess(ID3D11Device * create, int kernel_radius, float multiplier, bool x_dir)
	:PostProcess(create), mXDir(x_dir),mKernelRadius(kernel_radius),mMuliplier(multiplier)
{
	CD3D11_SAMPLER_DESC mSampleDesc{ D3D11_DEFAULT };
	mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	bilinear_sampler = leo::RenderStates().CreateSamplerState(L"bilinear_sampler", mSampleDesc);

	leo::dx::CreateGPUCBuffer<decltype(cpu_params)>(create, gpu_params, "blur_params");

	mBlurVS = ShaderMgr().CreateVertexShader(
		FileSearch::Search(x_dir?L"BlurXVS.cso":L"BlurYVS.cso")
		);

	BindProcess(create, FileSearch::Search(x_dir ? L"BlurXPS.cso" : L"BlurYPS.cso"));
}

void leo::SeparableGaussianFilterProcess::InputPin(ID3D11DeviceContext * context, uint32 width, uint32 height, uint32 format)
{
	CalcSampleOffsets(mXDir ? width : height, 3.f);
	context->UpdateSubresource(gpu_params, 0, nullptr, cpu_params, 0, 0);
}

void leo::SeparableGaussianFilterProcess::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
	context->VSSetShader(mBlurVS, nullptr, 0);
	context->VSSetConstantBuffers(0,1, &gpu_params);
	context->PSSetConstantBuffers(0, 1, &gpu_params);
	context->PSSetSamplers(0, 1, &bilinear_sampler);
}

float leo::SeparableGaussianFilterProcess::GaussianDistribution(float x, float y, float rho)
{
	auto g = 1.f / sqrt(2.f*leo::LM_PI*rho*rho);
	g *= exp(-(x*x + y*y) / (2 * rho*rho));
	return g;
}

void leo::SeparableGaussianFilterProcess::CalcSampleOffsets(uint32 tex_size, float deviation)
{
	std::array<float, 8> color_weight = {};
	std::array<float, 8> tex_coord_offset = {};

	std::vector<float> tmp_weights(mKernelRadius * 2,0);
	std::vector<float> tmp_offset(mKernelRadius * 2, 0);

	const auto tu = 1.f / tex_size;

	auto sum_weight = 0.f;

	for (auto i = 0; i < 2 * mKernelRadius; ++i) {
		auto weight = GaussianDistribution(static_cast<float>(i-mKernelRadius), 0, mKernelRadius / deviation);
		tmp_weights[i] = weight;
		sum_weight += weight;
	}

	for (auto i = 0; i < 2 * mKernelRadius; ++i) {
		tmp_weights[i] /= sum_weight;
	}

	//Fill the offsets
	for (auto i = 0; i < mKernelRadius; ++i) {
		tmp_offset[i] = static_cast<float>(i - mKernelRadius);
		tmp_offset[i + mKernelRadius] = static_cast<float>(i);
	}

	// Bilinear filtering taps
	// Ordering is left to right.
	for (int i = 0; i < mKernelRadius; ++i)
	{
		float const scale = tmp_weights[i * 2] + tmp_weights[i * 2 + 1];
		float const frac = tmp_weights[i * 2] / scale;

		tex_coord_offset[i] = (tmp_offset[i * 2] + (1 - frac)) * tu;
		color_weight[i] = mMuliplier * scale;
	}

	cpu_params[0] =static_cast<float>(tex_size);
	cpu_params[1] = 1.f / tex_size;
	for (auto i = 0; i != 8; ++i) {
		cpu_params[i + 4] = color_weight[i];
		cpu_params[i + 12] = tex_coord_offset[i];
	}
}
