#include "HUDRenderSystem.h"
#include "DeviceMgr.h"
#include "Core\Vertex.hpp"
#include "Core\FileSearch.h"
#include "RenderSystem/d3dx11.hpp"
#include "RenderSystem/d3d11/d3d11texture.hpp"
#include "RenderSystem/ShaderMgr.h"
#include "RenderSystem/RenderStates.hpp"
#include <list>

using namespace leo::HUD;
using namespace leo;

HUDRenderSystem::~HUDRenderSystem()
{
}

//beacuse dtor list twice will be crash
std::list<Rect> able_rect;
std::list<Rect> unable_rect;

class HUDRenderSystemImpl : public HUDRenderSystem
{
public:
	TexturePtr big_tb;
	win::unique_com<ID3D11Buffer> big_vb;
	win::unique_com<ID3D11Buffer> big_ib;

	leo::float2 window_inv_size;
	win::unique_com<ID3D11Buffer> vs_params;
	leo::float4 color;
	win::unique_com<ID3D11Buffer> ps_params;


	ID3D11PixelShader* ui_ps;
	ID3D11InputLayout* ui_layout;
	ID3D11VertexShader* ui_vs;
	ID3D11SamplerState* ui_ss;
	ID3D11DepthStencilState* ui_dss;
	ID3D11BlendState* ui_bs;

	uint16 vb_offset = 0;
	uint16 ib_offset = 0;

	std::vector<hud_command> commands;

	ID3D11Device* device = DeviceMgr().GetDevice();
	ID3D11DeviceContext* context = DeviceMgr().GetDeviceContext();

	lconstexpr static unsigned vertex_num = 4 * 1024;
	lconstexpr static unsigned index_num = 12 * 1024;

	HUDRenderSystemImpl() {
		big_tb = X::MakeTexture2D(4096, 4096, 1, 1, EFormat::EF_ABGR8, {}, EAccess::EA_C_W | EAccess::EA_G_R, {});
		
		CD3D11_BUFFER_DESC vbDesc{ vertex_num* sizeof(float4),D3D11_BIND_VERTEX_BUFFER,D3D11_USAGE_DYNAMIC,D3D11_CPU_ACCESS_WRITE};
		device->CreateBuffer(&vbDesc, nullptr, &big_vb);

		CD3D11_BUFFER_DESC ibDesc{ index_num* sizeof(uint16),D3D11_BIND_INDEX_BUFFER,D3D11_USAGE_DYNAMIC,D3D11_CPU_ACCESS_WRITE };
		device->CreateBuffer(&ibDesc, nullptr, &big_ib);

		dx::CreateGPUCBuffer<leo::float2>(device, vs_params);
		dx::CreateGPUCBuffer<leo::float4>(device, ps_params);

		ui_vs = ShaderMgr().CreateVertexShader(FileSearch::Search(L"UIVS.cso"), nullptr,InputLayoutDesc::Sky, 1, &ui_layout);
		ui_ps = ShaderMgr().CreatePixelShader(FileSearch::Search(L"UIPS.cso"));

		CD3D11_SAMPLER_DESC ui_SamplerDesc{ D3D11_DEFAULT };
		ui_SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		ui_ss = leo::RenderStates().CreateSamplerState(L"bilinear_sampler", ui_SamplerDesc);

		CD3D11_DEPTH_STENCIL_DESC ui_DepthStencilDesc(D3D11_DEFAULT);
		ui_DepthStencilDesc.DepthEnable = false;
		ui_DepthStencilDesc.StencilEnable = false;
		ui_dss = leo::RenderStates().CreateDepthStencilState(L"ui_no_depth_stencil", ui_DepthStencilDesc);

		CD3D11_BLEND_DESC ui_BlendDesc(D3D11_DEFAULT);
		ui_BlendDesc.AlphaToCoverageEnable = false;
		ui_BlendDesc.IndependentBlendEnable = false;
		ui_BlendDesc.RenderTarget[0].BlendEnable = true;
		ui_BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		ui_BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		ui_BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		ui_BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		ui_BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		ui_BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		ui_BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		ui_bs = leo::RenderStates().CreateBlendState(L"ui_transparent", ui_BlendDesc);
		
	}

