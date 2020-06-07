#include "Texture.h"
#include "ResourceView.h"
#include "Context.h"
#include "Convert.h"

using namespace platform_ex::Windows::D3D12;
using BTexture = platform::Render::TextureCube;

TextureCube::TextureCube(uint16 size_,
	uint8 numMipMaps, uint8 array_size_, EFormat format_,
	uint32 access_hint, SampleDesc sample_info)
	:BTexture(numMipMaps, array_size_, format_, access_hint, sample_info),
	Texture(format_),
	size(size_)
{
	if (0 == mipmap_size) {
		mipmap_size = 1;
		auto w = size;
		while (w > 1) {
			++mipmap_size;
			w /= 2;
		}
	}
}

void TextureCube::BuildMipSubLevels(){
	if (IsDepthFormat(format) || IsCompressedFormat(format))
	{
		for (uint8 index = 0; index < GetArraySize(); ++index)
		{
			for (int f = 0; f < 6; ++f)
			{
				CubeFaces const face = static_cast<CubeFaces>(f);
				for (uint8 level = 1; level < GetNumMipMaps(); ++level)
				{
					Resize(*this, { {index, level, 0, 0, GetWidth(level), GetHeight(level) },face },
					{ {index,static_cast<uint8>(level - 1), 0, 0, GetWidth(level - 1), GetHeight(level - 1) }, face
					}, true);
				}
			}
		}
	}
	else
		DoHWBuildMipSubLevels(array_size, mipmap_size, size, size, 6);
}

void TextureCube::HWResourceCreate(ElementInitData const *  init_data) {
	DoCreateHWResource(D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		size, size, 1, array_size * 6, init_data);
}

void TextureCube::HWResourceDelete(){
	return Texture::DeleteHWResource();
}
bool TextureCube::HWResourceReady() const{
	return Texture::ReadyHWResource();
}

uint16 TextureCube::GetWidth(uint8 level) const{
	LAssert(level < mipmap_size, "level out of range");
	return std::max(1, size >> level);
}
uint16 TextureCube::GetHeight(uint8 level) const{
	return GetWidth(level);
}


void TextureCube::Map(TextureMapAccess tma,
	void*& data, uint32& row_pitch, const BoxCube& box){
	auto subres = CalcSubresource(box.level, box.array_index * 6 + box.face - CubeFaces::Positive_X, 0, mipmap_size, array_size);

	uint32_t slice_pitch;
	DoMap(format,subres, tma, box.x_offset, box.y_offset, 0, box.height, 1, data, row_pitch, slice_pitch);
}

void TextureCube::UnMap(const Sub1D& box, TextureCubeFaces face){
	auto subres = CalcSubresource(box.level, box.array_index * 6 + face - CubeFaces::Positive_X, 0, mipmap_size, array_size);
	DoUnmap(subres);
}


ViewSimulation* TextureCube::RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels){
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

	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	desc.TextureCube.MostDetailedMip = first_level;
	desc.TextureCube.MipLevels = num_levels;
	desc.TextureCube.ResourceMinLODClamp = 0;

	return RetriveSRV(desc);
}

ShaderResourceView* TextureCube::RetriveShaderResourceView()
{
	if (!default_srv)
	{
		default_srv.reset(new ShaderResourceView(*this, 0,GetNumMipMaps()));
	}
	return default_srv.get();
}

ViewSimulation* TextureCube::RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level){
	return RetriveUnorderedAccessView(first_array_index, num_items, CubeFaces::Positive_X, 6, level);
}
ViewSimulation* TextureCube::RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, TextureCubeFaces first_face, uint8 num_faces, uint8 level){
	LAssert(GetAccessMode() & EA_GPUUnordered, "Access mode must have EA_GPUUnordered flag");

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;

	desc.Format = dxgi_format;
	desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	desc.Texture2DArray.MipSlice = level;
	desc.Texture2DArray.ArraySize = num_items * 6 + num_faces;
	desc.Texture2DArray.FirstArraySlice = first_array_index * 6 + first_face;
	desc.Texture2DArray.PlaneSlice = 0;

	return RetriveUAV(desc);
}

