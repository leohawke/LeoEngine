#include <LBase/pointer.hpp>
#include "Context.h"
#include "Convert.h"
#include "Display.h"
#include "FrameBuffer.h"
#include "Texture.h"

#define TEST_CODE 1
#if TEST_CODE
extern HWND g_hwnd;
#endif

namespace platform_ex::Windows::D3D12 {
	Context::Context()
		:adapter_list()
	{
#ifndef NDEBUG
		{
			COMPtr<ID3D12Debug> debug_ctrl;
			if (SUCCEEDED(D3D12::GetDebugInterface(COMPtr_RefParam(debug_ctrl, IID_ID3D12Debug)))) {
				LAssertNonnull(debug_ctrl);
				debug_ctrl->EnableDebugLayer();
			}
		}
#endif
	}

	DXGI::Adapter & Context::DefaultAdapter()
	{
		return adapter_list.CurrentAdapter();
	}

	void D3D12::Context::SyncCPUGPU(bool force)
	{
		CommitCommandList(Device::Command_Render);
		SyncCommand(Device::Command_Render);

		ResetCommand(Device::Command_Render);

		ClearPSOCache();
	}

	const COMPtr<ID3D12GraphicsCommandList> & D3D12::Context::GetCommandList(Device::CommandType index) const
	{
		return d3d_cmd_lists[index];
	}

	std::mutex & D3D12::Context::GetCommandListMutex(Device::CommandType index)
	{
		return cmd_list_mutexs[index];
	}

	void D3D12::Context::SyncCommand(Device::CommandType type)
	{
		auto val = fences[type]->Signal((Fence::Type)type);
		fences[type]->Wait(val);
	}

	void D3D12::Context::ResetCommand(Device::CommandType type)
	{
		CheckHResult(GetDevice().d3d_cmd_allocators[type]->Reset());
		CheckHResult(d3d_cmd_lists[type]->Reset(GetDevice().d3d_cmd_allocators[type].Get(), nullptr));
	}

	const COMPtr<ID3D12CommandQueue>& D3D12::Context::GetCommandQueue(Device::CommandType type) const
	{
		return d3d_cmd_queues[type];
	}

	void D3D12::Context::CommitCommandList(Device::CommandType type)
	{
		CheckHResult(d3d_cmd_lists[type]->Close());
		ID3D12CommandList* cmd_lists[] = { d3d_cmd_lists[type].Get() };
		device->d3d_cmd_queue->ExecuteCommandLists(1, cmd_lists);

		if (type == Device::CommandType::Command_Resource) {
			auto val = fences[type]->Signal(Fence::Render);
			fences[type]->Wait(val);

			ResetCommand(type);
		}
	}

	void D3D12::Context::Push(const platform::Render::PipleState & ps)
	{
		d3d_cmd_lists[Device::Command_Render]->OMSetStencilRef(ps.DepthStencilState.front_stencil_ref);
		d3d_cmd_lists[Device::Command_Render]->OMSetBlendFactor(ps.BlendState.blend_factor.begin());
	}

	void D3D12::Context::ClearPSOCache()
	{
	}

	void D3D12::Context::UpdateRenderPSO(const Effect::Effect & effect, const Effect::Technique & tech, const Effect::Pass & pass, const platform::Render::InputLayout & layout)
	{
		auto& shader_compose = static_cast<ShaderCompose&>(pass.GetShader(effect));
		auto& piple_state = static_cast<const PipleState&>(pass.GetState());

		auto& render_cmd_list = d3d_cmd_lists[Device::Command_Render];

		auto pso = piple_state.RetrieveGraphicsPSO(layout, shader_compose, GetCurrFrame(), tech.HasTessellation());

		render_cmd_list->SetPipelineState(pso.Get());
		render_cmd_list->SetGraphicsRootSignature(shader_compose.RootSignature());

		if (pass.GetState().RasterizerState.scissor_enable) {
			//TODO  RSSetScissorRects
		}
		else {
			D3D12_RECT rc =
			{
				//TODO Viewport
			};
			//d3d_cmd_lists[Device::Command_Render]->RSSetScissorRects(1,&rc);
		}

		std::size_t num_handle = 0;
		for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
			num_handle += shader_compose.Srvs[i].size() + shader_compose.Uavs[i].size();
		}

