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
		auto pDSV = GetDepthStencilView();
		if (pDSV)
		{
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
		auto& cmd_list = Context::Instance().GetCommandList(Device::Command_Render);

		uint32 ClearRectCount = 0;
		D3D12_RECT* pClearRects = nullptr;
		D3D12_RECT ClearRects[4];

		if (flags & Color) {
			for (auto i = 0; i != clr_views.size(); ++i) {
				if (clr_views[i].Texture) {
					auto RTTexture = dynamic_cast<Texture*>(clr_views[i].Texture);

					int32 RTMipIndex = clr_views[i].MipIndex;
					int32 RTSliceIndex = clr_views[i].ArraySlice;

					auto RenderTargetView = RTTexture->GetRenderTargetView(RTMipIndex, RTSliceIndex);
					
					cmd_list->ClearRenderTargetView(RenderTargetView->GetView(), reinterpret_cast<const float*>(&clr), ClearRectCount, pClearRects);
				}
			}
		}

		DepthStencilView* DSView = GetDepthStencilView();

		const bool bClearDepth = (flags & Depth) == Depth;
		const bool bClearStencil = (flags & Stencil) == Stencil;

		const bool ClearDSV = (bClearDepth || bClearStencil) && DSView;

		uint32 ClearFlags = 0;

		if (ClearDSV)
		{
			if (bClearDepth && DSView->HasDepth())
			{
				ClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
			}

			if (bClearStencil && DSView->HasStencil())
			{
				ClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
			}

			cmd_list->ClearDepthStencilView(DSView->GetView(), (D3D12_CLEAR_FLAGS)ClearFlags, depth, stencil, ClearRectCount, pClearRects);
		}
	}

	DepthStencilView* D3D12::FrameBuffer::GetDepthStencilView() const
	{
		if (ds_view.Texture)
			return dynamic_cast<Texture*>(ds_view.Texture)->GetDepthStencilView({});
		return nullptr;
	}
	platform::Render::Texture* FrameBuffer::Attached(FrameBuffer::Attachment which) const
	{
		if (which == DepthStencil)
			return ds_view.Texture;
		if(which < clr_views.size())
			return clr_views[which].Texture;
		return nullptr;
	}
}
