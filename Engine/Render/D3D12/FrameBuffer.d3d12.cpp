#include "FrameBuffer.h"
#include "RenderView.h"
#include "Context.h"

namespace platform_ex::Windows::D3D12 {
	FrameBuffer::~FrameBuffer() = default;

	void FrameBuffer::SetRenderTargets()
	{
		auto& cmd_list = Context::Instance().GetCommandList(Device::Command_Render);

		std::vector<ID3D12Resource*> rt_src;
		std::vector<uint32> rt_first_subres;
		std::vector<uint32> rt_num_subres;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rt_handles(clr_views.size());

		for (auto i = 0; i != clr_views.size(); ++i) {
			if (clr_views[i]) {
				auto p = static_cast<RenderTargetView*>(clr_views[i].get());
				rt_src.emplace_back(p->Resource());
				rt_first_subres.emplace_back(p->FirstSubResIndex());
				rt_num_subres.emplace_back(p->SubResNum());

				rt_handles[i] = p->View()->GetHandle();
			}
			else
			{
				rt_handles[i].ptr = ~decltype(rt_handles[i].ptr)();
			}
		}

		D3D12_CPU_DESCRIPTOR_HANDLE ds_handle;
		D3D12_CPU_DESCRIPTOR_HANDLE* ds_handle_ptr;
		if (ds_view)
		{
			auto p = static_cast<DepthStencilView*>(ds_view.get());

			ds_handle = p->View()->GetHandle();
			ds_handle_ptr = &ds_handle;
		}
		else
		{
			ds_handle_ptr = nullptr;
		}

		cmd_list->OMSetRenderTargets(static_cast<UINT>(rt_handles.size()),
			rt_handles.empty() ? nullptr : &rt_handles[0], false, ds_handle_ptr);

		
		d3d12_viewport.TopLeftX = static_cast<float>(viewport.left);
		d3d12_viewport.TopLeftY = static_cast<float>(viewport.top);
		d3d12_viewport.Width = static_cast<float>(viewport.width);
		d3d12_viewport.Height = static_cast<float>(viewport.height);

		cmd_list->RSSetViewports(1, &d3d12_viewport);
	}
	void FrameBuffer::BindBarrier()
	{
	}

	void FrameBuffer::UnBindBarrier()
	{
	}

	void FrameBuffer::OnBind() {
		SetRenderTargets();

		for (auto& uav_view : uav_views) {
			static_cast<UnorderedAccessView*>(uav_view.get())->ResetInitCount();
		}
	}
}