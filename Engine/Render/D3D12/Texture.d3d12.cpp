#include "Texture.h"
#include "Context.h"
#include "Convert.h"
#include "View.h"
#include "../Effect/CopyEffect.h"
#include <LBase/id.hpp>
#include <LFramework/LCLib/Debug.h>


using namespace platform_ex::Windows::D3D12;
using platform::Render::TextureMapAccess;

static DXGI_FORMAT ConvertWrap(EFormat format) {
	switch (format) {
	case EF_D16:
		return DXGI_FORMAT_R16_TYPELESS;
	case EF_D24S8:
		return DXGI_FORMAT_R24G8_TYPELESS;
	case EF_D32F:
		return DXGI_FORMAT_R32_TYPELESS;
	}
	return Convert(format);
}

platform_ex::Windows::D3D12::Texture::Texture(EFormat format)
	:dxgi_format(ConvertWrap(format))
{
}

platform_ex::Windows::D3D12::Texture::Texture(const COMPtr<ID3D12Resource>& pResource)
	:ResourceHolder(pResource)
{
	dxgi_format = desc.Format;
}

std::string  platform_ex::Windows::D3D12::Texture::HWDescription() const
{
	return leo::sfmt("[D3D12] Texture pResource=%p",resource.Get());
}

void Texture::DeleteHWResource()
{
	resource = nullptr;
	texture_upload_heaps = nullptr;
	texture_readback_heaps = nullptr;
}

bool Texture::ReadyHWResource() const
{
	return bool(resource);
}

