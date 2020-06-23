#include "FrameBuffer.h"
#include "Context.h"

namespace platform_ex::Windows::D3D12 {
	FrameBuffer::FrameBuffer()
		:d3d12_viewport({ 0,0,0,0,0,1 })
	{
	}
	FrameBuffer::~FrameBuffer() = default;

	void FrameBuffer::SetRenderTargets()
	{
		auto& cmd_list = Context::Instance().GetCommandList(Device::Command_Render);

		std::vector<ID3D12Resource*> rt_src;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rt_handles(clr_views.size());

		for (auto i = 0; i != clr_views.size(); ++i) {
			if (clr_views[i].Texture) {
				auto pD3DTexture = dynamic_cast<Texture*>(clr_views[i].Texture);
				auto pRTV = pD3DTexture->GetRenderTargetView(0, -1);
				
				rt_handles[i] = pRTV->GetView();
			}
			else
			{
				rt_handles[i].ptr = ~decltype(rt_handles[i].ptr)();
			}
		}

		D3D12_CPU_DESCRIPTOR_HANDLE ds_handle;
		D3D12_CPU_DESCRIPTOR_HANDLE* ds_handle_ptr;
		if (ds_view.Texture)
		{
			auto pD3DTexture = dynamic_cast<Texture*>(ds_view.Texture);
			auto pDSV = pD3DTexture->GetDepthStencilView({});

			ds_handle = pDSV->GetView();
			ds_handle_ptr = &ds_handle;
		}
		else
		{
			ds_handle_ptr = nullptr;
		}

		cmd_list->OMSetRenderTargets(static_cast<UINT>(rt_handles.size()),
			rt_handles.empty() ? nullptr : &rt_handles[0], false, ds_handle_ptr);


		d3d12_viewport.TopLeftX = static_cast<float>(viewport.x);
		d3d12_viewport.TopLeftY = static_cast<float>(viewport.y);
		d3d12_viewport.Width = static_cast<float>(viewport.width);
		d3d12_viewport.Height = static_cast<float>(viewport.height);

		//×´Ì¬×ªÒÆ
		Context::Instance().RSSetViewports(1, &d3d12_viewport);
	}
	void FrameBuffer::BindBarrier()
	{
		auto& cmd_list = Context::Instance().GetCommandList(Device::Command_Render);

		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		for (auto i = 0; i != clr_views.size(); ++i) {
			if (clr_views[i].Texture) {
				D3D12_RESOURCE_BARRIER barrier;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				auto p = dynamic_cast<Texture*>(clr_views[i].Texture);
				if (p->UpdateResourceBarrier(barrier, D3D12_RESOURCE_STATE_RENDER_TARGET))
					barriers.push_back(barrier);
			}
		}

		if (ds_view.Texture) {
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			auto p = dynamic_cast<Texture*>(ds_view.Texture);
			if (p->UpdateResourceBarrier(barrier, D3D12_RESOURCE_STATE_DEPTH_WRITE))
				barriers.push_back(barrier);
		}
		
		if (!barriers.empty())
		{
			cmd_list->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
		}
	}

	void FrameBuffer::OnBind() {
		SetRenderTargets();

		for (auto& uav_view : uav_views) {
			//static_cast<UnorderedAccessView*>(uav_view.get())->ResetInitCount();
		}
	}

	void FrameBuffer::OnUnBind()
	{
		auto& cmd_list = Context::Instance().GetCommandList(Device::Command_Render);

		cmd_list->OMSetRenderTargets(0,
			nullptr , false, nullptr);
	}

	void FrameBuffer::Clear(leo::uint32 flags, const leo::math::float4 & clr, float depth, leo::int32 stencil)
	{
		if (flags & Color) {
			for (auto i = 0; i != clr_views.size(); ++i) {
				if (clr_views[i].Texture) {
					;//static_cast<RenderTargetView*>(clr_views[i].get())->ClearColor(clr);
				}
			}
		}
		if ((flags & Depth) && (flags & Stencil)) {
			if (ds_view.Texture)
				;//static_cast<DepthStencilView*>(ds_view.get())->ClearDepthStencil(depth, stencil);
		}
		else {
			if (flags & Depth)
				;//static_cast<DepthStencilView*>(ds_view.get())->ClearDepth(depth);
			if (flags & Stencil)
				;//static_cast<DepthStencilView*>(ds_view.get())->ClearStencil(stencil);
		}
	}

	DepthStencilView* D3D12::FrameBuffer::GetDepthStencilView() const
	{
		if (ds_view.Texture)
			return dynamic_cast<Texture*>(ds_view.Texture)->GetDepthStencilView({});
		return nullptr;
	}
}
