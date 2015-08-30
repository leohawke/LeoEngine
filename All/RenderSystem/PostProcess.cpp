#include <platform.h>
#include <exception.hpp>

#include <Core\EngineConfig.h>
#include <Core\FileSearch.h>


#include "d3dx11.hpp"
#include "ShaderMgr.h"
#include "PostProcess.hpp"

decltype(leo::PostProcess::mCommonThunk) leo::PostProcess::mCommonThunk;

namespace {
	
	D3D11_INPUT_ELEMENT_DESC elements_Desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,loffsetof(leo::PostProcess::Vertex,PosH), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,loffsetof(leo::PostProcess::Vertex,Tex), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
}

leo::PostProcess::PostProcess(ID3D11Device* device){
	++mCommonThunk.mRefCount;
	if (mCommonThunk.mRefCount && !mCommonThunk.mVertexShader) {
		mCommonThunk.mVertexShader = ShaderMgr().CreateVertexShader(
			FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"postprocess", D3D11_VERTEX_SHADER)),
			nullptr,
			elements_Desc,
			2, &mCommonThunk.mLayout
			);
	}

	CD3D11_BUFFER_DESC vb;
	vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb.ByteWidth = sizeof(mVertexs);
	vb.CPUAccessFlags = 0;
	vb.MiscFlags = 0;
	vb.StructureByteStride = 0;
	vb.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA vbsubResData;
	vbsubResData.pSysMem = mVertexs;

	try {
		dxcall(device->CreateBuffer(&vb, &vbsubResData, &mVertexBuffer));
	}
	Catch_DX_Exception
}

leo::PostProcess::~PostProcess() {
	--mCommonThunk.mRefCount;
	if (!mCommonThunk.mRefCount && mCommonThunk.mVertexShader) {
		//do nothing
	}
}

leo::PostProcess::PostProcess(PostProcess && rvalue)
:mPixelShader(std::move(rvalue.mPixelShader)),mVertexBuffer(std::move(rvalue.mVertexBuffer)){
	assert(mCommonThunk.mRefCount);
	++mCommonThunk.mRefCount;
}

void leo::PostProcess::operator=(PostProcess&& rvalue) {
	PostProcess(std::move(rvalue));
}

bool leo::PostProcess::BindProcess(ID3D11Device* device, const std::string& psfilename) {
	return BindProcess(device, psfilename.c_str());
}
bool leo::PostProcess::BindProcess(ID3D11Device* device, const char* psfilename) {
	return false;
}


bool leo::PostProcess::BindRect(ops::Rect& src, ops::Rect& dst) {
	return false;
}

bool leo::PostProcess::Apply()
{
	return false;
}

void leo::PostProcess::Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) {

}