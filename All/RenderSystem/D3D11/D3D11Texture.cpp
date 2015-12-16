#include "D3D11Texture.hpp"
#include <id.hpp>

#include "..\..\DeviceMgr.h"

namespace leo {
	auto device = [] {return DeviceMgr().GetDevice(); };


	EFormat Format(DXGI_FORMAT format) {
		switch (format)
		{
		case DXGI_FORMAT_A8_UNORM:
			return EF_A8;

		case DXGI_FORMAT_B5G6R5_UNORM:
			return EF_R5G6B5;

		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return EF_A1RGB5;

		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return EF_ARGB4;

		case DXGI_FORMAT_R8_UNORM:
			return EF_R8;

		case DXGI_FORMAT_R8_SNORM:
			return EF_SIGNED_R8;

		case DXGI_FORMAT_R8G8_UNORM:
			return EF_GR8;

		case DXGI_FORMAT_R8G8_SNORM:
			return EF_SIGNED_GR8;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return EF_ARGB8;

		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return EF_ABGR8;

		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return EF_SIGNED_ABGR8;

		case DXGI_FORMAT_R10G10B10A2_UNORM:
			return EF_A2BGR10;

		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			return EF_SIGNED_A2BGR10;

		case DXGI_FORMAT_R8_UINT:
			return EF_R8UI;

		case DXGI_FORMAT_R8_SINT:
			return EF_R8I;

		case DXGI_FORMAT_R8G8_UINT:
			return EF_GR8UI;

		case DXGI_FORMAT_R8G8_SINT:
			return EF_GR8I;

		case DXGI_FORMAT_R8G8B8A8_UINT:
			return EF_ABGR8UI;

		case DXGI_FORMAT_R8G8B8A8_SINT:
			return EF_ABGR8I;

		case DXGI_FORMAT_R10G10B10A2_UINT:
			return EF_A2BGR10UI;

		case DXGI_FORMAT_R16_UNORM:
			return EF_R16;

		case DXGI_FORMAT_R16_SNORM:
			return EF_SIGNED_R16;

		case DXGI_FORMAT_R16G16_UNORM:
			return EF_GR16;

		case DXGI_FORMAT_R16G16_SNORM:
			return EF_SIGNED_GR16;

		case DXGI_FORMAT_R16G16B16A16_UNORM:
			return EF_ABGR16;

		case DXGI_FORMAT_R16G16B16A16_SNORM:
			return EF_SIGNED_ABGR16;

		case DXGI_FORMAT_R16_UINT:
			return EF_R16UI;

		case DXGI_FORMAT_R16_SINT:
			return EF_R16I;

		case DXGI_FORMAT_R16G16_UINT:
			return EF_GR16UI;

		case DXGI_FORMAT_R16G16_SINT:
			return EF_GR16I;

		case DXGI_FORMAT_R16G16B16A16_UINT:
			return EF_ABGR16UI;

		case DXGI_FORMAT_R16G16B16A16_SINT:
			return EF_ABGR16I;

		case DXGI_FORMAT_R32_UINT:
			return EF_R32UI;

		case DXGI_FORMAT_R32_SINT:
			return EF_R32I;

		case DXGI_FORMAT_R32G32_UINT:
			return EF_GR32UI;

		case DXGI_FORMAT_R32G32_SINT:
			return EF_GR32I;

		case DXGI_FORMAT_R32G32B32_UINT:
			return EF_BGR32UI;

		case DXGI_FORMAT_R32G32B32_SINT:
			return EF_BGR32I;

		case DXGI_FORMAT_R32G32B32A32_UINT:
			return EF_ABGR32UI;

		case DXGI_FORMAT_R32G32B32A32_SINT:
			return EF_ABGR32I;

		case DXGI_FORMAT_R16_FLOAT:
			return EF_R16F;

		case DXGI_FORMAT_R16G16_FLOAT:
			return EF_GR16F;

		case DXGI_FORMAT_R11G11B10_FLOAT:
			return EF_B10G11R11F;

		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return EF_ABGR16F;

		case DXGI_FORMAT_R32_FLOAT:
			return EF_R32F;

		case DXGI_FORMAT_R32G32_FLOAT:
			return EF_GR32F;

		case DXGI_FORMAT_R32G32B32_FLOAT:
			return EF_BGR32F;

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return EF_ABGR32F;

		case DXGI_FORMAT_BC1_UNORM:
			return EF_BC1;

		case DXGI_FORMAT_BC2_UNORM:
			return EF_BC2;

		case DXGI_FORMAT_BC3_UNORM:
			return EF_BC3;

		case DXGI_FORMAT_BC4_UNORM:
			return EF_BC4;

		case DXGI_FORMAT_BC4_SNORM:
			return EF_SIGNED_BC4;

		case DXGI_FORMAT_BC5_UNORM:
			return EF_BC5;

		case DXGI_FORMAT_BC5_SNORM:
			return EF_SIGNED_BC5;

		case DXGI_FORMAT_BC6H_UF16:
			return EF_BC6;

		case DXGI_FORMAT_BC6H_SF16:
			return EF_SIGNED_BC6;

		case DXGI_FORMAT_BC7_UNORM:
			return EF_BC7;

		case DXGI_FORMAT_D16_UNORM:
			return EF_D16;

		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return EF_D24S8;

		case DXGI_FORMAT_D32_FLOAT:
			return EF_D32F;

		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return EF_ABGR8_SRGB;

		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return EF_BC1_SRGB;

		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return EF_BC2_SRGB;

		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return EF_BC3_SRGB;

		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return EF_BC7_SRGB;

			/*case 0x80000000UL:
				return EF_ETC1;

			case 0x80000001UL:
				return EF_ETC2_R11;

			case 0x80000002UL:
				return EF_SIGNED_ETC2_R11;

			case 0x80000003UL:
				return EF_ETC2_GR11;

			case 0x80000004UL:
				return EF_SIGNED_ETC2_GR11;

			case 0x80000005UL:
				return EF_ETC2_BGR8;

			case 0x80000006UL:
				return EF_ETC2_BGR8_SRGB;

			case 0x80000007UL:
				return EF_ETC2_A1BGR8;

			case 0x80000008UL:
				return EF_ETC2_A1BGR8_SRGB;

			case 0x80000009UL:
				return EF_ETC2_ABGR8;

			case 0x8000000AUL:
				return EF_ETC2_ABGR8_SRGB;*/

		default:
			throw unsupported();
		}
	}
	DXGI_FORMAT Format(EFormat format) {
		switch (format)
		{
		case EF_A8:
			return DXGI_FORMAT_A8_UNORM;

		case EF_R5G6B5:
			return DXGI_FORMAT_B5G6R5_UNORM;

		case EF_A1RGB5:
			return DXGI_FORMAT_B5G5R5A1_UNORM;

		case EF_ARGB4:
			return DXGI_FORMAT_B4G4R4A4_UNORM;

		case EF_R8:
			return DXGI_FORMAT_R8_UNORM;

		case EF_SIGNED_R8:
			return DXGI_FORMAT_R8_SNORM;

		case EF_GR8:
			return DXGI_FORMAT_R8G8_UNORM;

		case EF_SIGNED_GR8:
			return DXGI_FORMAT_R8G8_SNORM;

		case EF_ARGB8:
		case EF_ARGB8_SRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM;

		case EF_ABGR8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case EF_SIGNED_ABGR8:
			return DXGI_FORMAT_R8G8B8A8_SNORM;

		case EF_A2BGR10:
			return DXGI_FORMAT_R10G10B10A2_UNORM;

		case EF_SIGNED_A2BGR10:
			return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

		case EF_R8UI:
			return DXGI_FORMAT_R8_UINT;

		case EF_R8I:
			return DXGI_FORMAT_R8_SINT;

		case EF_GR8UI:
			return DXGI_FORMAT_R8G8_UINT;

		case EF_GR8I:
			return DXGI_FORMAT_R8G8_SINT;

		case EF_ABGR8UI:
			return DXGI_FORMAT_R8G8B8A8_UINT;

		case EF_ABGR8I:
			return DXGI_FORMAT_R8G8B8A8_SINT;

		case EF_A2BGR10UI:
			return DXGI_FORMAT_R10G10B10A2_UINT;

		case EF_R16:
			return DXGI_FORMAT_R16_UNORM;

		case EF_SIGNED_R16:
			return DXGI_FORMAT_R16_SNORM;

		case EF_GR16:
			return DXGI_FORMAT_R16G16_UNORM;

		case EF_SIGNED_GR16:
			return DXGI_FORMAT_R16G16_SNORM;

		case EF_ABGR16:
			return DXGI_FORMAT_R16G16B16A16_UNORM;

		case EF_SIGNED_ABGR16:
			return DXGI_FORMAT_R16G16B16A16_SNORM;

		case EF_R16UI:
			return DXGI_FORMAT_R16_UINT;

		case EF_R16I:
			return DXGI_FORMAT_R16_SINT;

		case EF_GR16UI:
			return DXGI_FORMAT_R16G16_UINT;

		case EF_GR16I:
			return DXGI_FORMAT_R16G16_SINT;

		case EF_ABGR16UI:
			return DXGI_FORMAT_R16G16B16A16_UINT;

		case EF_ABGR16I:
			return DXGI_FORMAT_R16G16B16A16_SINT;

		case EF_R32UI:
			return DXGI_FORMAT_R32_UINT;

		case EF_R32I:
			return DXGI_FORMAT_R32_SINT;

		case EF_GR32UI:
			return DXGI_FORMAT_R32G32_UINT;

		case EF_GR32I:
			return DXGI_FORMAT_R32G32_SINT;

		case EF_BGR32UI:
			return DXGI_FORMAT_R32G32B32_UINT;

		case EF_BGR32I:
			return DXGI_FORMAT_R32G32B32_SINT;

		case EF_ABGR32UI:
			return DXGI_FORMAT_R32G32B32A32_UINT;

		case EF_ABGR32I:
			return DXGI_FORMAT_R32G32B32A32_SINT;

		case EF_R16F:
			return DXGI_FORMAT_R16_FLOAT;

		case EF_GR16F:
			return DXGI_FORMAT_R16G16_FLOAT;

		case EF_B10G11R11F:
			return DXGI_FORMAT_R11G11B10_FLOAT;

		case EF_ABGR16F:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case EF_R32F:
			return DXGI_FORMAT_R32_FLOAT;

		case EF_GR32F:
			return DXGI_FORMAT_R32G32_FLOAT;

		case EF_BGR32F:
			return DXGI_FORMAT_R32G32B32_FLOAT;

		case EF_ABGR32F:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case EF_BC1:
			return DXGI_FORMAT_BC1_UNORM;

		case EF_BC2:
			return DXGI_FORMAT_BC2_UNORM;

		case EF_BC3:
			return DXGI_FORMAT_BC3_UNORM;

		case EF_BC4:
			return DXGI_FORMAT_BC4_UNORM;

		case EF_SIGNED_BC4:
			return DXGI_FORMAT_BC4_SNORM;

		case EF_BC5:
			return DXGI_FORMAT_BC5_UNORM;

		case EF_SIGNED_BC5:
			return DXGI_FORMAT_BC5_SNORM;

		case EF_BC6:
			return DXGI_FORMAT_BC6H_UF16;

		case EF_SIGNED_BC6:
			return DXGI_FORMAT_BC6H_SF16;

		case EF_BC7:
			return DXGI_FORMAT_BC7_UNORM;

		case EF_D16:
			return DXGI_FORMAT_D16_UNORM;

		case EF_D24S8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;

		case EF_D32F:
			return DXGI_FORMAT_D32_FLOAT;

		case EF_ABGR8_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case EF_BC1_SRGB:
			return DXGI_FORMAT_BC1_UNORM_SRGB;

		case EF_BC2_SRGB:
			return DXGI_FORMAT_BC2_UNORM_SRGB;

		case EF_BC3_SRGB:
			return DXGI_FORMAT_BC3_UNORM_SRGB;

		case EF_BC7_SRGB:
			return DXGI_FORMAT_BC7_UNORM_SRGB;

			/*case EF_ETC1:
				return static_cast<DXGI_FORMAT>(0x80000000UL);

			case EF_ETC2_R11:
				return static_cast<DXGI_FORMAT>(0x80000001UL);

			case EF_SIGNED_ETC2_R11:
				return static_cast<DXGI_FORMAT>(0x80000002UL);

			case EF_ETC2_GR11:
				return static_cast<DXGI_FORMAT>(0x80000003UL);

			case EF_SIGNED_ETC2_GR11:
				return static_cast<DXGI_FORMAT>(0x80000004UL);

			case EF_ETC2_BGR8:
				return static_cast<DXGI_FORMAT>(0x80000005UL);

			case EF_ETC2_BGR8_SRGB:
				return static_cast<DXGI_FORMAT>(0x80000006UL);

			case EF_ETC2_A1BGR8:
				return static_cast<DXGI_FORMAT>(0x80000007UL);

			case EF_ETC2_A1BGR8_SRGB:
				return static_cast<DXGI_FORMAT>(0x80000008UL);

			case EF_ETC2_ABGR8:
				return static_cast<DXGI_FORMAT>(0x80000009UL);

			case EF_ETC2_ABGR8_SRGB:
				return static_cast<DXGI_FORMAT>(0x8000000AUL);
	*/
		default:
			throw unsupported();
		}
	}

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

