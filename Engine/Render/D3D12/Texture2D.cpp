#include "Texture.h"
#include "Convert.h"
#include "RenderView.h"

using namespace platform_ex::Windows::D3D12;
using BTexture = platform::Render::Texture2D;

Texture2D::Texture2D(uint16 height_, uint16 width_, 
	uint8 numMipMaps, uint8 array_size_, 
	EFormat format_, uint32 access_hint, platform::Render::SampleDesc sample_info)
	:BTexture(numMipMaps,array_size_,format_,access_hint,sample_info),
	Texture(format),
	width(width_),height(height_)
{
	if (0 == mipmap_size) {
		mipmap_size = 1;
		auto w = width;
		auto h = height;
		while ((w > 1) || (h > 1)) {
			++mipmap_size;
			w /= 2;
			h /= 2;
		}
	}
}

void Texture2D::BuildMipSubLevels()
{
	for (auto index = 0; index != GetArraySize(); ++index)
	{
		for (auto level = 1; level != GetNumMipMaps(); ++level)
		{
			Resize(*this, index, level, 0, 0, GetWidth(level), GetHeight(level),
				index, level - 1, 0, 0, GetWidth(level - 1), GetHeight(level - 1), true);
		}
	}

	//TODO GPU Support
	//Wait Effect
}

void Texture2D::HWResourceCreate(ElementInitData const * init_data)
{
	Texture::DoCreateHWResource(D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		width,height,1,array_size,
		init_data);
}

void Texture2D::HWResourceDelete()
{
	return Texture::DeleteHWResource();
}

bool Texture2D::HWResourceReady() const
{
	return Texture::ReadyHWResource();
}

uint16 Texture2D::GetWidth(uint8 level) const
{
	LAssert(level < mipmap_size, "level out of range");
	return std::max(1,width>>level);
}

uint16 Texture2D::GetHeight(uint8 level) const
{
	LAssert(level < mipmap_size, "level out of range");
	return std::max(1, height >> level);
}

void Texture2D::Map(uint8 array_index, uint8 level, TextureMapAccess tma, uint16 x_offset, uint16 y_offset, uint16 width, uint16 height, void *& data, uint32 & row_pitch)
{
	auto subres = CalcSubresource(level, array_index, 0, mipmap_size, array_size);

	uint32 slice_pitch;
	DoMap(format,subres, tma, x_offset, y_offset, 0, height, 1, data, row_pitch, slice_pitch);
}

void Texture2D::UnMap(uint8 array_index, uint8 level)
{
	auto subres = CalcSubresource(level, array_index, 0, mipmap_size, array_size);
	DoUnmap(subres);
}

ViewSimulation * Texture2D::RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels)
{
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
	if (array_size > 1) {
		if (sample_info.Count > 1)
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
		else
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;

		desc.Texture2DArray.MostDetailedMip = first_level;
		desc.Texture2DArray.MipLevels = num_levels;
		desc.Texture2DArray.ArraySize = num_items;
		desc.Texture2DArray.FirstArraySlice = first_array_index;
		desc.Texture2DArray.PlaneSlice = 0;
		desc.Texture2DArray.ResourceMinLODClamp = 0;
	}
	else {
		if (sample_info.Count > 1)
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
		else
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		desc.Texture2D.MostDetailedMip = first_level;
		desc.Texture2D.MipLevels = num_levels;
		desc.Texture2D.PlaneSlice = 0;
		desc.Texture2D.ResourceMinLODClamp = 0;
	}
	return RetriveSRV(desc);
}

ViewSimulation * Texture2D::RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level)
{
	LAssert(GetAccessMode() & EA_GPUUnordered, "Access mode must have EA_GPUUnordered flag");
	
	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;

	desc.Format = dxgi_format;
	desc.Texture2DArray.MipSlice = level;
	desc.Texture2DArray.PlaneSlice = 0;

	if (array_size) {
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = num_items;
		desc.Texture2DArray.FirstArraySlice = first_array_index;
	}
	else {
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	}
	return RetriveUAV(desc);
}

ViewSimulation * Texture2D::RetriveRenderTargetView(uint8 first_array_index, uint16 first_slice, uint16 num_slices, uint8 level)
{
	LAssert(GetAccessMode() & EA_GPUWrite, "Access mode must have EA_GPUWrite flag");

	D3D12_RENDER_TARGET_VIEW_DESC desc;

	desc.Format = Convert(format);

	if (sample_info.Count == 1) {
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = array_size;
		desc.Texture2DArray.FirstArraySlice = first_array_index;
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.PlaneSlice = 0;
	}
	else {
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
		desc.Texture2DMSArray.FirstArraySlice = first_array_index;
		desc.Texture2DMSArray.ArraySize = array_size;
	}

	return RetriveRTV(desc);
}

ViewSimulation * Texture2D::RetriveDepthStencilView(uint8 first_array_index, uint16 first_slice, uint16 num_slices, uint8 level)
{
	LAssert(GetAccessMode() & EA_GPUWrite, "Access mode must have EA_GPUWrite flag");

	D3D12_DEPTH_STENCIL_VIEW_DESC desc;

	desc.Format = Convert(format);
	desc.Flags = D3D12_DSV_FLAG_NONE;

	if (sample_info.Count == 1) {
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.ArraySize = array_size;
		desc.Texture2DArray.FirstArraySlice = first_array_index;
	}
	else {
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
		desc.Texture2DMSArray.ArraySize = array_size;
		desc.Texture2DMSArray.FirstArraySlice = first_array_index;
	}

	return RetriveDSV(desc);
}