ViewSimulation* TextureCube::RetriveRenderTargetView(uint8 first_array_index, uint8 array_size, uint8 level){
	LAssert(GetAccessMode() & EA_GPUWrite, "Access mode must have EA_GPUWrite flag");

	D3D12_RENDER_TARGET_VIEW_DESC desc;

	desc.Format = Convert(format);
	if (sample_info.Count > 1)
	{
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
		/*
		desc.Texture2DMSArray.ArraySize = array_size * 6;;
		desc.Texture2DMSArray.FirstArraySlice = first_array_index*6;
		*/
	}
	else
	{
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		/*
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.ArraySize = array_size * 6;
		desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
		desc.Texture2DArray.PlaneSlice = 0;
		*/
	}
	desc.Texture2DArray.MipSlice = level;
	desc.Texture2DArray.ArraySize = array_size * 6;
	desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
	desc.Texture2DArray.PlaneSlice = 0;
	
	return RetriveRTV(desc);
}

ViewSimulation* TextureCube::RetriveDepthStencilView(uint8 first_array_index, uint8 array_size, uint8 level){
	LAssert(GetAccessMode() & EA_GPUWrite, "Access mode must have EA_GPUWrite flag");

	D3D12_DEPTH_STENCIL_VIEW_DESC desc;

	desc.Format = Convert(format);
	desc.Flags = D3D12_DSV_FLAG_NONE;

	if (sample_info.Count > 1)
	{
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
		/*
		desc.Texture2DMSArray.ArraySize = array_size * 6;;
		desc.Texture2DMSArray.FirstArraySlice = first_array_index*6;
		*/
	}
	else
	{
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		/*
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.ArraySize = array_size * 6;
		desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
		desc.Texture2DArray.PlaneSlice = 0;
		*/
	}
	desc.Texture2DArray.MipSlice = level;
	desc.Texture2DArray.ArraySize = array_size * 6;
	desc.Texture2DArray.FirstArraySlice = first_array_index * 6;

	return RetriveDSV(desc);
}

ViewSimulation* TextureCube::RetriveRenderTargetView(uint8 array_index, TextureCubeFaces face, uint8 level){
	LAssert(GetAccessMode() & EA_GPUWrite, "Access mode must have EA_GPUWrite flag");

	D3D12_RENDER_TARGET_VIEW_DESC desc;

	desc.Format = Convert(format);
	if (sample_info.Count > 1)
	{
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
		/*
		desc.Texture2DMSArray.ArraySize = 1;;
		desc.Texture2DMSArray.FirstArraySlice = array_index*6 + face - CubeFaces::Positive_X;
		*/
	}
	else
	{
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		/*
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.ArraySize = 1;
		desc.Texture2DArray.FirstArraySlice = array_index * 6 + face - CubeFaces::Positive_X;
		desc.Texture2DArray.PlaneSlice = 0;
		*/
	}
	desc.Texture2DArray.MipSlice = level;
	desc.Texture2DArray.ArraySize = 1;
	desc.Texture2DArray.FirstArraySlice = array_index * 6 + face - CubeFaces::Positive_X;
	desc.Texture2DArray.PlaneSlice = 0;

	return RetriveRTV(desc);
}

ViewSimulation* TextureCube::RetriveDepthStencilView(uint8 array_index, TextureCubeFaces face, uint8 level){
	LAssert(GetAccessMode() & EA_GPUWrite, "Access mode must have EA_GPUWrite flag");

	D3D12_DEPTH_STENCIL_VIEW_DESC desc;

	desc.Format = Convert(format);
	desc.Flags = D3D12_DSV_FLAG_NONE;

	if (sample_info.Count > 1)
	{
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
		/*
		desc.Texture2DMSArray.ArraySize = 1;;
		desc.Texture2DMSArray.FirstArraySlice = array_index*6 + face - CubeFaces::Positive_X;
		*/
	}
	else
	{
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		/*
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.ArraySize = 1;
		desc.Texture2DArray.FirstArraySlice = array_index * 6 + face - CubeFaces::Positive_X;
		desc.Texture2DArray.PlaneSlice = 0;
		*/
	}
	desc.Texture2DArray.MipSlice = level;
	desc.Texture2DArray.ArraySize = 1;
	desc.Texture2DArray.FirstArraySlice = array_index * 6 + face - CubeFaces::Positive_X;

	return RetriveDSV(desc);
}

ViewSimulation* TextureCube::RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) {
	throw leo::unsupported("TextureCube::RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level)");
}
ViewSimulation* TextureCube::RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) {
	throw leo::unsupported("TextureCube::RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level)");
}
