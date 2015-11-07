#include <Core\EngineConfig.h>
#include <Core\FileSearch.h>

#include "HDRProcess.h"
#include "ShaderMgr.h"
#include "CopyProcess.hpp"

leo::HDRProcess::HDRLensProcess::HDRLensProcess(ID3D11Device * create, ID3D11Texture2D * tex)
{
}

leo::HDRProcess::IHDRLensPorcess::~IHDRLensPorcess()
{
}

void leo::HDRProcess::HDRLensProcess::Apply(ID3D11DeviceContext *)
{
}

ID3D11ShaderResourceView * leo::HDRProcess::HDRLensProcess::Output()
{
	return nullptr;
}

void leo::HDRProcess::HDRLensProcess::Input(ID3D11Device *, ID3D11Texture2D * tex)
{
}