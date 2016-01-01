#include "HUDPseudoWindow.h"
#include "HUD\HUDRenderSystem.h"
#include "UI\UIImpl.h"
#include <ref.hpp>
LEO_BEGIN

#pragma warning(disable:4244)

struct HUDPseudoWindow::HUDPseudoWindowImpl {

public:
	lref<HUD::HostRenderer> ref;

	size_type window_size;
	size_type window_pos;

	HUDPseudoWindowImpl(HUD::HostRenderer & host)
		:ref(host)
	{}

	~HUDPseudoWindowImpl(){}

	void Resize(const size_type& size) {
		window_size = size;
	}

	void RePos(const size_type& pos) {
		window_pos = pos;
	}

	void Update(float t) {

	}

	void Render(size_type host_size) {
		auto& rs = HUD::HUDRenderSystem::GetInstance();

		auto vbdata = rs.LockVB(4);

		vbdata.vertex[vbdata.vertex_start] =	leo::float4(
				0.f+window_pos.first,
				0.f+window_pos.second,
				0.f,0.f);
		vbdata.vertex[vbdata.vertex_start+2] =	leo::float4(
			window_size.first+ window_pos.first,
			window_size.second + window_pos.second,
			1.f,1.f);
		vbdata.vertex[vbdata.vertex_start+1] =	leo::float4(
			window_size.first+ window_pos.first, 
			0.f + window_pos.second, 
			1.f, 0.f);
		vbdata.vertex[vbdata.vertex_start +3] = leo::float4(
			0.f+ window_pos.first, 
			window_size.second + window_pos.second,
			0.f, 1.f);

		auto ibdata = rs.LockIB(6);

		rs.FillQuadIBByVB(vbdata, ibdata);

		rs.UnLockIB(ibdata);
		rs.UnLockVB(vbdata);

		rs.PushRenderCommand(
			rs.MakeCommand(
				vbdata,ibdata, 
				(dynamic_cast<Drawing::details::hud_tex_wrapper&>(ref.get().GetImageBuffer())).tex)
			);

		//this function code can push into excetue
		rs.ExceuteCommand(host_size);
	}
};

HUDPseudoWindow::HUDPseudoWindow(HUD::HostRenderer & host)
	:pImpl(new HUDPseudoWindowImpl(host))
{
}

void HUDPseudoWindow::Resize(const size_type & size)
{
	pImpl->Resize(size);
}

void HUDPseudoWindow::RePos(const size_type & pos)
{
	pImpl->RePos(pos);
}

void HUDPseudoWindow::Update(float t)
{
	pImpl->Update(t);
}

void HUDPseudoWindow::Render(size_type host_size)
{
	pImpl->Render(host_size);
}

bool HUDPseudoWindow::IsMined()
{
	return false;
}

//because impl
HUDPseudoWindow::~HUDPseudoWindow()
{
}

LEO_END
