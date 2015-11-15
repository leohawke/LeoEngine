#include <platform.h>
#include <exception.hpp>
#include <memory.hpp>

#include <Core\EngineConfig.h>
#include <Core\FileSearch.h>


#include "d3dx11.hpp"
#include "ShaderMgr.h"
#include "RenderStates.hpp"
#include "PostProcess.hpp"


decltype(leo::PostProcess::mCommonThunk) leo::PostProcess::mCommonThunk;

namespace {

	D3D11_INPUT_ELEMENT_DESC elements_Desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,loffsetof(leo::PostProcess::Vertex,PosH), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,loffsetof(leo::PostProcess::Vertex,Tex), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
}

leo::PostProcess::PostProcess(ID3D11Device* device) {
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
	vb.Usage = D3D11_USAGE_DEFAULT;

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
	:mPixelShader(std::move(rvalue.mPixelShader)), mVertexBuffer(std::move(rvalue.mVertexBuffer)) {
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
	mPixelShader = (ShaderMgr().CreatePixelShader(psfilename));
	return true;
}


bool leo::PostProcess::BindRect(ID3D11DeviceContext* context, const ops::Rect& src, const ops::Rect& dst) {

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

	context->UpdateSubresource(mVertexBuffer, 0, nullptr, mVertexs, 0, 0);

	return true;
}

void leo::PostProcess::Apply(ID3D11DeviceContext* context)
{
	context->IASetInputLayout(mCommonThunk.mLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT offsets[] = { 0 };
	UINT strides[] = { sizeof(Vertex) };

	context->IASetVertexBuffers(0, 1, &mVertexBuffer, strides, offsets);

	context->VSSetShader(mCommonThunk.mVertexShader, nullptr, 0);
}

void leo::PostProcess::Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) {
	context->OMSetRenderTargets(1, &dst, nullptr);
	context->PSSetShaderResources(0, 1, &src);
	context->PSSetShader(mPixelShader, nullptr, 0);
	context->Draw(4, 0);
}


class leo::details::ScalaerProcessDelegate :public leo::PassAlloc {
public:
	ScalaerProcessDelegate(ID3D11Device* device, int level, PostProcess* container)
	{
		//TODO: 需要进行设备检查
		auto filename = EngineConfig::ShaderConfig::GetShaderFileName(L"ScalaerProcessPS", D3D11_PIXEL_SHADER);
		auto pos = filename.find(L"_2");
		if (pos == std::wstring::npos)
			throw std::runtime_error("ScalaerProcessPS Error:Invalid FileName");

		filename.replace(pos, 2, std::wstring(L"_") + std::to_wstring(level));
		filename = FileSearch::Search(filename);
		(container)->BindProcess(device, filename);

		//TODO:计算坐标

		D3D11_BUFFER_DESC Desc;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = 0;
		Desc.MiscFlags = 0;
		Desc.StructureByteStride = 0;
		Desc.ByteWidth = sizeof(mOffsets);

		dxcall(device->CreateBuffer(&Desc, nullptr, &mOffsetsBuffer));

		CD3D11_SAMPLER_DESC mSampleDesc{ D3D11_DEFAULT };
		mSampleDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		mSamplerState = RenderStates().CreateSamplerState(L"ScalaerProcessDelegate", mSampleDesc);
	}

	template<uint16 Len>
	void SetSampleOffset(const std::array<float4, Len>& offset)
	{
		std::copy(offset.begin(), offset.end(), mOffsets.begin());
	}

	void Update(ID3D11DeviceContext* context)
	{
		context->UpdateSubresource(mOffsetsBuffer, 0, nullptr, mOffsets.data(), 0, 0);
	}

	void Apply(ID3D11DeviceContext* context)
	{
		context->PSSetSamplers(0, 1, &mSamplerState);
		context->PSSetConstantBuffers(0, 1, &mOffsetsBuffer);
	}

	ops::Rect GetTextureRect(D3D11_TEXTURE2D_DESC desc, int level)
	{
		auto rect =  ops::IRect<ops::axis_system::dx_texture_system>();

		float du = 1.f / desc.Width;
		float dv = 1.f / desc.Height;
		level /= 2;

		auto & tl = rect.GetLeftTopCornet();
		tl.x += du*level;
		tl.y += dv*level;

		auto & br = rect.GetRightBottomCornet();
		br.x -= du*level;
		br.y -= dv*level;

		return rect;
	}

	void GetSampleOffset(D3D11_TEXTURE2D_DESC desc,std::array<float4, 32>& offset, int level)
	{
		float du = 1.f / desc.Width;
		float dv = 1.f / desc.Height;

		auto index = 0u;
		for (auto y = 0u; y != level; ++y)
			for (auto x = 0u;x != level/2;++x)
			{
				offset[index].x = (x*2- (level/2-0.5f))*du;
				offset[index].y = (y - (level/2-0.5f))*dv;

				offset[index].z = offset[index].x+du;
				offset[index].w = offset[index].y;
				++index;
			}
	}

	void DrawBegin(ID3D11DeviceContext* context, PostProcess* container,ID3D11ShaderResourceView* src,int level)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC resDesc;
		src->GetDesc(&resDesc);

		if (resDesc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D && resDesc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2DMS)
			throw std::runtime_error("leo::ScalaerProcess<2>::Draw Error: Invalid Argument src(Please Check ViewDimension)");

		auto tex = win::make_scope_com<ID3D11Texture2D>(nullptr);
		auto res = win::make_scope_com<ID3D11Resource>(nullptr);
		src->GetResource(&res);
		dxcall(res->QueryInterface(&tex));
		D3D11_TEXTURE2D_DESC desc;
		tex->GetDesc(&desc);
		auto src_rect = GetTextureRect(desc, level);
		auto& dst_rect = ops::IRect<ops::axis_system::dx_texture_system>();
		container->BindRect(context, src_rect, dst_rect);
		
		std::array<float4, 32> sampleoffset;
		GetSampleOffset(desc, sampleoffset,level);
		SetSampleOffset(sampleoffset);
		Update(context);
	}

private:
	ID3D11SamplerState* mSamplerState = nullptr;
	win::unique_com<ID3D11Buffer> mOffsetsBuffer = nullptr;
	std::array<float4, 32> mOffsets;
};

