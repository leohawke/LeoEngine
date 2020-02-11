/*! \file Engine\Asset\DDSX.h
\ingroup Engine
\brief DDS File IO/Infomation ...
*/
#ifndef LE_ASSET_DDS_X_H
#define LE_ASSET_DDS_X_Hs 1

#include <LBase/ldef.h>
#include <LBase/exception.h>
#include <LBase/typeinfo.h>
#include <LBase/id.hpp>

#include "TextureX.h"
#include "Loader.hpp"
#include "CompressionBC.hpp"
#include "../Render/IContext.h"

#include <filesystem>

namespace dds {
	using namespace leo::inttype;
	using namespace platform::Render::IFormat;
	using namespace platform::Render;
	using namespace bc;

	lconstexpr auto header_magic = asset::four_cc_v<'D', 'D', 'S', ' '>;

	enum
	{
		// The surface has alpha channel information in the pixel format.
		DDSPF_ALPHAPIXELS = 0x00000001,

		// The pixel format contains alpha only information
		DDSPF_ALPHA = 0x00000002,

		// The FourCC code is valid.
		DDSPF_FOURCC = 0x00000004,

		// The RGB data in the pixel format structure is valid.
		DDSPF_RGB = 0x00000040,

		// Luminance data in the pixel format is valid.
		// Use this flag for luminance-only or luminance+alpha surfaces,
		// the bit depth is then ddpf.dwLuminanceBitCount.
		DDSPF_LUMINANCE = 0x00020000,

		// Bump map dUdV data in the pixel format is valid.
		DDSPF_BUMPDUDV = 0x00080000
	};

	struct PIXELFORMAT
	{
		uint32	size;				// size of structure
		uint32	flags;				// pixel format flags
		uint32	four_cc;			// (FOURCC code)
		uint32	rgb_bit_count;		// how many bits per pixel
		uint32	r_bit_mask;			// mask for red bit
		uint32	g_bit_mask;			// mask for green bits
		uint32	b_bit_mask;			// mask for blue bits
		uint32	rgb_alpha_bit_mask;	// mask for alpha channels
	};

	enum
	{
		// Indicates a complex surface structure is being described.  A
		// complex surface structure results in the creation of more than
		// one surface.  The additional surfaces are attached to the root
		// surface.  The complex structure can only be destroyed by
		// destroying the root.
		DDSCAPS_COMPLEX = 0x00000008,

		// Indicates that this surface can be used as a 3D texture.  It does not
		// indicate whether or not the surface is being used for that purpose.
		DDSCAPS_TEXTURE = 0x00001000,

		// Indicates surface is one level of a mip-map. This surface will
		// be attached to other DDSCAPS_MIPMAP surfaces to form the mip-map.
		// This can be done explicitly, by creating a number of surfaces and
		// attaching them with AddAttachedSurface or by implicitly by CreateSurface.
		// If this bit is set then DDSCAPS_TEXTURE must also be set.
		DDSCAPS_MIPMAP = 0x00400000,
	};

	enum
	{
		// This flag is used at CreateSurface time to indicate that this set of
		// surfaces is a cubic environment map
		DDSCAPS2_CUBEMAP = 0x00000200,

		// These flags preform two functions:
		// - At CreateSurface time, they define which of the six cube faces are
		//   required by the application.
		// - After creation, each face in the cubemap will have exactly one of these
		//   bits set.
		DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400,
		DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800,
		DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000,
		DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000,
		DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000,
		DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000,

		// Indicates that the surface is a volume.
		// Can be combined with DDSCAPS_MIPMAP to indicate a multi-level volume
		DDSCAPS2_VOLUME = 0x00200000,
	};

	struct CAPS2
	{
		uint32	caps1;			// capabilities of surface wanted
		uint32	caps2;
		uint32	reserved[2];
	};

	enum
	{
		DDSD_CAPS = 0x00000001,	// default, dds_caps field is valid.
		DDSD_HEIGHT = 0x00000002,	// height field is valid.
		DDSD_WIDTH = 0x00000004,	// width field is valid.
		DDSD_PITCH = 0x00000008,	// pitch is valid.
		DDSD_PIXELFORMAT = 0x00001000,	// pixel_format is valid.
		DDSD_MIPMAPCOUNT = 0x00020000,	// mip_map_count is valid.
		DDSD_LINEARSIZE = 0x00080000,	// linear_size is valid
		DDSD_DEPTH = 0x00800000,	// depth is valid
	};