		ID3D12DescriptorHeap* heaps[2];
		uint32 num_heaps = 0;
		COMPtr<ID3D12DescriptorHeap> cbv_srv_uav_heap;
		auto sampler_heap = shader_compose.SamplerHeap();
		if (num_handle > 0) {
			//hash cache
			D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_heap_desc;
			cbv_srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			cbv_srv_heap_desc.NumDescriptors = static_cast<UINT>(num_handle);
			cbv_srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbv_srv_heap_desc.NodeMask = 0;
			CheckHResult((*device)->CreateDescriptorHeap(&cbv_srv_heap_desc, COMPtr_RefParam(cbv_srv_uav_heap, IID_ID3D12DescriptorHeap)));
			heaps[num_heaps++] = cbv_srv_uav_heap.Get();
		}
		if (sampler_heap)
			heaps[num_heaps++] = sampler_heap;

		if (num_heaps > 0)
			render_cmd_list->SetDescriptorHeaps(num_heaps, heaps);

		uint32 root_param_index = 0;
		//CBuffer Bind
		for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
			for (auto & cbuffer : shader_compose.CBuffs[i]) {
				auto& resource = static_cast<GraphicsBuffer*>(cbuffer)->buffer;
				if (resource)
					render_cmd_list->SetGraphicsRootConstantBufferView(root_param_index++, resource->GetGPUVirtualAddress());
				else
					render_cmd_list->SetGraphicsRootConstantBufferView(root_param_index, 0);
			}
		}

		//SRV/UAV  Bind
		if (cbv_srv_uav_heap) {
			auto cbv_srv_uav_desc_size = (*device)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			auto cpu_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
			auto gpu_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();
			for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
				if (!shader_compose.Srvs[i].empty()) {
					render_cmd_list->SetGraphicsRootDescriptorTable(root_param_index, gpu_handle);
					for (auto& srv : shader_compose.Srvs[i]) {
						//TODO null_srv_handle
						(*device)->CopyDescriptorsSimple(1, cpu_handle, srv->GetHandle(),
							D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						cpu_handle.ptr += cbv_srv_uav_desc_size;
						gpu_handle.ptr += cbv_srv_uav_desc_size;
					}
					++root_param_index;
				}
			}

			for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
				if (!shader_compose.Uavs[i].empty()) {
					render_cmd_list->SetGraphicsRootDescriptorTable(root_param_index, gpu_handle);
					for (auto& uav : shader_compose.Uavs[i]) {
						//TODO null_srv_handle
						(*device)->CopyDescriptorsSimple(1, cpu_handle, uav->GetHandle(),
							D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						cpu_handle.ptr += cbv_srv_uav_desc_size;
						gpu_handle.ptr += cbv_srv_uav_desc_size;
					}
					++root_param_index;
				}
			}
		}

		//Sampler Bind
		if (sampler_heap) {
			auto sampler_desc_size = (*device)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

			auto gpu_sampler_handle = sampler_heap->GetGPUDescriptorHandleForHeapStart();

			for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
				if (!shader_compose.Srvs[i].empty()) {
					render_cmd_list->SetGraphicsRootDescriptorTable(root_param_index, gpu_sampler_handle);
					gpu_sampler_handle.ptr += sampler_desc_size *  shader_compose.Samplers[i].size();

					++root_param_index;
				}
			}
		}

	}

	void Context::ContextEx(ID3D12Device * d3d_device, ID3D12CommandQueue * cmd_queue)
	{
		d3d_cmd_queues[Device::Command_Render] = cmd_queue;
		d3d_cmd_queues[Device::Command_Render]->AddRef();
		CheckHResult(d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
			device->d3d_cmd_allocators[Device::Command_Render].Get(), nullptr,
			COMPtr_RefParam(d3d_cmd_lists[Device::Command_Render], IID_ID3D12GraphicsCommandList)));
		D3D::Debug(d3d_cmd_lists[Device::Command_Render], "Render_Command");

		CheckHResult(d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
			device->d3d_cmd_allocators[Device::Command_Resource].Get(), nullptr,
			COMPtr_RefParam(d3d_cmd_lists[Device::Command_Resource], IID_ID3D12GraphicsCommandList)));
		D3D::Debug(d3d_cmd_lists[Device::Command_Resource], "Resource_Command");

		for (auto& fence : fences)
			fence.swap(std::make_unique<Fence>());
	}

	void Context::CreateDeviceAndDisplay() {
		device = leo::make_shared<Device>(DefaultAdapter());
		ContextEx(device->d3d_device.Get(), device->d3d_cmd_queue.Get());
		DisplaySetting setting;
		display = leo::make_shared<Display>(GetDXGIFactory4(), device->d3d_cmd_queue.Get(), setting, g_hwnd);//test code
	}
	void Context::DoBindFrameBuffer(const std::shared_ptr<platform::Render::FrameBuffer>&)
	{
	}
	void Context::Render(const Effect::Effect & effect, const Effect::Technique & tech, const platform::Render::InputLayout & layout)
	{
		//TODO Compute/Copy State -> SyncCPUGPU(true)

		auto& framebuffer = static_pointer_cast<FrameBuffer>(GetCurrFrame());
		framebuffer->SetRenderTargets();
		framebuffer->BindBarrier();

		std::vector<D3D12_RESOURCE_BARRIER> barriers;

		//TODO StreamOuput

		//Vertex Stream Barrier
		auto num_vertex_streams = layout.GetVertexStreamsSize();
		for (auto i = 0; i != num_vertex_streams; ++i) {
			auto& stream = layout.GetVertexStream(i);
			auto& vb = static_cast<GraphicsBuffer&>(*stream.stream);
			if (!(vb.GetAccess() & (EAccessHint::EA_CPURead | EAccessHint::EA_CPUWrite))) {
				D3D12_RESOURCE_BARRIER barrier;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.Subresource = 0;
				if (vb.UpdateResourceBarrier(barrier, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)) {
					barriers.emplace_back(barrier);
				}
			}
		}

		//TODO Instance Stream Barrier

		//optional Index Stream
		if (layout.GetIndexStream()) {
			auto& ib = static_cast<GraphicsBuffer&>(*layout.GetIndexStream());
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.Subresource = 0;
			if (ib.UpdateResourceBarrier(barrier, D3D12_RESOURCE_STATE_INDEX_BUFFER)) {
				barriers.push_back(barrier);
			}
		}

		if (!barriers.empty())
			d3d_cmd_lists[Device::Command_Render]->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());

		static_cast<const InputLayout&>(layout).Active();

		auto vertex_count = layout.GetIndexStream() ? layout.GetNumIndices() : layout.GetNumVertices();


		auto tt = layout.GetTopoType();
		//TODO Tessllation

		//State Cache?

		d3d_cmd_lists[Device::Command_Render]->IASetPrimitiveTopology(Convert<D3D12_PRIMITIVE_TOPOLOGY>(tt));

		auto prim_count = vertex_count;
		switch (tt)
		{
		case platform::Render::InputLayout::LineList:
			prim_count /= 2;
			break;
		case platform::Render::InputLayout::LineStrip:
			--prim_count;
			break;
		case platform::Render::InputLayout::TriangleList:
			prim_count /= 2;
			break;
		case platform::Render::InputLayout::TriangleStrip:
			prim_count -= 2;
			break;
		}

		//TODO Instance
		auto num_instances = layout.GetVertexStream(0).instance_freq;

		//Statistics Render Infomation

		auto num_passes = tech.NumPasses();

		//TODO Indirect Args
		if (layout.GetIndexStream()) {
			auto num_indices = layout.GetNumIndices();
			for (auto i = 0; i != num_passes; ++i) {
				auto& pass = tech.GetPass(i);
				pass.Bind(effect);

				UpdateRenderPSO(effect, tech, pass, layout);
				d3d_cmd_lists[Device::Command_Render]->DrawIndexedInstanced(num_indices,
					num_instances,
					layout.GetIndexStart(), layout.GetVertexStart(), 0);
				pass.UnBind(effect);
			}
		}
		else {
			auto num_vertices = layout.GetNumVertices();
			for (auto i = 0; i != num_passes; ++i) {
				auto& pass = tech.GetPass(i);
				pass.Bind(effect);

				UpdateRenderPSO(effect, tech, pass, layout);
				d3d_cmd_lists[Device::Command_Render]->DrawInstanced(num_vertices,
					num_instances,
					layout.GetVertexStart(), 0);
				pass.UnBind(effect);
			}
		}

		//Statistics Render Infomation

		framebuffer->UnBindBarrier();

	}
	Context & Context::Instance()
	{
		static Context context;
		return context;
	}
}

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {
			bool Support() {
				return true;
			}
			platform::Render::Context& GetContext() {
				return Context::Instance();
			}
		}
	}
}