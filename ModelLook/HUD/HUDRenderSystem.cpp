#include "HUDRenderSystem.h"
#include "DeviceMgr.h"
#include "RenderSystem/d3dx11.hpp"
#include "RenderSystem/d3d11/d3d11texture.hpp"
#include <list>

using namespace leo::HUD;
using namespace leo;

HUDRenderSystem::~HUDRenderSystem()
{
}

class HUDRenderSystemImpl : public HUDRenderSystem
{
public:
	TexturePtr big_tb;
	win::unique_com<ID3D11Buffer> big_vb;
	win::unique_com<ID3D11Buffer> big_ib;

	std::list<Rect> able_rect;
	std::list<Rect> unable_rect;

	std::vector<hud_command> commands;

	ID3D11Device* device = DeviceMgr().GetDevice();
	ID3D11DeviceContext* context = DeviceMgr().GetDeviceContext();

	lconstexpr static unsigned vertex_num = 4 * 1024;
	lconstexpr static unsigned index_num = 12 * 1024;

	HUDRenderSystemImpl() {
		big_tb = X::MakeTexture2D(4096, 4096, 1, 1, EFormat::EF_ABGR8, {}, EAccess::EA_C_W | EAccess::EA_G_R, {});
		
		CD3D11_BUFFER_DESC vbDesc{ vertex_num* sizeof(float2),D3D11_BIND_VERTEX_BUFFER,D3D11_USAGE_DYNAMIC,D3D11_CPU_ACCESS_READ};

		device->CreateBuffer(&vbDesc, nullptr, &big_vb);

		CD3D11_BUFFER_DESC ibDesc{ index_num* sizeof(uint16),D3D11_BIND_INDEX_BUFFER,D3D11_USAGE_DYNAMIC,D3D11_CPU_ACCESS_READ };

	}

	void PushRenderCommand(hud_command  c) {
		commands.emplace_back(c);
	}

	//thread safe
	vb_data LockVB(uint32 num) {

	}
	void UnLockVB(const vb_data&)
	{}
	ib_data LockIB(uint16)
	{}
	void UnLockIB(const ib_data&)
	{}
	Texture::Mapper Map2D(Size)
	{}
	void ExceuteCommand()
	{
		std::sort(commands.begin(),commands.end(),
			[](const hud_command& x,const hud_command& y)
		{
			return x.mat.ptr < y.mat.ptr;
		}
		);

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
			context->DrawIndexed(iter->index_num, iter->index_start, iter->vertex_start);
			++iter;
		}
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
