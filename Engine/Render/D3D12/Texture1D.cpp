#include "Texture.h"
#include "RenderView.h"
#include "Convert.h"

using namespace platform_ex::Windows::D3D12;
using BTexture = platform::Render::Texture1D;

Texture1D::Texture1D(uint16 width_, uint8 numMipMaps, uint8 array_size_, EFormat format_, uint32 access_hint, platform::Render::SampleDesc sample_info)
	:BTexture(numMipMaps, array_size_, format_, access_hint, sample_info),
	Texture(format_),
	width(width_)
{
	if (0 == mipmap_size) {
		mipmap_size = 1;
		auto w = width;
		while (w > 1) {
			++mipmap_size;
			w /= 2;
		}
	}
}

void Texture1D::BuildMipSubLevels()
{
	for (uint8 index = 0; index != GetArraySize(); ++index)
	{
		for (uint8 level = 1; level != GetNumMipMaps(); ++level)
		{
			Resize(*this, { index, level, 0,  GetWidth(level) },
			{ index, static_cast<uint8>(level - 1), 0,GetWidth(level - 1) }, true);
		}
	}
}

void Texture1D::HWResourceCreate(ElementInitData const * init_data)
{
	Texture::DoCreateHWResource(D3D12_RESOURCE_DIMENSION_TEXTURE1D,
		width, 1, 1, array_size,
		init_data);
}

void Texture1D::HWResourceDelete()
{
	return Texture::DeleteHWResource();
}

bool Texture1D::HWResourceReady() const
{
	return Texture::ReadyHWResource();
}

uint16 Texture1D::GetWidth(uint8 level) const
{
	LAssert(level < mipmap_size, "level out of range");
	return std::max(1, width >> level);
}

void Texture1D::Map(TextureMapAccess tma, void *& data,const Box1D& box)
{
	auto subres = CalcSubresource(box.level, box.array_index, 0, mipmap_size, array_size);

	uint32 row_pitch;
	uint32 slice_pitch;
	DoMap(format,subres, tma, box.x_offset, 0, 0, 1, 1, data, row_pitch, slice_pitch);
}

void Texture1D::UnMap(const Sub1D& box)
{
	auto subres = CalcSubresource(box.level, box.array_index, 0, mipmap_size, array_size);
	DoUnmap(subres);
}

ViewSimulation * Texture1D::RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels)
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

	if (GetArraySize() > 1)
	{
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
		desc.Texture1DArray.MostDetailedMip = first_level;
		desc.Texture1DArray.MipLevels = num_levels;
		desc.Texture1DArray.ArraySize = num_items;
		desc.Texture1DArray.FirstArraySlice = first_array_index;
		desc.Texture1DArray.ResourceMinLODClamp = 0;
	}
	else
	{
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MostDetailedMip = first_level;
		desc.Texture1D.MipLevels = num_levels;
		desc.Texture1D.ResourceMinLODClamp = 0;
	}

	return RetriveSRV(desc);
}

ViewSimulation * Texture1D::RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level)
{
	LAssert(GetAccessMode() & EA_GPUUnordered, "Access mode must have EA_GPUUnordered flag");

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;

	desc.Format = dxgi_format;

	if (GetArraySize() > 1) {
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
		desc.Texture1DArray.MipSlice = level;
		desc.Texture1DArray.ArraySize = num_items;
		desc.Texture1DArray.FirstArraySlice = first_array_index;
	}
	else {
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MipSlice = level;
	}

	return RetriveUAV(desc);
}

ViewSimulation * Texture1D::RetriveRenderTargetView(uint8 first_array_index, uint16 first_slice, uint16 num_slices, uint8 level)
{
	LAssert(GetAccessMode() & EA_GPUWrite, "Access mode must have EA_GPUWrite flag");

	D3D12_RENDER_TARGET_VIEW_DESC desc;

	desc.Format = Convert(format);
	if (GetArraySize() > 1) {
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
		desc.Texture1DArray.MipSlice = level;
		desc.Texture1DArray.ArraySize = array_size;
		desc.Texture1DArray.FirstArraySlice = first_array_index;
	}
	else {
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MipSlice = level;
	}

	return RetriveRTV(desc);
}

ViewSimulation * Texture1D::RetriveDepthStencilView(uint8 first_array_index, uint16 first_slice, uint16 num_slices, uint8 level)
{
	LAssert(GetAccessMode() & EA_GPUWrite, "Access mode must have EA_GPUWrite flag");

	D3D12_DEPTH_STENCIL_VIEW_DESC desc;

	desc.Format = Convert(format);
	desc.Flags = D3D12_DSV_FLAG_NONE;

	if (GetArraySize() > 1) {
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
		desc.Texture1DArray.MipSlice = level;
		desc.Texture1DArray.ArraySize = array_size;
		desc.Texture1DArray.FirstArraySlice = first_array_index;
	}
	else {
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MipSlice = level;
	}

	return RetriveDSV(desc);
}