	ID3D11Resource* D3D11Texture::Resource() const {
		return nullptr;
	}

	ID3D11ShaderResourceView* D3D11Texture::ResourceView() {
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

	template<typename COM, typename DESC, typename MEM_F>
	COM* CreateInMap(D3D11Texture* texture, std::unordered_map<std::size_t,win::unique_com<COM>>& container, MEM_F&& f, DESC const & desc) {
		char const * p = reinterpret_cast<char const *>(&desc);
		auto hash_val = hash(p, p + sizeof(desc));
		auto iter = container.find(hash_val);
		if (iter != container.end())
		{
			return iter->second;
		}
		win::unique_com<COM> ptr;
		std::invoke(f, device(), texture->Resource(), &desc, &ptr);
		auto ret = container.emplace(std::make_pair(hash_val, std::move(ptr)));
		return ret.first->second.get();
	}

	ID3D11ShaderResourceView * D3D11Texture::CreateD3DSRV(D3D11_SHADER_RESOURCE_VIEW_DESC const & desc)
	{
		return CreateInMap(this, d3d_srv_maps, &ID3D11Device::CreateShaderResourceView, desc);
	}

	ID3D11UnorderedAccessView* D3D11Texture::CreateD3DUAV(D3D11_UNORDERED_ACCESS_VIEW_DESC const & desc)
	{
		return CreateInMap(this, d3d_uav_maps, &ID3D11Device::CreateUnorderedAccessView, desc);
	}

	ID3D11RenderTargetView* D3D11Texture::CreateD3DRTV(D3D11_RENDER_TARGET_VIEW_DESC const & desc)
	{
		return CreateInMap(this, d3d_rtv_maps, &ID3D11Device::CreateRenderTargetView, desc);
	}

	ID3D11DepthStencilView* D3D11Texture::CreateD3DDSV(D3D11_DEPTH_STENCIL_VIEW_DESC const & desc)
	{
		return CreateInMap(this, d3d_dsv_maps, &ID3D11Device::CreateDepthStencilView, desc);
	}

	
}