	struct SURFACEDESC2
	{
		uint32	size;					// size of the DDSURFACEDESC structure
		uint32	flags;					// determines what fields are valid
		uint32	height;					// height of surface to be created
		uint32	width;					// width of input surface
		union
		{
			int32		pitch;				// distance to start of next line (return value only)
			uint32	linear_size;		// Formless late-allocated optimized surface size
		};
		uint32		depth;				// the depth if this is a volume texture
		uint32		mip_map_count;		// number of mip-map levels requestde
		uint32		reserved1[11];		// reserved
		PIXELFORMAT	pixel_format;		// pixel format description of the surface
		CAPS2		dds_caps;			// direct draw surface capabilities
		uint32		reserved2;
	};

	enum D3D_RESOURCE_DIMENSION
	{
		D3D_RESOURCE_DIMENSION_UNKNOWN = 0,
		D3D_RESOURCE_DIMENSION_BUFFER = 1,
		D3D_RESOURCE_DIMENSION_TEXTURE1D = 2,
		D3D_RESOURCE_DIMENSION_TEXTURE2D = 3,
		D3D_RESOURCE_DIMENSION_TEXTURE3D = 4,
	};

	enum D3D_RESOURCE_MISC_FLAG
	{
		D3D_RESOURCE_MISC_GENERATE_MIPS = 0x1L,
		D3D_RESOURCE_MISC_SHARED = 0x2L,
		D3D_RESOURCE_MISC_TEXTURECUBE = 0x4L,
		D3D_RESOURCE_MISC_SHARED_KEYEDMUTEX = 0x10L,
		D3D_RESOURCE_MISC_GDI_COMPATIBLE = 0x20L
	};