	void PushRenderCommand(hud_command  c) override{
		commands.emplace_back(c);
	}

	vb_data LockVB(uint16 num) override {
		D3D11_MAPPED_SUBRESOURCE subRes;
		dxcall(context->Map(big_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes));

		vb_offset += num;

		return{ static_cast<uint16>(vb_offset - num),num,reinterpret_cast<float4*>(subRes.pData) };
	}
	void UnLockVB(const vb_data&) override
	{
		context->Unmap(big_vb, 0);
	}
	ib_data LockIB(uint16 num) override
	{
		D3D11_MAPPED_SUBRESOURCE subRes;
		dxcall(context->Map(big_ib, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes));

		ib_offset += num;

		return{ static_cast<uint16>(ib_offset - num),num,reinterpret_cast<uint16*>(subRes.pData) };
	}
	void UnLockIB(const ib_data&) override
	{
		context->Unmap(big_ib, 0);
	}
	std::unique_ptr<Texture::Mapper> Map2D(Size s) override
	{
		return std::make_unique<Texture::Mapper>(*big_tb, 0, 0, Texture::MapAccess::MA_WO, 0, 0,static_cast<uint16>(s.GetWidth()), static_cast<uint16>(s.GetHeight()));
	}
	void ExceuteCommand(std::pair<uint16, uint16> window_size) override
	{
		std::sort(commands.begin(),commands.end(),
			[](const hud_command& x,const hud_command& y)
		{
			return x.mat.ptr < y.mat.ptr;
		}
		);

		window_inv_size.x = 1.f / window_size.first;
		window_inv_size.y = 1.f / window_size.second;
		context->UpdateSubresource(vs_params, 0, nullptr, &window_inv_size, 0, 0);

		UINT strides[] = { sizeof(float4) };
		UINT offsets[] = { 0 };

		context->IASetIndexBuffer(big_ib, DXGI_FORMAT_R16_UINT, 0);
		context->IASetVertexBuffers(0, 1, &big_vb, strides, offsets);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->PSSetShader(ui_ps, nullptr, 0);
		context->VSSetShader(ui_vs, nullptr, 0);
		context->VSSetConstantBuffers(0, 1, &vs_params);
		context->PSSetConstantBuffers(0, 1, &ps_params);

		const float blend_factor[] = { 1,1,1,1 };
		context->OMSetBlendState(ui_bs, blend_factor, 0xffffffff);
		context->OMSetDepthStencilState(ui_dss, 0xff);
		context->PSSetSamplers(0, 1, &ui_ss);

		auto iter = commands.begin();
		auto prev_mat_ptr = iter->mat.ptr;
		auto srv =dynamic_cast<D3D11Texture2D*>(prev_mat_ptr.get())->ResouceView();
		context->PSSetShaderResources(0, 1, &srv);
		while (iter != commands.end())
		{
			if (iter->mat.ptr != prev_mat_ptr) {
				prev_mat_ptr = iter->mat.ptr;
				auto srv = dynamic_cast<D3D11Texture2D*>(prev_mat_ptr.get())->ResouceView();
				context->PSSetShaderResources(0, 1, &srv);
			}
			color.x = iter->mat.color.GetR() / 255.f;
			color.y = iter->mat.color.GetG() / 255.f;
			color.z = iter->mat.color.GetB() / 255.f;
			color.w = iter->mat.color.GetA() / 255.f;

			context->UpdateSubresource(ps_params, 0, nullptr, &color, 0, 0);
			context->DrawIndexed(iter->index_num/2, iter->index_start, iter->vertex_start);
			++iter;
		}

		//状态clear
		ib_offset = 0;
		vb_offset = 0;
		commands.clear();
		//需要对tb有一定的cache策略[内部实现]
	}
};


HUDRenderSystem & HUDRenderSystem::GetInstance()
{
	static HUDRenderSystemImpl mInstance;
	return mInstance;
}

HUDRenderSystem::hud_mat::hud_mat()
	:color(Drawing::ColorSpace::White), ptr(dynamic_cast<HUDRenderSystemImpl&>(HUDRenderSystem::GetInstance()).big_tb)
{
}

HUDRenderSystem::hud_mat::hud_mat(TexturePtr p)
	: color(Drawing::ColorSpace::White), ptr(p)
{
}