void Texture::DoCreateHWResource(D3D12_RESOURCE_DIMENSION dim, uint16 width, uint16 height, uint16 depth, uint8 array_size, ElementInitData const *  init_data)
{
	auto & device = Context::Instance().GetDevice();
	auto base_this = dynamic_cast<platform::Render::Texture*>(this);
	lconstraint(base_this->GetSampleQuality() == 0);

	D3D12_RESOURCE_DESC tex_desc;
	tex_desc.Dimension = dim;
	tex_desc.Alignment = 0;
	tex_desc.Width = width;
	tex_desc.Height = height;
	switch (dim) {
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		tex_desc.DepthOrArraySize = array_size;
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		tex_desc.DepthOrArraySize = depth;
		break;
	default:
		LAssert(false, "RESOURCE_DIMENSION Type Error:Must TEXTURE_TYPE");
	}
	tex_desc.MipLevels = base_this->GetNumMipMaps();
	tex_desc.Format = dxgi_format;
	tex_desc.SampleDesc.Count = base_this->GetSampleCount();
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	tex_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	if (base_this->GetAccessMode() & EA_GPUWrite) {
		if (IsDepthFormat(base_this->GetFormat()))
			tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		else
			tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if (base_this->GetAccessMode() & EA_GPUUnordered)
		tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_HEAP_PROPERTIES heap_prop;
	heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
	heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_prop.CreationNodeMask = 0;
	heap_prop.VisibleNodeMask = 0;

	D3D12_RESOURCE_STATES init_state = D3D12_RESOURCE_STATE_COMMON;
	if (IsDepthFormat(base_this->GetFormat()) && (base_this->GetAccessMode() & EA_GPUWrite))
	{
		init_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		curr_state = init_state;
	}

	D3D12_CLEAR_VALUE* ClearValuePtr = nullptr;
	D3D12_CLEAR_VALUE ClearValue;
	if (init_data && init_data->clear_value) {
		auto& clear_value = *init_data->clear_value;
		if (tex_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET && clear_value.ColorBinding == platform::Render::ClearBinding::ColorBound)
		{
			ClearValue = CD3DX12_CLEAR_VALUE(tex_desc.Format, clear_value.Value.Color);
			ClearValuePtr = &ClearValue;
		}
		else if (tex_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL && clear_value.ColorBinding == platform::Render::ClearBinding::DepthStencilBound)
		{
			ClearValue = CD3DX12_CLEAR_VALUE(tex_desc.Format, clear_value.Value.DSValue.Depth, (uint8)clear_value.Value.DSValue.Stencil);
			ClearValuePtr = &ClearValue;
		}
	}

	CheckHResult(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
		&tex_desc, init_state, ClearValuePtr,
		COMPtr_RefParam(resource, IID_ID3D12Resource)));

	auto num_subres = array_size * base_this->GetNumMipMaps();
	uint64 upload_buffer_size = 0;
	device->GetCopyableFootprints(&tex_desc, 0, num_subres, 0, nullptr, nullptr, nullptr,
		&upload_buffer_size);

	D3D12_HEAP_PROPERTIES upload_heap_prop;
	upload_heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	upload_heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	upload_heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	upload_heap_prop.CreationNodeMask = 0;
	upload_heap_prop.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC buff_desc;
	buff_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buff_desc.Alignment = 0;
	buff_desc.Width = upload_buffer_size;
	buff_desc.Height = 1;
	buff_desc.DepthOrArraySize = 1;
	buff_desc.MipLevels = 1;
	buff_desc.Format = DXGI_FORMAT_UNKNOWN;
	buff_desc.SampleDesc.Count = base_this->GetSampleCount();
	buff_desc.SampleDesc.Quality = 0;
	buff_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	buff_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	CheckHResult(device->CreateCommittedResource(&upload_heap_prop, D3D12_HEAP_FLAG_NONE,
		&buff_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		COMPtr_RefParam(texture_upload_heaps, IID_ID3D12Resource)));

	D3D12_HEAP_PROPERTIES readback_heap_prop;
	readback_heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
	readback_heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	readback_heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	readback_heap_prop.CreationNodeMask = 0;
	readback_heap_prop.VisibleNodeMask = 0;

	CheckHResult(device->CreateCommittedResource(&readback_heap_prop, D3D12_HEAP_FLAG_NONE,
		&buff_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
		COMPtr_RefParam(texture_readback_heaps, IID_ID3D12Resource)));

	if (init_data != nullptr && init_data->data != nullptr) {
		auto& context = Context::Instance();
		auto & cmd_list = context.GetCommandList(Device::Command_Resource);

		D3D12_RESOURCE_BARRIER barrier_before;
		barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before.Transition.pResource = resource.Get();
		barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier_before.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		D3D12_RESOURCE_BARRIER barrier_after;
		barrier_after.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_after.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_after.Transition.pResource = resource.Get();
		barrier_after.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier_after.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		barrier_after.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(num_subres);
		std::vector<uint64> row_sizes_in_bytes(num_subres);
		std::vector<uint32> num_rows(num_subres);

		uint64 required_size = 0;
		device->GetCopyableFootprints(&tex_desc, 0, num_subres, 0, &layouts[0], &num_rows[0], &row_sizes_in_bytes[0], &required_size);

		uint8* p;
		texture_upload_heaps->Map(0, nullptr, reinterpret_cast<void**>(&p));
		for (auto i = 0; i < num_subres; ++i)
		{
			D3D12_SUBRESOURCE_DATA src_data;
			src_data.pData = init_data[i].data;
			src_data.RowPitch = init_data[i].row_pitch;
			src_data.SlicePitch = init_data[i].slice_pitch;

			D3D12_MEMCPY_DEST dest_data;
			dest_data.pData = p + layouts[i].Offset;
			dest_data.RowPitch = layouts[i].Footprint.RowPitch;
			dest_data.SlicePitch = layouts[i].Footprint.RowPitch * num_rows[i];

			for (auto z = 0U; z != layouts[i].Footprint.Depth; ++z)
			{
				uint8 const * src_slice
					= reinterpret_cast<uint8 const *>(src_data.pData) + src_data.SlicePitch * z;
				uint8* dest_slice = reinterpret_cast<uint8*>(dest_data.pData) + dest_data.SlicePitch * z;
				for (UINT y = 0; y < num_rows[i]; ++y)
				{
					memcpy(dest_slice + dest_data.RowPitch * y, src_slice + src_data.RowPitch * y,
						static_cast<size_t>(row_sizes_in_bytes[i]));
				}
			}
		}
		texture_upload_heaps->Unmap(0, nullptr);

		std::lock_guard<std::mutex> lock(context.GetCommandListMutex(Device::Command_Resource));

		cmd_list->ResourceBarrier(1, &barrier_before);
		for (auto i = 0; i != num_subres; ++i)
		{
			D3D12_TEXTURE_COPY_LOCATION src;
			src.pResource = texture_upload_heaps.Get();
			src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			src.PlacedFootprint = layouts[i];

			D3D12_TEXTURE_COPY_LOCATION dst;
			dst.pResource = resource.Get();
			dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dst.SubresourceIndex = i;

			cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}
		cmd_list->ResourceBarrier(1, &barrier_after);
		context.CommitCommandList(Device::Command_Resource);

	}

	desc = tex_desc;

}

