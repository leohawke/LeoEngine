#include "Texture.h"
#include "RenderView.h"

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
}

void Texture1D::HWResourceCreate(ElementInitData const * init_data)
{
}

void Texture1D::HWResourceDelete()
{
}

bool Texture1D::HWResourceReady() const
{
	return false;
}

uint16 Texture1D::GetWidth(uint8 level) const
{
	return uint16();
}

void Texture1D::Map(TextureMapAccess tma, void *& data,const Box1D& box)
{
}

void Texture1D::UnMap(const Sub1D&)
{
}

ViewSimulation * Texture1D::RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels)
{
	return nullptr;
}

ViewSimulation * Texture1D::RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level)
{
	return nullptr;
}

ViewSimulation * Texture1D::RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level)
{
	return nullptr;
}

ViewSimulation * Texture1D::RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level)
{
	return nullptr;
}
