#include "D3D11Texture.hpp"

namespace leo {
	D3D11Texture::D3D11Texture(Dis_Type type, uint32 access, SampleDesc sample_info)
		:Texture(type, access, sample_info)
	{
		if (access & EAccess::EA_G_W) {
			LAssert(!(access&EA_C_R), "GPU_Write can't togother with CPU_Read");
			LAssert(!(access&EA_C_W), "GPU_Write can't togother with CPU_Write");
		}
	}
	ImplDeDtor(D3D11Texture)

		std::string const & D3D11Texture::Name() const {
		static const std::string name("Direct3D11 Texture");
		return name;
	}

	uint16 D3D11Texture::Width(uint8 level) const
	{
		LAssert(level < NumMipMaps(), "level out of mipmaps range");
		return 1;
	}

	uint16 D3D11Texture::Height(uint8 level) const
	{
		LAssert(level < NumMipMaps(), "level out of mipmaps range");
		return 1;
	}

	uint16 D3D11Texture::Depth(uint8 level) const
	{
		LAssert(level < NumMipMaps(), "level out of mipmaps range");
		return 1;
	}

	ID3D11Resource* D3D11Texture::Resouce() const {
		return nullptr;
	}

	ID3D11ShaderResourceView* D3D11Texture::ResouceView() {
		return nullptr;
	}
	ID3D11UnorderedAccessView* D3D11Texture::AccessView() {
		return nullptr;
	}
	ID3D11RenderTargetView* D3D11Texture::TargetView() {
		return nullptr;
	}
	ID3D11DepthStencilView* D3D11Texture::DepthStencilView()
	{
		return nullptr;
	}

	ImplDeDtor(D3D11Texture2D)
}