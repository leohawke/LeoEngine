#include "D3D11RenderSystem.hpp"
#include <utility.hpp>

namespace leo {
	namespace D3D11Mapping {
		DXGI_FORMAT MappingFormat(EFormat format)
		{
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

			case EF_ARGB8_SRGB:
				return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

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

			default:
				throw leo::unimplemented();
			}
		}

		D3D11_MAP Mapping(Texture::MapAccess tma, Texture::Dis_Type type, uint32 access, uint8 numMipMaps)
		{
			switch (tma)
			{
			case Texture::MA_RO:
				return D3D11_MAP_READ;

			case Texture::MA_WO:
				if (((EA_C_W | EA_G_R) == access)
					|| ((EA_C_W == access) && (1 == numMipMaps) && (type != Texture::DT_Cube)))
				{
					return D3D11_MAP_WRITE_DISCARD;
				}
				else
				{
					return D3D11_MAP_WRITE;
				}

			case Texture::MA_RW:
				return D3D11_MAP_READ_WRITE;

			default:
				assert(false);
				return D3D11_MAP_READ;
			};
		}
	}
}