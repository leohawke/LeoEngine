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

	std::copy_n(rvalue.mVertexs, 4, mVertexs);
}

void leo::PostProcess::operator=(PostProcess&& rvalue) {
	PostProcess(std::move(rvalue));
}

bool leo::PostProcess::BindProcess(ID3D11Device* device, const std::wstring& psfilename) {
	return BindProcess(device, psfilename.c_str());
}
bool leo::PostProcess::BindProcess(ID3D11Device* device, const wchar_t* psfilename) {
	mPixelShader.reset(ShaderMgr().CreatePixelShader(psfilename));
	return true;
}


bool leo::PostProcess::BindRect(ID3D11Device* device,const ops::Rect& src,const ops::Rect& dst) {

	using leo::ops::axis_system;

	auto dst_ndc = ops::Convert<axis_system::dx_texture_system, axis_system::normalize_device_system>(dst);

	auto rb = dst_ndc.GetRightBottomCornet();
	mVertexs[0].PosH.x = rb.x;
	mVertexs[1].PosH.x = rb.x;
	mVertexs[1].PosH.y = rb.y;
	mVertexs[3].PosH.y = rb.y;

	auto lt = dst_ndc.GetLeftTopCornet();
	mVertexs[2].PosH.x = lt.x;
	mVertexs[3].PosH.x = lt.x;
	mVertexs[2].PosH.y = lt.y;
	mVertexs[0].PosH.y = lt.y;

	rb = src.GetRightBottomCornet();
	lt = src.GetLeftTopCornet();
	mVertexs[0].Tex.x = rb.x;
	mVertexs[1].Tex.x = rb.x;
	mVertexs[1].Tex.y = rb.y;
	mVertexs[3].Tex.y = rb.y;
	mVertexs[2].Tex.x = lt.x;
	mVertexs[3].Tex.x = lt.x;
	mVertexs[2].Tex.y = lt.y;
	mVertexs[0].Tex.y = lt.y;

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
		mVertexBuffer->Release();
		dxcall(device->CreateBuffer(&vb, &vbsubResData, &mVertexBuffer));
		return true;
	}
	Catch_DX_Exception
	return false;
}

bool leo::PostProcess::Apply(ID3D11DeviceContext* context)
{
	context->IASetInputLayout(mCommonThunk.mLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT offsets[] = { 0 };
	UINT strides[] = { sizeof(Vertex) };

	context->IASetVertexBuffers(0, 1, &mVertexBuffer, strides, offsets);

	context->VSSetShader(mCommonThunk.mVertexShader, nullptr, 0);
	context->PSSetShader(mPixelShader, nullptr, 0);
	
	return true;
}

void leo::PostProcess::Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) {
	context->PSSetShaderResources(0, 1, &src);
	context->OMSetRenderTargets(1, &dst, nullptr);
	context->Draw(4, 0);
}


class leo::details::ScalaerProcessDelegate {

};

leo::ScalaerProcess<2>::ScalaerProcess(ID3D11Device * device)
	:PostProcess(device)
{
}

leo::ScalaerProcess<2>::~ScalaerProcess()
{
}

bool leo::ScalaerProcess<2>::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
	return false;
}

leo::ScalaerProcess<4>::ScalaerProcess(ID3D11Device * device)
	:PostProcess(device)
{
}

leo::ScalaerProcess<4>::~ScalaerProcess()
{
}

bool leo::ScalaerProcess<4>::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
	return false;
}


leo::ScalaerProcess<8>::ScalaerProcess(ID3D11Device * device)
	:PostProcess(device)
{
}

leo::ScalaerProcess<8>::~ScalaerProcess()
{
}

bool leo::ScalaerProcess<8>::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
	return false;
}