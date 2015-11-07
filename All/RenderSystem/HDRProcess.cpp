#include <Core\EngineConfig.h>
#include <Core\FileSearch.h>

#include "HDRProcess.h"
#include "ShaderMgr.h"
#include "CopyProcess.hpp"


leo::HDRProcess::HDRProcess(ID3D11Device * create, ID3D11Texture2D* src)
	:PostProcess(create),
	mStatProcess(std::make_unique<HDRStatProcess>(create, src)),
	mToneCpuParams(1.f, 0.25f)
{
#ifdef NO_SINGLE_CHANNEL_FLOAT
	BindProcess(create, "Shader/HDRFinalPS_multiple.cso");
#else
	BindProcess(create, L"Shader/HDRFinalPS_single.cso");
#endif

	leo::dx::CreateGPUCBuffer(create, mToneCpuParams, mToneGpuParams, "tone_params");
}

void leo::HDRProcess::SetFrameDelta(float dt)
{
	mDt = dt;
}

void leo::HDRProcess::Apply(ID3D11DeviceContext * context)
{
	mStatProcess->Apply(context, mDt);
	auto src = mStatProcess->Output();
	context->PSSetShaderResources(2, 1, &src);
	context->PSSetConstantBuffers(0, 1, &mToneGpuParams);
	PostProcess::Apply(context);
}

void leo::HDRProcess::ReSize(ID3D11Device * create, ID3D11Texture2D * src)
{
}

