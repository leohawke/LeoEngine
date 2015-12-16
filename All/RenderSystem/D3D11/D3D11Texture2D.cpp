#include "D3D11Texture.hpp"

leo::D3D11Texture2D::D3D11Texture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access, SampleDesc sample_info, ElementInitData)
	:D3D11Texture(DT_2D,access,sample_info)
{
}

leo::D3D11Texture2D::~D3D11Texture2D()
{
}

leo::uint16 leo::D3D11Texture2D::Width(uint8) const
{
	return uint16();
}

leo::uint16 leo::D3D11Texture2D::Height(uint8) const
{
	return uint16();
}

ID3D11Resource * leo::D3D11Texture2D::Resource() const
{
	return nullptr;
}

ID3D11ShaderResourceView * leo::D3D11Texture2D::ResouceView()
{
	return nullptr;
}

ID3D11UnorderedAccessView * leo::D3D11Texture2D::AccessView()
{
	return nullptr;
}

ID3D11RenderTargetView * leo::D3D11Texture2D::TargetView()
{
	return nullptr;
}

ID3D11DepthStencilView * leo::D3D11Texture2D::DepthStencilView()
{
	return nullptr;
}

ID3D11Texture2D * leo::D3D11Texture2D::D3DTexture() const
{
	return nullptr;
}

void leo::D3D11Texture2D::ReclaimHWResource(ElementInitData const * init_data)
{
}
