#include "Texture.h"
#include "Context.h"

using namespace platform_ex::Windows::D3D12;

void Texture::DeleteHWResource()
{
	texture = nullptr;
	texture_upload_heaps = nullptr;
	texture_readback_heaps = nullptr;
}

bool Texture::HWResourceReady()
{
	return bool(texture);
}

void Texture::DoCreateHWResource(D3D12_RESOURCE_DIMENSION dim, uint16 width, uint16 height, uint16 depth, uint8 array_size, ElementInitData const * init_data)
{
	auto & device = Context::Instance().GetDevice();
	auto base_this = dynamic_cast<platform::Render::Texture*>(this);

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
		tex_desc.DepthOrArraySize = array_size;
	default:
		LAssert(false, "RESOURCE_DIMENSION Type Error:Must TEXTURE_TYPE");
	}
	tex_desc.MipLevels = base_this->GetNumMipMaps();
	tex_desc.Format = dxgi_format;
	tex_desc.SampleDesc.Count = 1;
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

	CheckHResult(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
		&tex_desc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		COMPtr_RefParam(texture,IID_ID3D12Resource)));

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
	buff_desc.SampleDesc.Count = 1;
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

	if (init_data != nullptr) {
		auto& context = Context::Instance();
		auto & cmd_list = context.GetCommandList(Device::Command_Resource);

		D3D12_RESOURCE_BARRIER barrier_before;
		barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before.Transition.pResource = texture.Get();
		barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier_before.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		D3D12_RESOURCE_BARRIER barrier_after;
		barrier_after.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_after.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_after.Transition.pResource = texture.Get();
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
			dst.pResource = texture.Get();
			dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dst.SubresourceIndex = i;

			cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}
		cmd_list->ResourceBarrier(1, &barrier_after);
		context.CommitCommandList(Device::Command_Resource);
	}
}

template<typename _type>
void Texture::DoHWCopyToTexture(_type& src,_type & dst,RESOURCE_STATE_TRANSITION src_st,RESOURCE_STATE_TRANSITION dst_st)
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
	RESOURCE_STATE_TRANSITION src_st, RESOURCE_STATE_TRANSITION dst_st)
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
	D3D12_TEXTURE_COPY_LOCATION dst_location= {
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

template<typename _type>
bool Texture1DEqual(_type & lhs,_type & rhs) {
	return (lhs.GetWidth(0) == rhs.GetWidth(0)) &&
		(lhs.GetArraySize() == rhs.GetArraySize()) &&
		(lhs.GetNumMipMaps() == rhs.GetNumMipMaps()) &&
		(lhs.GetFormat() == rhs.GetFormat());
}

template<typename _type>
bool Texture2DEqual(_type & lhs, _type & rhs) {
	return (lhs.GetHeight(0) == rhs.GetHeight(0)) &&
		Texture1DEqual(lhs,rhs);
}

bool Texture3DEqual(Texture3D& lhs, Texture3D & rhs) {
	return (lhs.GetDepth(0) == rhs.GetDepth(0)) &&
		Texture3DEqual(lhs, rhs);
}

void Texture2D::CopyToTexture(platform::Render::Texture2D & base_target)
{
	auto& target = static_cast<Texture2D&>(base_target);

	if (Texture2DEqual(*this, target))
		DoHWCopyToTexture(*this, target);
	else {
		auto array_size = std::min(GetArraySize(), target.GetArraySize());
		auto num_mips = std::min(GetNumMipMaps(), target.GetNumMipMaps());
		for (auto index = 0; index != array_size; ++index) {
			for (auto level = 0; level != num_mips; ++level) {
				Resize(target, index, level, 0, 0, target.GetWidth(level), target.GetHeight(level),
					index, level, 0, 0, GetWidth(level), GetHeight(level), 
					true);
			}
		}
	}
}

void Texture2D::CopyToSubTexture(platform::Render::Texture2D & base_target, 
	uint8 dst_array_index, uint8 dst_level, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_width, uint16 dst_height, 
	uint8 src_array_index, uint8 src_level, uint16 src_x_offset, uint16 src_y_offset, uint16 src_width, uint16 src_height)
{
	auto& target = static_cast<Texture2D&>(base_target);

	if ((src_width == dst_width) && (src_height == dst_height) && (GetFormat() == target.GetFormat())) {
		auto src_subres = CalcSubresource(src_level, src_array_index, 0,
			GetNumMipMaps(),GetArraySize());
		auto dst_subres = CalcSubresource(dst_level, dst_array_index, 0,
			target.GetNumMipMaps(), target.GetArraySize());

		DoHWCopyToSubTexture(*this, target,
			dst_subres, dst_x_offset, dst_y_offset, 0,
			src_subres, src_x_offset, src_y_offset, 0,
			src_width, src_height, 1);
	}
	else {
		Resize(target,
			dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
			src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height,
			true);
	}
}

void Texture3D::CopyToTexture(platform::Render::Texture3D & target)
{
}


#include "../CommonTextureImpl.hcc"