void Texture::DoMap(EFormat format, uint32 subres, TextureMapAccess tma,
	uint16 x_offset, uint16 y_offset, uint16 z_offset,
	/*uint16 width,*/  uint16 height, uint16 depth,
	void *& data, uint32 & row_pitch, uint32 & slice_pitch)
{
	auto & context = Context::Instance();
	auto & device = context.GetDevice();

	last_tma = tma;

	auto tex_desc = resource->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	uint32 num_rows;
	uint64 row_sizes_in_bytes;
	uint64 required_size = 0;
	device->GetCopyableFootprints(&tex_desc, subres, 1, 0, &layout, &num_rows, &row_sizes_in_bytes, &required_size);

	if ((TextureMapAccess::ReadOnly == tma) || (TextureMapAccess::ReadWrite == tma)) {
		auto & cmd_list = context.GetCommandList(Device::Command_Render);

		TransitionBarrier barrier = {
			{D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_STATE_COPY_SOURCE},
			resource,
			subres
		};
		cmd_list->ResourceBarrier(1, barrier);

		D3D12_TEXTURE_COPY_LOCATION src = {
			resource.Get(),
			D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
			subres
		};

		D3D12_TEXTURE_COPY_LOCATION dst = {
			texture_readback_heaps.Get(),
			D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
			layout
		};

		cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		cmd_list->ResourceBarrier(1, !barrier);

		context.SyncCPUGPU();
	}

	uint8 * p;
	texture_upload_heaps->Map(0, nullptr, reinterpret_cast<void**>(&p));

	data = p + layout.Offset + (z_offset*layout.Footprint.Height + y_offset) * layout.Footprint.RowPitch
		+ x_offset * NumFormatBytes(format);
	row_pitch = layout.Footprint.RowPitch;
	slice_pitch = layout.Footprint.RowPitch * layout.Footprint.Height;

	if ((TextureMapAccess::ReadOnly == tma) || (TextureMapAccess::ReadWrite == tma)) {
		texture_readback_heaps->Map(0, nullptr, reinterpret_cast<void**>(&p));
		uint8* src_p = p + layout.Offset + (z_offset * layout.Footprint.Height + y_offset) * layout.Footprint.RowPitch
			+ x_offset * NumFormatBytes(format);
		uint8* dst_p = static_cast<uint8*>(data);
		for (auto z = 0; z != depth; ++z)
		{
			memcpy(dst_p + z * slice_pitch, src_p + z * slice_pitch, row_pitch * height);
		}
		texture_readback_heaps->Unmap(0, nullptr);
	}
}

void Texture::DoUnmap(uint32 subres)
{
	auto & context = Context::Instance();
	auto & device = context.GetDevice();

	texture_upload_heaps->Unmap(0, nullptr);


	auto tex_desc = resource->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	uint32 num_rows;
	uint64 row_sizes_in_bytes;
	uint64 required_size = 0;
	device->GetCopyableFootprints(&tex_desc, subres, 1, 0, &layout, &num_rows, &row_sizes_in_bytes, &required_size);
	if ((TextureMapAccess::WriteOnly == last_tma) || (TextureMapAccess::ReadWrite == last_tma)) {
		auto cmd_list = context.GetCommandList(Device::Command_Render);
		context.SyncCPUGPU();

		TransitionBarrier barrier{
			{D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_STATE_COPY_SOURCE},
			resource,
			subres
		};

		cmd_list->ResourceBarrier(1, barrier);

		D3D12_TEXTURE_COPY_LOCATION src = {
			texture_upload_heaps.Get(),
			D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
			layout
		};

		D3D12_TEXTURE_COPY_LOCATION dst = {
			resource.Get(),
			D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
			subres
		};

		cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);


		cmd_list->ResourceBarrier(1, !barrier);
	}
}