	struct HEADER_DXT10
	{
		uint32 dxgi_format;
		D3D_RESOURCE_DIMENSION resource_dim;
		uint32 misc_flag;
		uint32 array_size;
		uint32 reserved;
	};

#ifndef DXGI_FORMAT_DEFINED
	enum DXGI_FORMAT
	{
		DXGI_FORMAT_UNKNOWN = 0,
		DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
		DXGI_FORMAT_R32G32B32A32_UINT = 3,
		DXGI_FORMAT_R32G32B32A32_SINT = 4,
		DXGI_FORMAT_R32G32B32_TYPELESS = 5,
		DXGI_FORMAT_R32G32B32_FLOAT = 6,
		DXGI_FORMAT_R32G32B32_UINT = 7,
		DXGI_FORMAT_R32G32B32_SINT = 8,
		DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
		DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
		DXGI_FORMAT_R16G16B16A16_UNORM = 11,
		DXGI_FORMAT_R16G16B16A16_UINT = 12,
		DXGI_FORMAT_R16G16B16A16_SNORM = 13,
		DXGI_FORMAT_R16G16B16A16_SINT = 14,
		DXGI_FORMAT_R32G32_TYPELESS = 15,
		DXGI_FORMAT_R32G32_FLOAT = 16,
		DXGI_FORMAT_R32G32_UINT = 17,
		DXGI_FORMAT_R32G32_SINT = 18,
		DXGI_FORMAT_R32G8X24_TYPELESS = 19,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
		DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
		DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
		DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
		DXGI_FORMAT_R10G10B10A2_UNORM = 24,
		DXGI_FORMAT_R10G10B10A2_UINT = 25,
		DXGI_FORMAT_R11G11B10_FLOAT = 26,
		DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
		DXGI_FORMAT_R8G8B8A8_UNORM = 28,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
		DXGI_FORMAT_R8G8B8A8_UINT = 30,
		DXGI_FORMAT_R8G8B8A8_SNORM = 31,
		DXGI_FORMAT_R8G8B8A8_SINT = 32,
		DXGI_FORMAT_R16G16_TYPELESS = 33,
		DXGI_FORMAT_R16G16_FLOAT = 34,
		DXGI_FORMAT_R16G16_UNORM = 35,
		DXGI_FORMAT_R16G16_UINT = 36,
		DXGI_FORMAT_R16G16_SNORM = 37,
		DXGI_FORMAT_R16G16_SINT = 38,
		DXGI_FORMAT_R32_TYPELESS = 39,
		DXGI_FORMAT_D32_FLOAT = 40,
		DXGI_FORMAT_R32_FLOAT = 41,
		DXGI_FORMAT_R32_UINT = 42,
		DXGI_FORMAT_R32_SINT = 43,
		DXGI_FORMAT_R24G8_TYPELESS = 44,
		DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
		DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
		DXGI_FORMAT_R8G8_TYPELESS = 48,
		DXGI_FORMAT_R8G8_UNORM = 49,
		DXGI_FORMAT_R8G8_UINT = 50,
		DXGI_FORMAT_R8G8_SNORM = 51,
		DXGI_FORMAT_R8G8_SINT = 52,
		DXGI_FORMAT_R16_TYPELESS = 53,
		DXGI_FORMAT_R16_FLOAT = 54,
		DXGI_FORMAT_D16_UNORM = 55,
		DXGI_FORMAT_R16_UNORM = 56,
		DXGI_FORMAT_R16_UINT = 57,
		DXGI_FORMAT_R16_SNORM = 58,
		DXGI_FORMAT_R16_SINT = 59,
		DXGI_FORMAT_R8_TYPELESS = 60,
		DXGI_FORMAT_R8_UNORM = 61,
		DXGI_FORMAT_R8_UINT = 62,
		DXGI_FORMAT_R8_SNORM = 63,
		DXGI_FORMAT_R8_SINT = 64,
		DXGI_FORMAT_A8_UNORM = 65,
		DXGI_FORMAT_R1_UNORM = 66,
		DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
		DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
		DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
		DXGI_FORMAT_BC1_TYPELESS = 70,
		DXGI_FORMAT_BC1_UNORM = 71,
		DXGI_FORMAT_BC1_UNORM_SRGB = 72,
		DXGI_FORMAT_BC2_TYPELESS = 73,
		DXGI_FORMAT_BC2_UNORM = 74,
		DXGI_FORMAT_BC2_UNORM_SRGB = 75,
		DXGI_FORMAT_BC3_TYPELESS = 76,
		DXGI_FORMAT_BC3_UNORM = 77,
		DXGI_FORMAT_BC3_UNORM_SRGB = 78,
		DXGI_FORMAT_BC4_TYPELESS = 79,
		DXGI_FORMAT_BC4_UNORM = 80,
		DXGI_FORMAT_BC4_SNORM = 81,
		DXGI_FORMAT_BC5_TYPELESS = 82,
		DXGI_FORMAT_BC5_UNORM = 83,
		DXGI_FORMAT_BC5_SNORM = 84,
		DXGI_FORMAT_B5G6R5_UNORM = 85,
		DXGI_FORMAT_B5G5R5A1_UNORM = 86,
		DXGI_FORMAT_B8G8R8A8_UNORM = 87,
		DXGI_FORMAT_B8G8R8X8_UNORM = 88,
		DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
		DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
		DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
		DXGI_FORMAT_BC6H_TYPELESS = 94,
		DXGI_FORMAT_BC6H_UF16 = 95,
		DXGI_FORMAT_BC6H_SF16 = 96,
		DXGI_FORMAT_BC7_TYPELESS = 97,
		DXGI_FORMAT_BC7_UNORM = 98,
		DXGI_FORMAT_BC7_UNORM_SRGB = 99,
		DXGI_FORMAT_AYUV = 100,
		DXGI_FORMAT_Y410 = 101,
		DXGI_FORMAT_Y416 = 102,
		DXGI_FORMAT_NV12 = 103,
		DXGI_FORMAT_P010 = 104,
		DXGI_FORMAT_P016 = 105,
		DXGI_FORMAT_420_OPAQUE = 106,
		DXGI_FORMAT_YUY2 = 107,
		DXGI_FORMAT_Y210 = 108,
		DXGI_FORMAT_Y216 = 109,
		DXGI_FORMAT_NV11 = 110,
		DXGI_FORMAT_AI44 = 111,
		DXGI_FORMAT_IA44 = 112,
		DXGI_FORMAT_P8 = 113,
		DXGI_FORMAT_A8P8 = 114,
		DXGI_FORMAT_B4G4R4A4_UNORM = 115,
		DXGI_FORMAT_FORCE_UINT = 0xffffffff
	};
#endif

