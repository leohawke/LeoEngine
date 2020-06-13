#include <LBase/pointer.hpp>
#include "Context.h"
#include "Convert.h"
#include "Display.h"
#include "FrameBuffer.h"
#include "Texture.h"
#include "RayContext.h"
#include "RootSignature.h"
#include "../Effect/CopyEffect.h"
#include <LFramework/Core/LException.h>

#define TEST_CODE 1
#if TEST_CODE
extern HWND g_hwnd;
#endif

namespace platform_ex::Windows::D3D12 {
	Device& GetDevice()
	{
		return Context::Instance().GetDevice();
	}

	Context::Context()
		:adapter_list(), render_command_context(nullptr)
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
		ResetCommand(Device::Command_Render);

		SyncCommand(Device::Command_Render);

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
		auto val = GetFence(type).Signal((Fence::Type)type);
		GetFence(type).Wait(val);
	}

	void D3D12::Context::ResetCommand(Device::CommandType type)
	{
		CheckHResult(d3d_cmd_lists[type]->Reset(GetDevice().d3d_cmd_allocators[type].Get(), nullptr));
	}

	void Context::ExecuteUAVBarrier()
	{
		D3D12_RESOURCE_BARRIER Barrier = {};
		Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		Barrier.UAV.pResource = nullptr;
		d3d_cmd_lists[Device::Command_Render]->ResourceBarrier(1, &Barrier);
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
			auto val = GetFence(type).Signal(Fence::Render);
			GetFence(type).Wait(val);

			GetDevice().d3d_cmd_allocators[type]->Reset();
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
		device->curr_render_cmd_allocator->cbv_srv_uav_heap_cache.clear();

		for (auto const & item : device->curr_render_cmd_allocator->recycle_after_sync_upload_buffs)
			device->upload_resources.emplace(item.second, item.first);
		device->curr_render_cmd_allocator->recycle_after_sync_upload_buffs.clear();
		for (auto const & item : device->curr_render_cmd_allocator->recycle_after_sync_readback_buffs)
			device->readback_resources.emplace(item.second, item.first);
		device->curr_render_cmd_allocator->recycle_after_sync_readback_buffs.clear();

		device->curr_render_cmd_allocator->recycle_after_sync_residency_buffs.clear();
	}

	
	void D3D12::Context::UpdateRenderPSO(const Effect::Effect & effect, const Effect::Technique & tech, const Effect::Pass & pass, const platform::Render::InputLayout & layout)
	{
		auto& shader_compose = static_cast<ShaderCompose&>(pass.GetShader(effect));
		auto& piple_state = static_cast<const PipleState&>(pass.GetState());

		auto& render_cmd_list = d3d_cmd_lists[Device::Command_Render];

		auto pso = piple_state.RetrieveGraphicsPSO(layout, shader_compose, GetCurrFrame(), tech.HasTessellation());

		render_cmd_list->SetPipelineState(pso.get());
		render_cmd_list->SetGraphicsRootSignature(shader_compose.RootSignature()->Signature.Get());

		D3D12_RECT scissor_rc;
		if (pass.GetState().RasterizerState.scissor_enable) {
			throw leo::unsupported();
		}
		else {
			scissor_rc  =
			{
				static_cast<LONG>(curr_viewport.TopLeftX),
				static_cast<LONG>(curr_viewport.TopLeftY),
				static_cast<LONG>(curr_viewport.TopLeftX + curr_viewport.Width),
				static_cast<LONG>(curr_viewport.TopLeftY + curr_viewport.Height)
			};
		}
		render_cmd_list->RSSetScissorRects(1,&scissor_rc);
		UpdateCbvSrvUavSamplerHeaps(shader_compose);
	}

	void Context::UpdateCbvSrvUavSamplerHeaps(const ShaderCompose & shader_compose)
	{
		auto& render_cmd_list = d3d_cmd_lists[Device::Command_Render];
		std::size_t num_handle = 0;
		for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
			num_handle += shader_compose.Srvs[i].size() + shader_compose.Uavs[i].size();
		}

		ID3D12DescriptorHeap* heaps[2];
		uint32 num_heaps = 0;
		leo::observer_ptr<ID3D12DescriptorHeap> cbv_srv_uav_heap;
		auto sampler_heap = shader_compose.SamplerHeap();
		if (num_handle > 0) {
			size_t hash_val = 0;
			for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
				hash_val = hash_combine_seq(hash_val, i, shader_compose.Srvs[i].size());
				hash_val = hash(hash_val, shader_compose.Srvs[i].begin(), shader_compose.Srvs[i].end());
			}
			for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
				hash_val = hash_combine_seq(hash_val, i, shader_compose.Uavs[i].size());
				hash_val = hash(hash_val, shader_compose.Uavs[i].begin(), shader_compose.Uavs[i].end());
			}
			auto iter = device->cbv_srv_uav_heaps.find(hash_val);
			if (iter == device->cbv_srv_uav_heaps.end()) {
				cbv_srv_uav_heap = device->CreateDynamicCBVSRVUAVDescriptorHeap(static_cast<UINT>(num_handle));
				device->cbv_srv_uav_heaps.emplace(hash_val, *cbv_srv_uav_heap.get());
			}
			else {
				cbv_srv_uav_heap = leo::make_observer(iter->second.Get());
			}
			heaps[num_heaps++] = cbv_srv_uav_heap.get();
		}
		if (sampler_heap)
			heaps[num_heaps++] = sampler_heap;

		if (num_heaps > 0)
			render_cmd_list->SetDescriptorHeaps(num_heaps, heaps);

		auto signature = shader_compose.RootSignature();
		
		//SRV/UAV  Bind
		if (cbv_srv_uav_heap) {
			auto cbv_srv_uav_desc_size = (*device)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			auto cpu_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
			auto gpu_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();
			for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
				if (!shader_compose.Srvs[i].empty()) {
					render_cmd_list->SetGraphicsRootDescriptorTable(signature->SRVRDTBindSlot((ShaderType)i), gpu_handle);
					for (auto& srv : shader_compose.Srvs[i]) {
						 (*device)->CopyDescriptorsSimple(1, cpu_handle,srv!=nullptr? srv->GetHandle(): device->null_srv_handle,
								D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						cpu_handle.ptr += cbv_srv_uav_desc_size;
						gpu_handle.ptr += cbv_srv_uav_desc_size;
					}
				}
			}

			for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
				if (!shader_compose.Uavs[i].empty()) {
					render_cmd_list->SetGraphicsRootDescriptorTable(signature->UAVRDTBindSlot((ShaderType)i), gpu_handle);
					for (auto& uav : shader_compose.Uavs[i]) {
						(*device)->CopyDescriptorsSimple(1, cpu_handle,uav!=nullptr? uav->GetHandle():device->null_uav_handle,
							D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						cpu_handle.ptr += cbv_srv_uav_desc_size;
						gpu_handle.ptr += cbv_srv_uav_desc_size;
					}
				}
			}
		}

		//Sampler Bind
		if (sampler_heap) {
			auto sampler_desc_size = (*device)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

			auto gpu_sampler_handle = sampler_heap->GetGPUDescriptorHandleForHeapStart();

			for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
				if (!shader_compose.Srvs[i].empty()) {
					render_cmd_list->SetGraphicsRootDescriptorTable(signature->SamplerRDTBindSlot((ShaderType)i), gpu_sampler_handle);
					gpu_sampler_handle.ptr += sampler_desc_size * shader_compose.Samplers[i].size();
				}
			}
		}

		//CBuffer Bind
		for (auto i = 0; i != ShaderCompose::NumTypes; ++i) {
			int BufferIndex = 0;
			for (auto& cbuffer : shader_compose.CBuffs[i]) {
				auto& resource = static_cast<GraphicsBuffer*>(cbuffer)->resource;
				auto root_param_index = signature->CBVRDBindSlot((ShaderType)i, BufferIndex++);
				if (resource)
					render_cmd_list->SetGraphicsRootConstantBufferView(root_param_index, resource->GetGPUVirtualAddress());
				else
					render_cmd_list->SetGraphicsRootConstantBufferView(root_param_index, 0);
			}
		}
	}

	void Context::RSSetViewports(UINT NumViewports, D3D12_VIEWPORT const * pViewports)
	{
		if (NumViewports == 1)
		{
			//if (memcmp(&curr_viewport, pViewports, sizeof(pViewports[0])) != 0)
			{
				d3d_cmd_lists[Device::Command_Render]->RSSetViewports(NumViewports, pViewports);
				curr_viewport = pViewports[0];
			}
		}
		else
		{
			d3d_cmd_lists[Device::Command_Render]->RSSetViewports(NumViewports, pViewports);
			curr_viewport = pViewports[0];
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

		for (auto& fence :device->fences)
			fence.swap(std::make_unique<Fence>());

		FilterExceptions([&, this] {
			ray_context = std::make_shared<RayContext>(device.get(), this);
			},"ERROR: DirectX Raytracing is not supported by your OS, GPU and/or driver.");

		SubAllocatedOnlineHeap::SubAllocationDesc desc;
		render_command_context = new CommandContext(device.get(), desc, true);
	}

	COMPtr<ID3D12Resource> Context::InnerResourceAlloc(InnerReourceType type, leo::uint32 size_in_byte)
	{
		auto is_upload = type == Upload;
		auto& resources = is_upload ? device->upload_resources : device->readback_resources;
		auto iter = resources.lower_bound(size_in_byte);
		if (iter != resources.end() && (iter->first == size_in_byte))
		{
			auto ret = iter->second;
			resources.erase(iter);
			return ret;
		}
		else {
			D3D12_RESOURCE_STATES init_state;
			D3D12_HEAP_PROPERTIES heap_prop;
			if (is_upload)
			{
				init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
				heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
			}
			else
			{
				init_state = D3D12_RESOURCE_STATE_COPY_DEST;
				heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
			}
			heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_prop.CreationNodeMask = 0;
			heap_prop.VisibleNodeMask = 0;

			D3D12_RESOURCE_DESC res_desc;
			res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			res_desc.Alignment = 0;
			res_desc.Width = size_in_byte;
			res_desc.Height = 1;
			res_desc.DepthOrArraySize = 1;
			res_desc.MipLevels = 1;
			res_desc.Format = DXGI_FORMAT_UNKNOWN;
			res_desc.SampleDesc.Count = 1;
			res_desc.SampleDesc.Quality = 0;
			res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

			COMPtr<ID3D12Resource> resource {};
			CheckHResult(device->d3d_device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
				&res_desc, init_state, nullptr,
				COMPtr_RefParam(resource,IID_ID3D12Resource)));
			return resource;
		}
	}

	void Context::ResidencyResource(COMPtr<ID3D12Resource> resource)
	{
		auto& resources = device->curr_render_cmd_allocator->recycle_after_sync_residency_buffs;

		resources.emplace_back(resource);
	}

	void Context::InnerResourceRecycle(InnerReourceType type, COMPtr<ID3D12Resource> resource, leo::uint32 size)
	{
		if (resource) {
			bool is_upload = type == Upload;
			auto& resources =is_upload?device->curr_render_cmd_allocator->recycle_after_sync_upload_buffs:device->curr_render_cmd_allocator->recycle_after_sync_readback_buffs;
			resources.emplace_back(resource, size);
		}
	}

	Fence & Context::GetFence(Device::CommandType index)
	{
		return *(device->fences[index]);
	}

	void Context::CreateDeviceAndDisplay() {
		device = std::make_shared<Device>(DefaultAdapter());
		ContextEx(device->d3d_device.Get(), device->d3d_cmd_queue.Get());
		DisplaySetting setting;
		display = std::make_shared<Display>(GetDXGIFactory4(), device->d3d_cmd_queue.Get(), setting, g_hwnd);//test code
		screen_frame_buffer = display->GetFrameBuffer();
		SetFrame(display->GetFrameBuffer());
	}
	void Context::DoBindFrameBuffer(const std::shared_ptr<platform::Render::FrameBuffer>& framebuffer)
	{
		static_cast<FrameBuffer*>(framebuffer.get())->BindBarrier();
	}
	void Context::Render(const Effect::Effect & effect, const Effect::Technique & tech, const platform::Render::InputLayout & layout)
	{
		//TODO Compute/Copy State -> SyncCPUGPU(true)

		auto& framebuffer = static_pointer_cast<FrameBuffer>(GetCurrFrame());

		std::vector<D3D12_RESOURCE_BARRIER> barriers;

		//TODO StreamOuput

		//Vertex Stream Barrier
		auto num_vertex_streams = layout.GetVertexStreamsSize();
		for (auto i = 0; i != num_vertex_streams; ++i) {
			auto& stream = layout.GetVertexStream(i);
			auto& vb = static_cast<GraphicsBuffer&>(*stream.stream);
			if (!(vb.GetAccess() & (EAccessHint::EA_CPURead | EAccessHint::EA_CPUWrite))) {
				D3D12_RESOURCE_BARRIER barrier;
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
		D3D12_PRIMITIVE_TOPOLOGY curr_topology = Convert<D3D12_PRIMITIVE_TOPOLOGY>(tt);
		d3d_cmd_lists[Device::Command_Render]->IASetPrimitiveTopology(curr_topology);

		auto prim_count = vertex_count;
		switch (tt)
		{
		case platform::Render::PrimtivteType::LineList:
			prim_count /= 2;
			break;
		case platform::Render::PrimtivteType::TriangleList:
			prim_count /= 3;
			break;
		case platform::Render::PrimtivteType::TriangleStrip:
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
	}
	void Context::BeginFrame()
	{
		SetFrame(GetScreenFrame());
	}
	void Context::EndFrame()
	{
		auto val = GetFence(Device::Command_Render).Signal((Fence::Type)Device::Command_Render);
		device->CmdAllocatorRecycle(device->curr_render_cmd_allocator, val);
		device->curr_render_cmd_allocator =device->CmdAllocatorAlloc();
		device->d3d_cmd_allocators[Device::Command_Render] = device->curr_render_cmd_allocator->cmd_allocator;

		ResetCommand(Device::Command_Render);
		ClearPSOCache();
	}
	Display & Context::GetDisplay()
	{
		return *display;
	}

	RayContext& D3D12::Context::GetRayContext()
	{
		if (ray_context)
			return *ray_context;
		else
			throw leo::unsupported();
	}

	CommandContext* D3D12::Context::GetDefaultCommandContext()
	{
		return render_command_context;
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