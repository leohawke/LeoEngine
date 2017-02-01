#include "Texture.h"
#include "RenderView.h"
#include "Convert.h"

using namespace platform_ex::Windows::D3D12;
using BTexture = platform::Render::Texture3D;

Texture3D::Texture3D(uint16 width_, uint16 height_, uint16 depth_,
	uint8 numMipMaps, uint8 array_size_, 
	EFormat format_, uint32 access_hint,SampleDesc sample_info) 
:BTexture(numMipMaps, array_size_, format_, access_hint, sample_info),
Texture(format_),
width(width_), height(height_),depth(depth_)
{
	if (0 == mipmap_size) {
		mipmap_size = 1;
		auto w = width;
		auto h = height;
		auto d = depth;
		while ((w > 1) || (h > 1) || (d > 1)) {
			++mipmap_size;
			w /= 2;
			h /= 2;
			d /= 2;
		}
	}
}

void Texture3D::BuildMipSubLevels() {
	for (uint8 index = 0; index != GetArraySize(); ++index)
	{
		for (uint8 level = 1; level != GetNumMipMaps(); ++level)
		{
			Resize(*this, { index, level, 0, 0,0, GetWidth(level), GetHeight(level),GetDepth(level) },
			{ index, static_cast<uint8>(level - 1), 0, 0,0, GetWidth(level - 1), GetHeight(level - 1),GetDepth(level-1)}, true);
		}
	}
}

void Texture3D::HWResourceCreate(ElementInitData const *  init_data) {
	Texture::DoCreateHWResource(D3D12_RESOURCE_DIMENSION_TEXTURE3D,
		width, height,depth, array_size,
		init_data);
}

void Texture3D::HWResourceDelete() {
	return Texture::DeleteHWResource();
}

bool Texture3D::HWResourceReady() const {
	return Texture::ReadyHWResource();
}

uint16 Texture3D::GetWidth(uint8 level) const {
	LAssert(level < mipmap_size, "level out of range");
	return std::max(1, width >> level);
}
uint16 Texture3D::GetHeight(uint8 level) const {
	LAssert(level < mipmap_size, "level out of range");
	return std::max(1, height >> level);
}
uint16 Texture3D::GetDepth(uint8 level) const {
	LAssert(level < mipmap_size, "level out of range");
	return std::max(1, depth >> level);
}

void Texture3D::Map(TextureMapAccess tma,
	void*& data, uint32& row_pitch, uint32& slice_pitch, const Box3D& box) {
	auto subres = CalcSubresource(box.level, box.array_index, 0, mipmap_size, array_size);

	DoMap(format, subres, tma, box.x_offset, box.y_offset,box.z_offset, box.height,box.depth, data, row_pitch, slice_pitch);
}

void Texture3D::UnMap(const Sub1D& box) {
	auto subres = CalcSubresource(box.level, box.array_index, 0, mipmap_size, array_size);
	DoUnmap(subres);
}


ViewSimulation*  Texture3D::RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) {
	LAssert(GetAccessMode() & EA_GPURead, "Access mode must have EA_GPURead flag");
	D3D12_SHADER_RESOURCE_VIEW_DESC desc;
	switch (format) {
	case EF_D16:
		desc.Format = DXGI_FORMAT_R16_UNORM;
		break;
	case EF_D24S8:
		desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	case EF_D32F:
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		break;
	default:
		desc.Format = dxgi_format;
	}
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	desc.Texture3D.MostDetailedMip = first_level;
	desc.Texture3D.MipLevels = num_levels;
	desc.Texture3D.ResourceMinLODClamp = 0;

	return RetriveSRV(desc);
}

ViewSimulation*  Texture3D::RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level) {
	return RetriveUnorderedAccessView(first_array_index, 0, depth, level);
}
ViewSimulation*  Texture3D::RetriveUnorderedAccessView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) {
	LAssert(GetAccessMode() & EA_GPUUnordered, "Access mode must have EA_GPUUnordered flag");

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;

	desc.Format = dxgi_format;
	desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	desc.Texture3D.MipSlice = level;
	desc.Texture3D.FirstWSlice = first_slice;
	desc.Texture3D.WSize = num_slices;

	return RetriveUAV(desc);
}

ViewSimulation*  Texture3D::RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) {
	D3D12_RENDER_TARGET_VIEW_DESC desc;

	desc.Format = Convert(format);
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
	desc.Texture3D.MipSlice = level;
	desc.Texture3D.FirstWSlice = first_slice;
	desc.Texture3D.WSize = num_slices;

	return RetriveRTV(desc);
}
ViewSimulation*  Texture3D::RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) {

	LAssert(GetAccessMode() & EA_GPUWrite, "Access mode must have EA_GPUWrite flag");

	D3D12_DEPTH_STENCIL_VIEW_DESC desc;

	desc.Format = Convert(format);
	desc.Flags = D3D12_DSV_FLAG_NONE;

	if (sample_info.Count == 1) {
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		/*
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.ArraySize = num_slices;
		desc.Texture2DArray.FirstArraySlice = first_slice;
		*/
	}
	else {
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
		/*
		desc.Texture2DMSArray.ArraySize = num_slices;
		desc.Texture2DMSArray.FirstArraySlice = first_slice;
		*/
	}
	desc.Texture2DArray.MipSlice = level;
	desc.Texture2DArray.ArraySize = num_slices;
	desc.Texture2DArray.FirstArraySlice = first_slice;

	return RetriveDSV(desc);
}