	inline EFormat FromDXGIFormat(uint32_t format)
	{
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

			// My extensions for ETC

		case 0x80000000UL:
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
			return EF_ETC2_ABGR8_SRGB;

		default:
			throw leo::unimplemented();
		}
	}

	inline EFormat Convert(const SURFACEDESC2& desc, const HEADER_DXT10& desc10) {
		EFormat format = EF_ARGB8;
		if (desc.pixel_format.flags & DDSPF_FOURCC) {
			// From "Programming Guide for DDS", http://msdn.microsoft.com/en-us/library/bb943991.aspx
			switch (desc.pixel_format.four_cc)
			{
			case 36:
				format = EF_ABGR16;
				break;

			case 110:
				format = EF_SIGNED_ABGR16;
				break;

			case 111:
				format = EF_R16F;
				break;

			case 112:
				format = EF_GR16F;
				break;

			case 113:
				format = EF_ABGR16F;
				break;

			case 114:
				format = EF_R32F;
				break;

			case 115:
				format = EF_GR32F;
				break;

			case 116:
				format = EF_ABGR32F;
				break;

			case asset::four_cc<'D', 'X', 'T', '1'>::value:
				format = EF_BC1;
				break;

			case asset::four_cc<'D', 'X', 'T', '3'>::value:
				format = EF_BC2;
				break;

			case asset::four_cc<'D', 'X', 'T', '5'>::value:
				format = EF_BC3;
				break;

			case asset::four_cc<'B', 'C', '4', 'U'>::value:
			case asset::four_cc<'A', 'T', 'I', '1'>::value:
				format = EF_BC4;
				break;

			case asset::four_cc<'B', 'C', '4', 'S'>::value:
				format = EF_SIGNED_BC4;
				break;

			case asset::four_cc<'B', 'C', '5', 'U'>::value:
			case asset::four_cc<'A', 'T', 'I', '2'>::value:
				format = EF_BC5;
				break;

			case asset::four_cc<'B', 'C', '5', 'S'>::value:
				format = EF_SIGNED_BC5;
				break;

			case asset::four_cc<'D', 'X', '1', '0'>::value:
				format = FromDXGIFormat(desc10.dxgi_format);
				break;
			}
		}
		else {
			if ((desc.pixel_format.flags & DDSPF_RGB) != 0)
			{
				switch (desc.pixel_format.rgb_bit_count)
				{
				case 16:
					if ((0xF000 == desc.pixel_format.rgb_alpha_bit_mask)
						&& (0x0F00 == desc.pixel_format.r_bit_mask)
						&& (0x00F0 == desc.pixel_format.g_bit_mask)
						&& (0x000F == desc.pixel_format.b_bit_mask))
					{
						format = EF_ARGB4;
					}
					else
					{
						lassume(false);
					}
					break;

				case 32:
					if ((0x00FF0000 == desc.pixel_format.r_bit_mask)
						&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
						&& (0x000000FF == desc.pixel_format.b_bit_mask))
					{
						format = EF_ARGB8;
					}
					else
					{
						if ((0xC0000000 == desc.pixel_format.rgb_alpha_bit_mask)
							&& (0x000003FF == desc.pixel_format.r_bit_mask)
							&& (0x000FFC00 == desc.pixel_format.g_bit_mask)
							&& (0x3FF00000 == desc.pixel_format.b_bit_mask))
						{
							format = EF_A2BGR10;
						}
						else
						{
							if ((0xFF000000 == desc.pixel_format.rgb_alpha_bit_mask)
								&& (0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
								&& (0x00FF0000 == desc.pixel_format.b_bit_mask))
							{
								format = EF_ABGR8;
							}
							else
							{
								if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask)
									&& (0x0000FFFF == desc.pixel_format.r_bit_mask)
									&& (0xFFFF0000 == desc.pixel_format.g_bit_mask)
									&& (0x00000000 == desc.pixel_format.b_bit_mask))
								{
									format = EF_GR16;
								}
								else
								{
									lassume(false);
								}
							}
						}
					}
					break;

				default:
					lassume(false);
					break;
				}
			}
			else
			{
				if ((desc.pixel_format.flags & DDSPF_LUMINANCE) != 0)
				{
					switch (desc.pixel_format.rgb_bit_count)
					{
					case 8:
						if (0 == (desc.pixel_format.flags & DDSPF_ALPHAPIXELS))
						{
							format = EF_R8;
						}
						else
						{
							lassume(false);
						}
						break;

					case 16:
						if (0 == (desc.pixel_format.flags & DDSPF_ALPHAPIXELS))
						{
							format = EF_R16;
						}
						else
						{
							lassume(false);
						}
						break;

					default:
						lassume(false);
						break;
					}
				}
				else
				{
					if ((desc.pixel_format.flags & DDSPF_BUMPDUDV) != 0)
					{
						switch (desc.pixel_format.rgb_bit_count)
						{
						case 16:
							if ((0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask))
							{
								format = EF_SIGNED_GR8;
							}
							else
							{
								if (0x0000FFFF == desc.pixel_format.r_bit_mask)
								{
									format = EF_SIGNED_R16;
								}
								else
								{
									lassume(false);
								}
							}
							break;

						case 32:
							if ((0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
								&& (0x00FF0000 == desc.pixel_format.b_bit_mask))
							{
								format = EF_SIGNED_ABGR8;
							}
							else
							{
								if ((0xC0000000 == desc.pixel_format.rgb_alpha_bit_mask)
									&& (0x000003FF == desc.pixel_format.r_bit_mask)
									&& (0x000FFC00 == desc.pixel_format.g_bit_mask)
									&& (0x3FF00000 == desc.pixel_format.b_bit_mask))
								{
									format = EF_SIGNED_A2BGR10;
								}
								else
								{
									if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask)
										&& (0x0000FFFF == desc.pixel_format.r_bit_mask)
										&& (0xFFFF0000 == desc.pixel_format.g_bit_mask)
										&& (0x00000000 == desc.pixel_format.b_bit_mask))
									{
										format = EF_SIGNED_GR16;
									}
									else
									{
										lassume(false);
									}
								}
							}
							break;

						default:
							lassume(false);
							break;
						}
					}
					else
					{
						if ((desc.pixel_format.flags & DDSPF_ALPHA) != 0)
						{
							format = EF_A8;
						}
						else
						{
							lassume(false);
						}
					}
				}
			}
		}
		return format;
	}

	class DDSAsset :leo::noncopyable {
	public:
		DDSAsset() = default;

		DefGetter(const lnothrow, platform::Render::TextureType, TextureType, type)
			DefGetter(lnothrow, platform::Render::TextureType&, TextureTypeRef, type)

			DefGetter(const lnothrow, uint16, Width, width)
			DefGetter(lnothrow, uint16&, WidthRef, width)

			DefGetter(const lnothrow, uint16, Height, height)
			DefGetter(lnothrow, uint16&, HeightRef, height)

			DefGetter(const lnothrow, uint16, Depth, depth)
			DefGetter(lnothrow, uint16&, DepthRef, depth)

			DefGetter(const lnothrow, uint8, MipmapSize, mipmap_size)
			DefGetter(lnothrow, uint8&, MipmapSizeRef, mipmap_size)

			DefGetter(const lnothrow, uint8, ArraySize, array_size)
			DefGetter(lnothrow, uint8&, ArraySizeRef, array_size)

			DefGetter(const lnothrow, EFormat, Format, format)
			DefGetter(lnothrow, EFormat&, FormatRef, format)

			DefGetter(const lnothrow, const std::vector<ElementInitData>&, ElementInitDatas, init_data)
			DefGetter(lnothrow, std::vector<ElementInitData>&, ElementInitDatasRef, init_data)

			DefGetter(const lnothrow, const std::vector<uint8>&, DataBlock, data_block)
			DefGetter(lnothrow, std::vector<uint8>&, DataBlockRef, data_block)

	private:
		platform::Render::TextureType type;
		uint16 width, height, depth;
		uint8  mipmap_size, array_size;
		EFormat format;
		std::vector<ElementInitData> init_data;
		std::vector<uint8> data_block;
	};

	class DDSLoadingDesc : public asset::AssetLoading<DDSAsset> {
	private:
		using path = std::filesystem::path;
		struct DDSDesc {
			platform::File file;
			path dds_path;

			std::shared_ptr<AssetType> dds_asset;
		} dds_desc;

		static EFormat convert_fmts[][2];
	public:
		DDSLoadingDesc(path const& texpath) {
			dds_desc.dds_path = texpath;
			dds_desc.file = platform::File{ texpath.wstring(), platform::File::kToRead };

			if (!dds_desc.file.IsOpen())
				throw;
		}

		std::size_t Type() const override {
			return leo::type_id<DDSLoadingDesc>().hash_code();
		}

		std::size_t Hash() const override {
			return leo::hash_combine_seq(Type(), dds_desc.dds_path.wstring());
		}

		const path& Path() const override {
			return dds_desc.dds_path;
		}

		std::experimental::generator<std::shared_ptr<AssetType>> Coroutine() override {
			co_yield ReadDDS();
			co_yield ConvertFormat();
		}

	private:
		std::shared_ptr<AssetType> ReadDDS() {
			auto pAsset = dds_desc.dds_asset = std::make_shared<AssetType>();
			{
				platform::X::GetImageInfo(dds_desc.file, pAsset->GetTextureTypeRef(),
					pAsset->GetWidthRef(), pAsset->GetHeightRef(), pAsset->GetDepthRef(), pAsset->GetMipmapSizeRef(), pAsset->GetArraySizeRef(),
					pAsset->GetFormatRef(), pAsset->GetElementInitDatasRef(), pAsset->GetDataBlockRef());
			}
			return nullptr;
		}

		std::shared_ptr<AssetType> ConvertFormat() {
			auto pAsset = dds_desc.dds_asset;
			auto caps = Context::Instance().GetDevice().GetCaps();
			if ((TextureType::T_3D == pAsset->GetTextureType()) && (caps.max_texture_depth < pAsset->GetDepth()))
			{
				pAsset->GetTextureTypeRef() = TextureType::T_2D;
				pAsset->GetHeightRef() *= pAsset->GetDepth();
				pAsset->GetDepthRef() = 1;
				pAsset->GetMipmapSizeRef() = 1;
				pAsset->GetElementInitDatasRef().resize(1);
			}

			uint32_t array_size = pAsset->GetArraySize();
			if (TextureType::T_Cube == pAsset->GetTextureType())
			{
				array_size *= 6;
			}

			if (((EF_BC5 == pAsset->GetFormat()) && !caps.TextureFormatSupport(EF_BC5))
				|| ((EF_BC5_SRGB == pAsset->GetFormat()) && !caps.TextureFormatSupport(EF_BC5_SRGB)))
			{
				BC1Block tmp;
				for (size_t i = 0; i < pAsset->GetElementInitDatas().size(); ++i)
				{
					for (size_t j = 0; j < pAsset->GetElementInitDatas()[i].slice_pitch; j += sizeof(BC4Block) * 2)
					{
						char* p = static_cast<char*>(const_cast<void*>(pAsset->GetElementInitDatasRef()[i].data)) + j;

						BC4ToBC1G(tmp, *reinterpret_cast<BC4Block const *>(p + sizeof(BC4Block)));
						std::memcpy(p + sizeof(BC4Block), &tmp, sizeof(BC1Block));
					}
				}

				if (IsSRGB(pAsset->GetFormat()))
				{
					pAsset->GetFormatRef() = EF_BC3_SRGB;
				}
				else
				{
					pAsset->GetFormatRef() = EF_BC3;
				}
			}
			if (((EF_BC4 == pAsset->GetFormat()) && !caps.TextureFormatSupport(EF_BC4))
				|| ((EF_BC4_SRGB == pAsset->GetFormat()) && !caps.TextureFormatSupport(EF_BC4_SRGB)))
			{
				BC1Block tmp;
				for (size_t i = 0; i < pAsset->GetElementInitDatas().size(); ++i)
				{
					for (size_t j = 0; j < pAsset->GetElementInitDatas()[i].slice_pitch; j += sizeof(BC4Block))
					{
						char* p = static_cast<char*>(const_cast<void*>(pAsset->GetElementInitDatasRef()[i].data)) + j;

						BC4ToBC1G(tmp, *reinterpret_cast<BC4Block const *>(p));
						std::memcpy(p, &tmp, sizeof(BC1Block));
					}
				}

				if (IsSRGB(pAsset->GetFormat()))
				{
					pAsset->GetFormatRef() = EF_BC1_SRGB;
				}
				else
				{
					pAsset->GetFormatRef() = EF_BC1;
				}
			}

			while (!caps.TextureFormatSupport(pAsset->GetFormat()))
			{
				bool found = false;
				for (size_t i = 0; i != 27; ++i)
				{
					if (convert_fmts[i][0] == pAsset->GetFormat())
					{
						uint32_t const src_elem_size = NumFormatBytes(convert_fmts[i][0]);
						uint32_t const dst_elem_size = NumFormatBytes(convert_fmts[i][1]);

						bool needs_new_data_block = (src_elem_size < dst_elem_size)
							|| (IsCompressedFormat(convert_fmts[i][0]) && !IsCompressedFormat(convert_fmts[i][1]));

						std::vector<uint8_t> new_data_block;
						std::vector<uint32_t> new_sub_res_start;
						if (needs_new_data_block)
						{
							uint32_t new_data_block_size = 0;
							new_sub_res_start.resize(array_size * pAsset->GetMipmapSize());

							for (size_t index = 0; index < array_size; ++index)
							{
								uint32_t width = pAsset->GetWidth();
								uint32_t height = pAsset->GetHeight();
								for (size_t level = 0; level < pAsset->GetMipmapSize(); ++level)
								{
									uint32_t slice_pitch;
									if (IsCompressedFormat(convert_fmts[i][1]))
									{
										slice_pitch = ((width + 3) & ~3) * (height + 3) / 4 * dst_elem_size;
									}
									else
									{
										slice_pitch = width * height * dst_elem_size;
									}

									size_t sub_res = index * pAsset->GetMipmapSize() + level;
									new_sub_res_start[sub_res] = new_data_block_size;
									new_data_block_size += slice_pitch;

									width = std::max<uint32_t>(1U, width / 2);
									height = std::max<uint32_t>(1U, height / 2);
								}
							}

							new_data_block.resize(new_data_block_size);
						}

						for (size_t index = 0; index < array_size; ++index)
						{
							uint32_t width = pAsset->GetWidth();
							uint32_t height = pAsset->GetHeight();
							uint32_t depth = pAsset->GetDepth();
							for (size_t level = 0; level < pAsset->GetMipmapSize(); ++level)
							{
								uint32_t row_pitch, slice_pitch;
								if (IsCompressedFormat(convert_fmts[i][1]))
								{
									row_pitch = ((width + 3) & ~3) * dst_elem_size;
									slice_pitch = (height + 3) / 4 * row_pitch;
								}
								else
								{
									row_pitch = width * dst_elem_size;
									slice_pitch = height * row_pitch;
								}

								size_t sub_res = index * pAsset->GetMipmapSize() + level;
								uint8_t* sub_data_block;
								if (needs_new_data_block)
								{
									sub_data_block = &new_data_block[new_sub_res_start[sub_res]];
								}
								else
								{
									sub_data_block = static_cast<uint8_t*>(
										const_cast<void*>(pAsset->GetElementInitDatas()[sub_res].data));
								}
								platform::X::ResizeTexture(sub_data_block, row_pitch, slice_pitch,
									convert_fmts[i][1], width, height, depth,
									pAsset->GetElementInitDatas()[sub_res].data,
									pAsset->GetElementInitDatas()[sub_res].row_pitch,
									pAsset->GetElementInitDatas()[sub_res].slice_pitch,
									convert_fmts[i][0], width, height, depth, false);

								width = std::max<uint32_t>(1U, width / 2);
								height = std::max<uint32_t>(1U, height / 2);
								depth = std::max<uint32_t>(1U, depth / 2);

								pAsset->GetElementInitDatasRef()[sub_res].row_pitch = row_pitch;
								pAsset->GetElementInitDatasRef()[sub_res].slice_pitch = slice_pitch;
								pAsset->GetElementInitDatasRef()[sub_res].data = sub_data_block;
							}
						}

						if (needs_new_data_block)
						{
							pAsset->GetDataBlockRef().swap(new_data_block);
						}

						pAsset->GetFormatRef() = convert_fmts[i][1];
						found = true;
						break;
					}
				}

				if (!found)
				{
					LE_LogError("format (%ld) is not supported.", pAsset->GetFormat());
					break;
				}
			}

			return pAsset;
		}
	};
}


#endif