leo::ScalaerProcess<2>::ScalaerProcess(ID3D11Device * device)
	:PostProcess(device), mImpl(std::make_unique<leo::details::ScalaerProcessDelegate>(device, 2, this))
{
}

leo::ScalaerProcess<2>::~ScalaerProcess()
{
}

void leo::ScalaerProcess<2>::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
	mImpl->Apply(context);
}

void leo::ScalaerProcess<2>::Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst)
{
	mImpl->DrawBegin(context,this, src, 2);
	PostProcess::Draw(context, src, dst);
}

leo::ScalaerProcess<4>::ScalaerProcess(ID3D11Device * device)
	:PostProcess(device), mImpl(std::make_unique<leo::details::ScalaerProcessDelegate>(device, 4, this))
{
}

leo::ScalaerProcess<4>::~ScalaerProcess()
{
}

void leo::ScalaerProcess<4>::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
	mImpl->Apply(context);
}

void leo::ScalaerProcess<4>::Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst)
{
	mImpl->DrawBegin(context, this, src, 4);
	PostProcess::Draw(context, src, dst);
}

leo::ScalaerProcess<8>::ScalaerProcess(ID3D11Device * device)
	:PostProcess(device), mImpl(std::make_unique<leo::details::ScalaerProcessDelegate>(device, 8, this))
{
}

leo::ScalaerProcess<8>::~ScalaerProcess()
{
}

void leo::ScalaerProcess<8>::Apply(ID3D11DeviceContext * context)
{
	PostProcess::Apply(context);
	mImpl->Apply(context);
}

void leo::ScalaerProcess<8>::Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst)
{
	mImpl->DrawBegin(context, this, src, 8);
	PostProcess::Draw(context, src, dst);
}

leo::PostProcessChain::PostProcessChain()
{
}

leo::PostProcessChain::~PostProcessChain()
{
}

void leo::PostProcessChain::Append(const leo::PostProcessPtr & pp)
{
	mProcessPtrChain.push_back(pp);
	mChainSize.reserve(mProcessPtrChain.size());
	mChainInputTemp.reserve(mProcessPtrChain.size());
	mChaninOutputTemp.reserve(mProcessPtrChain.size());
}

leo::uint32 leo::PostProcessChain::NumPostProcesses() const
{
	return  static_cast<uint32>(mProcessPtrChain.size());
}

leo::PostProcessPtr const & leo::PostProcessChain::GetPostProcess(uint32 index) const
{
	return mProcessPtrChain.at(index);
}

void leo::PostProcessChain::OutputPin(ID3D11Device* create,uint32 index, uint32 width, uint32 height, uint32 format)
{
	mChainSize[index].first = width;
	mChainSize[index].second = height;
	if (index == mProcessPtrChain.size() - 1) {
		return;
	}

	auto tex = leo::win::make_scope_com<ID3D11Texture2D>();
	auto srv = leo::win::make_scope_com<ID3D11ShaderResourceView>();
	auto rtv = leo::win::make_scope_com<ID3D11RenderTargetView>();

	CD3D11_TEXTURE2D_DESC desc{static_cast<DXGI_FORMAT>(format),width,height,1,1,D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET };
	leo::dxcall(create->CreateTexture2D(&desc, nullptr, &tex));
	leo::dxcall(create->CreateShaderResourceView(tex, nullptr, &srv));
	leo::dxcall(create->CreateRenderTargetView(tex, nullptr, &rtv));

	mChainInputTemp.at(index).swap(srv);
	mChaninOutputTemp.at(index).swap(rtv);

	auto context = leo::win::make_scope_com<ID3D11DeviceContext>();
	create->GetImmediateContext(&context);
	mProcessPtrChain.at(index + 1)->InputPin(context, width, height,format);
}

void leo::PostProcessChain::InputPin(ID3D11DeviceContext * context, uint32 width, uint32 height, uint32 format)
{
	mProcessPtrChain.at(0)->InputPin(context, width, height, format);
}

void leo::PostProcessChain::Apply(ID3D11DeviceContext * context, ID3D11ShaderResourceView * src, ID3D11RenderTargetView * dst)
{
	mProcessPtrChain.at(0)->Apply(context);
	mProcessPtrChain.at(0)->Draw(context, src, mChaninOutputTemp.at(0));


	for (auto i = 1; i < mProcessPtrChain.size() - 1; ++i) {
		mProcessPtrChain[i]->Apply(context);
		mProcessPtrChain[i]->Draw(context, mChainInputTemp.at(i - 1), mChaninOutputTemp.at(i));
	}

	mProcessPtrChain.at(mProcessPtrChain.size()-1)->Apply(context);
	mProcessPtrChain.at(mProcessPtrChain.size() - 1)->Draw(context,mChainInputTemp.at(mProcessPtrChain.size() - 1),dst);
}