//TODO use platform::Render::CommandList
void platform_ex::Windows::D3D12::Texture::DoHWBuildMipSubLevels(uint8 array_size, uint8 mipmap_size, uint16 width, uint16 height, uint8 facecount)
{
		auto & device = D3D12::Context::Instance().GetDevice();
		auto & effect = device.BiltEffect();
		auto & tech = effect.BilinearCopy;
		auto & pass = tech.get().GetPass(0);

		auto& sc = static_cast<ShaderCompose&>(pass.GetShader(effect));

		auto & rl = static_cast<InputLayout&>(device.PostProcessLayout());
		
		auto& state = static_cast<const PipleState&>(pass.GetState());
		auto gps_desc = state.graphics_ps_desc;
		gps_desc.pRootSignature = sc.RootSignature()->Signature.Get();
		{
			gps_desc.VS << sc.sc_template->VertexShader;
			gps_desc.PS << sc.sc_template->PixelShader;

			gps_desc.DS << nullptr;
			gps_desc.HS << nullptr;
			gps_desc.GS << nullptr;
		}

		gps_desc.StreamOutput << nullptr;
		gps_desc.SampleMask = 0xFFFFFFFF;

		gps_desc.InputLayout.NumElements = static_cast<UINT>(rl.GetInputDesc().size());
		gps_desc.InputLayout.pInputElementDescs = rl.GetInputDesc().data();

		gps_desc.IBStripCutValue = rl.GetIndexFormat() == EFormat::EF_R16UI ? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;

		gps_desc.PrimitiveTopologyType = Convert<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(rl.GetTopoType());

		gps_desc.NumRenderTargets = 1;
		gps_desc.RTVFormats[0] = dxgi_format;

		for (uint32_t i = gps_desc.NumRenderTargets; i != leo::arrlen(gps_desc.RTVFormats); ++i)
		{
			gps_desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
		}

		gps_desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		gps_desc.SampleDesc.Count = 1;
		gps_desc.SampleDesc.Quality = 0;
		gps_desc.NodeMask = 0;
		gps_desc.CachedPSO.pCachedBlob = nullptr;
		gps_desc.CachedPSO.CachedBlobSizeInBytes = 0;
		gps_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		auto pso = device.CreateRenderPSO(gps_desc);

		auto& cmd_list = D3D12::Context::Instance().GetCommandList(Device::Command_Render);
		cmd_list->SetPipelineState(pso.get());
		cmd_list->SetGraphicsRootSignature(gps_desc.pRootSignature);

		auto dynamic_heap = device.CreateDynamicCBVSRVUAVDescriptorHeap(array_size*(mipmap_size - 1));
		auto  sampler_heap = sc.SamplerHeap();

		ID3D12DescriptorHeap* heaps[] = { dynamic_heap.get(),sampler_heap };
		cmd_list->SetDescriptorHeaps(static_cast<UINT>(leo::arrlen(heaps)), heaps);

		if (sampler_heap)
			cmd_list->SetGraphicsRootDescriptorTable(1, sampler_heap->GetGPUDescriptorHandleForHeapStart());


		auto & vb = static_cast<GraphicsBuffer&>(platform::Deref(rl.GetVertexStream(0).stream));

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = vb.Resource()->GetGPUVirtualAddress();
		vbv.SizeInBytes = vb.GetSize();
		vbv.StrideInBytes = rl.GetVertexStream(0).vertex_size;

		cmd_list->IASetVertexBuffers(0, 1, &vbv);

		cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		D3D12_VIEWPORT vp;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;

		D3D12_RECT scissor_rc;
		scissor_rc.left = 0;
		scissor_rc.top = 0;

		D3D12_RESOURCE_BARRIER barrier_before[2];
		barrier_before[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before[0].Transition.pResource = resource.Get();
		barrier_before[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier_before[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier_before[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before[1].Transition.pResource = resource.Get();
		barrier_before[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier_before[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		D3D12_RESOURCE_BARRIER barrier_after[2] = { barrier_before[0],barrier_before[1] };
		std::swap(barrier_after[0].Transition.StateBefore, barrier_after[0].Transition.StateAfter);
		std::swap(barrier_after[1].Transition.StateBefore, barrier_after[1].Transition.StateAfter);

		auto cpu_cbv_srv_uav_handle = dynamic_heap->GetCPUDescriptorHandleForHeapStart();
		auto gpu_cbv_srv_uav_handle = dynamic_heap->GetGPUDescriptorHandleForHeapStart();
		auto srv_desc_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		auto& ITexture = dynamic_cast<platform::Render::Texture&>(*this);
		for (uint8_t index = 0; index < array_size; ++index)
		{
			for (uint8 face = 0; face != facecount; ++face) {
				for (uint8_t level = 1; level < mipmap_size; ++level)
				{
					cmd_list->SetGraphicsRootDescriptorTable(0, gpu_cbv_srv_uav_handle);

					barrier_before[0].Transition.Subresource = CalcSubresource(level - 1, index*facecount+face, 0, mipmap_size, array_size);
					barrier_before[1].Transition.Subresource = CalcSubresource(level, index*facecount + face, 0, mipmap_size, array_size);
					cmd_list->ResourceBarrier(2, barrier_before);

					RenderTargetView rtv(GetDefaultNodeDevice(), CreateRTVDesc(ITexture, index* facecount + face, 1, level), *this);
					auto const rt_handle = rtv.GetView();

					ShaderResourceView srv(GetDefaultNodeDevice(), CreateSRVDesc(ITexture, index* facecount + face, 1, level - 1, 1), *this);
					auto const sr_handle = srv.GetView();
					device->CopyDescriptorsSimple(1, cpu_cbv_srv_uav_handle, sr_handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

					cmd_list->OMSetRenderTargets(1, &rt_handle, false, nullptr);

					vp.Width = static_cast<float>(width >> level);
					vp.Height = static_cast<float>(height >> level);
					cmd_list->RSSetViewports(1, &vp);

					scissor_rc.right = width >> level;
					scissor_rc.bottom = height >> level;
					cmd_list->RSSetScissorRects(1, &scissor_rc);

					cmd_list->DrawInstanced(4, 1, 0, 0);

					barrier_after[0].Transition.Subresource = barrier_before[0].Transition.Subresource;
					barrier_after[1].Transition.Subresource = barrier_before[1].Transition.Subresource;
					cmd_list->ResourceBarrier(2, barrier_after);

					cpu_cbv_srv_uav_handle.ptr += srv_desc_size;
					gpu_cbv_srv_uav_handle.ptr += srv_desc_size;
				}
			}
		}
}

template<typename _type>
void Texture::DoHWCopyToTexture(_type& src, _type & dst, ResourceStateTransition src_st, ResourceStateTransition dst_st)
{
	auto& context = Context::Instance();
	auto& cmd_list = context.GetCommandList(Device::Command_Render);

	if ((src.GetAccessMode() & EA_CPUWrite) && (dst.GetAccessMode() & EA_GPURead))
		context.SyncCPUGPU();

	D3D12_RESOURCE_BARRIER barrier_src;
	barrier_src.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier_src.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier_src.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	auto barrier_target = barrier_src;

	barrier_src.Transition.StateBefore = src_st.StateBefore;
	barrier_target.Transition.StateBefore = dst_st.StateBefore;
	barrier_src.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier_target.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier_src.Transition.pResource = src.Resource();
	barrier_target.Transition.pResource = dst.Resource();

	D3D12_RESOURCE_BARRIER barriers[] = { barrier_src,barrier_target };
	cmd_list->ResourceBarrier(2, barriers);

	auto num_subres = src.GetArraySize() * src.GetNumMipMaps();
	if ((src.GetSampleCount() > 1) && (1 == dst.GetSampleCount())) {
		for (auto i = 0; i != num_subres; ++i)
			cmd_list->ResolveSubresource(dst.Resource(), i, src.Resource(), i, src.GetDXGIFormat());
	}
	else
		cmd_list->CopyResource(dst.Resource(), src.Resource());

	barrier_src.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier_target.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier_src.Transition.StateAfter = src_st.StateAfter;
	barrier_target.Transition.StateAfter = dst_st.StateAfter;
	D3D12_RESOURCE_BARRIER barriers_rollback[] = { barrier_src,barrier_target };
	cmd_list->ResourceBarrier(2, barriers_rollback);
}

template<typename _type>
void Texture::DoHWCopyToSubTexture(_type & src, _type & target,
	uint32 dst_subres, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_z_offset,
	uint32 src_subres, uint16 src_x_offset, uint16 src_y_offset, uint16 src_z_offset,
	uint16 width, uint16 height, uint16 depth,
	ResourceStateTransition src_st, ResourceStateTransition dst_st)
{
	auto& context = Context::Instance();
	auto& cmd_list = context.GetCommandList(Device::Command_Render);

	if ((src.GetAccessMode() & EA_CPUWrite) && (target.GetAccessMode() & EA_GPURead))
		context.SyncCPUGPU();

	D3D12_RESOURCE_BARRIER barrier_src;
	barrier_src.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier_src.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	auto barrier_target = barrier_src;

	barrier_src.Transition.Subresource = src_subres;
	barrier_target.Transition.Subresource = dst_subres;
	barrier_src.Transition.StateBefore = src_st.StateBefore;
	barrier_target.Transition.StateBefore = dst_st.StateBefore;
	barrier_src.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier_target.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier_src.Transition.pResource = src.Resource();
	barrier_target.Transition.pResource = target.Resource();

	D3D12_RESOURCE_BARRIER barriers[] = { barrier_src,barrier_target };
	cmd_list->ResourceBarrier(2, barriers);

	D3D12_TEXTURE_COPY_LOCATION src_location = {
		src.Resource(),
		D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		src_subres };
	D3D12_TEXTURE_COPY_LOCATION dst_location = {
		target.Resource(),
		D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		dst_subres };

	D3D12_BOX src_box = {
		src_x_offset,
		src_y_offset,
		src_z_offset,
		static_cast<UINT>(src_x_offset + width),
		static_cast<UINT>(src_y_offset + height),
		static_cast<UINT>(src_z_offset + depth)
	};

	cmd_list->CopyTextureRegion(&dst_location, dst_x_offset, dst_y_offset, dst_z_offset, &src_location, &src_box);

	barrier_src.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier_target.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier_src.Transition.StateAfter = src_st.StateAfter;
	barrier_target.Transition.StateAfter = dst_st.StateAfter;
	D3D12_RESOURCE_BARRIER barriers_rollback[] = { barrier_src,barrier_target };
	cmd_list->ResourceBarrier(2, barriers_rollback);
}

//@{
void Texture1D::CopyToTexture(platform::Render::Texture1D & base_target)
{
	auto& target = static_cast<Texture1D&>(base_target);

	if (Equal(*this, target))
		DoHWCopyToTexture(*this, target);
	else {
		for (uint8 index = 0; index != array_size; ++index) {
			for (uint8 level = 0; level != mipmap_size; ++level) {
				Resize(target,
					Box1D({ index, level }, 0, target.GetWidth(level)),
					Box1D({ index, level }, 0, GetWidth(level)),
					true);
			}
		}
	}
}

void Texture1D::CopyToSubTexture(platform::Render::Texture1D & base_target, const Box1D& dst, const Box1D& src)
{
	auto& target = static_cast<Texture1D&>(base_target);

	if ((src.width == dst.width) && (GetFormat() == target.GetFormat())) {
		auto src_subres = CalcSubresource(src.level, src.array_index, 0,
			GetNumMipMaps(), GetArraySize());
		auto dst_subres = CalcSubresource(dst.level, dst.array_index, 0,
			target.GetNumMipMaps(), target.GetArraySize());

		DoHWCopyToSubTexture(*this, target,
			dst_subres, dst.x_offset, 0, 0,
			src_subres, src.x_offset, 0, 0,
			src.width, 1, 1);
	}
	else {
		Resize(target,
			dst,
			src,
			true);
	}
}


void Texture2D::CopyToTexture(platform::Render::Texture2D & base_target)
{
	auto& target = static_cast<Texture2D&>(base_target);

	if (Equal(*this, target))
		DoHWCopyToTexture(*this, target);
	else {
		auto array_size = std::min(GetArraySize(), target.GetArraySize());
		auto num_mips = std::min(GetNumMipMaps(), target.GetNumMipMaps());
		for (uint8 index = 0; index != array_size; ++index) {
			for (uint8 level = 0; level != mipmap_size; ++level) {
				Resize(target,
				{ {{index, level}, 0,target.GetWidth(level)},0, target.GetHeight(level) },
				{ {{index, level}, 0, GetWidth(level)},0, GetHeight(level) },
					true);
			}
		}
	}
}

void Texture2D::CopyToSubTexture(platform::Render::Texture2D & base_target,
	const Box2D& dst,
	const Box2D& src)
{
	auto& target = static_cast<Texture2D&>(base_target);

	if ((src.width == dst.width) && (src.height == dst.height) && (GetFormat() == target.GetFormat())) {
		auto src_subres = CalcSubresource(src.level, src.array_index, 0,
			GetNumMipMaps(), GetArraySize());
		auto dst_subres = CalcSubresource(dst.level, dst.array_index, 0,
			target.GetNumMipMaps(), target.GetArraySize());

		DoHWCopyToSubTexture(*this, target,
			dst_subres, dst.x_offset, dst.y_offset, 0,
			src_subres, src.x_offset, src.y_offset, 0,
			src.width, src.height, 1);
	}
	else {
		Resize(target,
			dst,
			src,
			true);
	}
}

void Texture3D::CopyToTexture(platform::Render::Texture3D & base_target)
{
	auto& target = static_cast<Texture3D&>(base_target);

	if (Equal(*this, target))
		DoHWCopyToTexture(*this, target);
	else {
		auto array_size = std::min(GetArraySize(), target.GetArraySize());
		auto num_mips = std::min(GetNumMipMaps(), target.GetNumMipMaps());
		for (uint8 index = 0; index != array_size; ++index) {
			for (uint8 level = 0; level != num_mips; ++level) {
				Resize(target,
				{ {{{index, level}, 0, target.GetWidth(level)},0, target.GetHeight(level)},0,target.GetDepth(level) },
					Box3D(Texture2D::Box2D({ {index, level}, 0, GetWidth(level) }, 0, GetHeight(level)), 0, GetDepth(level)),
					true);
			}
		}
	}
}

void Texture3D::CopyToSubTexture(platform::Render::Texture3D & base_target, const Box3D& dst, const Box3D& src)
{
	auto& target = static_cast<Texture3D&>(base_target);

	if ((src.width == dst.width) && (src.height == dst.height) && (GetFormat() == target.GetFormat())) {
		auto src_subres = CalcSubresource(src.level, src.array_index, 0,
			GetNumMipMaps(), GetArraySize());
		auto dst_subres = CalcSubresource(dst.level, dst.array_index, 0,
			target.GetNumMipMaps(), target.GetArraySize());

		DoHWCopyToSubTexture(*this, target,
			dst_subres, dst.x_offset, dst.y_offset, dst.z_offset,
			src_subres, src.x_offset, src.y_offset, src.z_offset,
			src.width, src.height, 1);
	}
	else {
		Resize(target,
			dst,
			src,
			true);
	}
}


void TextureCube::CopyToTexture(platform::Render::TextureCube & base_target)
{
	auto& target = static_cast<TextureCube&>(base_target);

	if (Texture1D::Equal(*this, target))
		DoHWCopyToTexture(*this, target);
	else {
		auto array_size = std::min(GetArraySize(), target.GetArraySize());
		auto num_mips = std::min(GetNumMipMaps(), target.GetNumMipMaps());
		for (uint8 index = 0; index != array_size; ++index) {
			for (uint8 f = 0; f != 6; ++f) {
				auto face = static_cast<CubeFaces>(f);
				for (uint8 level = 0; level != mipmap_size; ++level) {
					Resize(target,
					{ {{ { index, level }, 0,target.GetWidth(level) },0, target.GetHeight(level)},face },
					{ {{ { index, level }, 0, GetWidth(level) },0, GetHeight(level) }, face
					},
						true);
				}
			}
		}
	}
}

void TextureCube::CopyToSubTexture(platform::Render::TextureCube & base_target,
	const BoxCube& dst,
	const BoxCube& src)
{
	auto& target = static_cast<TextureCube&>(base_target);

	if ((src.width == dst.width) && (src.height == dst.height) && (GetFormat() == target.GetFormat())) {
		auto src_subres = CalcSubresource(src.level, src.array_index, 0,
			GetNumMipMaps(), GetArraySize() * 6);
		auto dst_subres = CalcSubresource(dst.level, dst.array_index, 0,
			target.GetNumMipMaps(), target.GetArraySize() * 6);

		DoHWCopyToSubTexture(*this, target,
			dst_subres, dst.x_offset, dst.y_offset, 0,
			src_subres, src.x_offset, src.y_offset, 0,
			src.width, src.height, 1);
	}
	else {
		Resize(target,
			dst,
			src,
			true);
	}
}


std::string  Texture1D::Description() const
{
	// TODO FORMAT Width Infomation
	return HWDescription();
}

std::string  Texture2D::Description() const
{
	// TODO FORMAT Width Height Infomation
	return HWDescription();
}

std::string  Texture3D::Description() const
{
	// TODO FORMAT Width Height Depth Infomation
	return HWDescription();
}

std::string  TextureCube::Description() const
{
	// TODO FORMAT Size Infomation
	return HWDescription();
}

//}

using platform::Render::TextureType;
using namespace platform_ex::Windows;

Texture* platform_ex::Windows::D3D12::dynamic_cast_texture(platform::Render::Texture* pTex)
{
	switch (pTex->GetType())
	{
	case TextureType::T_1D:
		return  static_cast<Texture1D*>(pTex);
	case TextureType::T_2D:
		return  static_cast<Texture2D*>(pTex);
	case TextureType::T_3D:
		return  static_cast<Texture3D*>(pTex);
	case TextureType::T_Cube:
		return  static_cast<TextureCube*>(pTex);
	}
	return dynamic_cast<Texture*>(pTex);
}

D3D12_SHADER_RESOURCE_VIEW_DESC D3D12::CreateSRVDesc(const platform::Render::Texture& tex, uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC Desc{};
	switch (tex.GetType())	
	{
	case TextureType::T_1D:
		Desc = static_cast<const Texture1D&>(tex).CreateSRVDesc(first_array_index, num_items, first_level, num_levels);
		break;
	case TextureType::T_2D:
		Desc = static_cast<const Texture2D&>(tex).CreateSRVDesc(first_array_index, num_items, first_level, num_levels); break;
	case TextureType::T_3D:
		Desc = static_cast<const Texture3D&>(tex).CreateSRVDesc(first_array_index, num_items, first_level, num_levels); break;
	case TextureType::T_Cube:
		Desc = static_cast<const TextureCube&>(tex).CreateSRVDesc(first_array_index, num_items, first_level, num_levels); break;
	}
	return Desc;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12::CreateUAVDesc(const platform::Render::Texture& tex, uint8 first_array_index, uint8 num_items, uint8 level)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC Desc{};
	switch (tex.GetType())
	{
	case TextureType::T_1D:
		Desc = static_cast<const Texture1D&>(tex).CreateUAVDesc(first_array_index, num_items, level);
		break;
	case TextureType::T_2D:
		Desc = static_cast<const Texture2D&>(tex).CreateUAVDesc(first_array_index, num_items, level);
		break;
	case TextureType::T_3D:
		Desc = static_cast<const Texture3D&>(tex).CreateUAVDesc(first_array_index, num_items, level); 
		break;
	case TextureType::T_Cube:
		Desc = static_cast<const TextureCube&>(tex).CreateUAVDesc(first_array_index, num_items, level);
		break;
	}
	return Desc;
}

D3D12_RENDER_TARGET_VIEW_DESC platform_ex::Windows::D3D12::CreateRTVDesc(const platform::Render::Texture& tex, uint8 first_array_index, uint8 num_items, uint8 level)
{
	D3D12_RENDER_TARGET_VIEW_DESC Desc{};
	switch (tex.GetType())
	{
	case TextureType::T_1D:
		Desc = static_cast<const Texture1D&>(tex).CreateRTVDesc(first_array_index, num_items, level);
		break;
	case TextureType::T_2D:
		Desc = static_cast<const Texture2D&>(tex).CreateRTVDesc(first_array_index, num_items, level); break;
	case TextureType::T_3D:
		lconstraint(false);
		break;
	case TextureType::T_Cube:
		Desc = static_cast<const TextureCube&>(tex).CreateRTVDesc(first_array_index, num_items, level); break;
	}
	return Desc;
}

D3D12_DEPTH_STENCIL_VIEW_DESC D3D12::CreateDSVDesc(const platform::Render::Texture& tex, uint8 first_array_index, uint8 num_items, uint8 level)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC Desc{};
	switch (tex.GetType())
	{
	case TextureType::T_1D:
		lconstraint(false);
		break;
	case TextureType::T_2D:
		Desc = static_cast<const Texture2D&>(tex).CreateDSVDesc(first_array_index, num_items, level); break;
	case TextureType::T_3D:
		lconstraint(false);
		break;
	case TextureType::T_Cube:
		Desc = static_cast<const TextureCube&>(tex).CreateDSVDesc(first_array_index, num_items, level); break;
	}
	return Desc;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12::CreateUAVDesc(const platform::Render::Texture& tex, uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC Desc{};
	switch (tex.GetType())
	{
	case TextureType::T_1D:
	case TextureType::T_2D:
	case TextureType::T_Cube:
		lconstraint(false);
		break;
	case TextureType::T_3D:
		Desc = static_cast<const Texture3D&>(tex).CreateUAVDesc(array_index, first_slice, num_slices, level);
		break;
	}
	return Desc;
}

D3D12_RENDER_TARGET_VIEW_DESC D3D12::CreateRTVDesc(const platform::Render::Texture& tex, uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level)
{
	D3D12_RENDER_TARGET_VIEW_DESC Desc{};
	switch (tex.GetType())
	{
	case TextureType::T_1D:
	case TextureType::T_2D:
	case TextureType::T_Cube:
		lconstraint(false);
		break;
	case TextureType::T_3D:
		Desc = static_cast<const Texture3D&>(tex).CreateRTVDesc(array_index, first_slice, num_slices, level);
		break;
	}
	return Desc;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC platform_ex::Windows::D3D12::CreateUAVDesc(const platform::Render::Texture& tex, uint8 first_array_index, uint8 num_items, TextureCubeFaces first_face, uint8 num_faces, uint8 level)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC Desc{};
	switch (tex.GetType())
	{
	case TextureType::T_1D:
	case TextureType::T_2D:
	case TextureType::T_3D:
		lconstraint(false);
		break;
	case TextureType::T_Cube:
		Desc = static_cast<const TextureCube&>(tex).CreateUAVDesc(first_array_index, num_items, first_face, num_faces,level); 
		break;
	}
	return Desc;
}

D3D12_RENDER_TARGET_VIEW_DESC platform_ex::Windows::D3D12::CreateRTVDesc(const platform::Render::Texture& tex, uint8 array_index, TextureCubeFaces face, uint8 level)
{
	D3D12_RENDER_TARGET_VIEW_DESC Desc{};
	switch (tex.GetType())
	{
	case TextureType::T_1D:
	case TextureType::T_2D:
	case TextureType::T_3D:
		lconstraint(false);
		break;
	case TextureType::T_Cube:
		Desc = static_cast<const TextureCube&>(tex).CreateRTVDesc(array_index, face, level);
		break;
	}
	return Desc;
}

D3D12_DEPTH_STENCIL_VIEW_DESC platform_ex::Windows::D3D12::CreateDSVDesc(const platform::Render::Texture& tex, uint8 array_index, TextureCubeFaces face, uint8 level)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC Desc{};
	switch (tex.GetType())
	{
	case TextureType::T_1D:
	case TextureType::T_2D:
	case TextureType::T_3D:
		lconstraint(false);
		break;
	case TextureType::T_Cube:
		Desc = static_cast<const TextureCube&>(tex).CreateDSVDesc(array_index, face, level);
		break;
	}
	return Desc;
}


#include "../CommonTextureImpl.hcc"


