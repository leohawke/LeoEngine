#include "TextureX.h"
#include "DDSX.h"

#include "CompressionBC.hpp"
#include "CompressionETC.hpp"

#include "../Render/Color_T.hpp"

#include <LFramework/LCLib/Debug.h>

namespace platform {
	using Render::TextureType;
	void X::GetImageInfo(File const & file, Render::TextureType & type, 
		uint16 & width, uint16 & height, uint16 & depth, 
		uint8 & num_mipmaps, uint8 & array_size, Render::EFormat & format, 
		uint32 & row_pitch, uint32 & slice_pitch)
	{
		FileRead tex_res{ file };

		uint32 magic;
		tex_res.Read(&magic, sizeof(magic));
		lassume(dds::header_magic == magic);

		dds::SURFACEDESC2 desc;
		tex_res.Read(&desc, sizeof(desc));

		dds::HEADER_DXT10 desc10;
		if (four_cc_v<'D', 'X', '1', '0'> == desc.pixel_format.four_cc) {
			tex_res.Read(&desc10, sizeof(desc10));
			array_size = desc10.array_size;
		}
		else {
			std::memset(&desc10, 0, sizeof(desc10));
			array_size = 1;
			lassume(desc.flags & dds::DDSD_CAPS);
			lassume(desc.flags & dds::DDSD_PIXELFORMAT);
		}

		lassume(desc.flags & dds::DDSD_WIDTH);
		lassume(desc.flags & dds::DDSD_HEIGHT);

		if (0 == (desc.flags & dds::DDSD_MIPMAPCOUNT))
			desc.mip_map_count = 1;

		format = dds::Convert(desc,desc10);

		if (desc.flags & dds::DDSD_PITCH)
			row_pitch = desc.pitch;
		else if (desc.flags & desc.pixel_format.flags & 0X00000040)
			row_pitch = desc.width * desc.pixel_format.rgb_bit_count / 8;
		else
			row_pitch = desc.width * NumFormatBytes(format);

		slice_pitch = row_pitch * desc.height;

		if (desc.reserved1[0])
			format = MakeSRGB(format);

		width = desc.width;
		num_mipmaps = desc.mip_map_count;


		if ((four_cc<'D', 'X', '1', '0'>::value == desc.pixel_format.four_cc))
		{
			if (dds::D3D_RESOURCE_MISC_TEXTURECUBE == desc10.misc_flag)
			{
				type = TextureType::T_Cube;
				array_size /= 6;
				height = desc.width;
				depth = 1;
			}
			else
			{
				switch (desc10.resource_dim)
				{
				case dds::D3D_RESOURCE_DIMENSION_TEXTURE1D:
					type = TextureType::T_1D;
					height = 1;
					depth = 1;
					break;

				case dds::D3D_RESOURCE_DIMENSION_TEXTURE2D:
					type = TextureType::T_2D;
					height = desc.height;
					depth = 1;
					break;

				case dds::D3D_RESOURCE_DIMENSION_TEXTURE3D:
					type = TextureType::T_3D;
					height = desc.height;
					depth = desc.depth;
					break;

				default:
					lassume(false);
					break;
				}
			}
		}
		else
		{
			if ((desc.dds_caps.caps2 & dds::DDSCAPS2_CUBEMAP) != 0)
			{
				type = TextureType::T_Cube;
				height = desc.width;
				depth = 1;
			}
			else
			{
				if ((desc.dds_caps.caps2 & dds::DDSCAPS2_VOLUME) != 0)
				{
					type = TextureType::T_3D;
					height = desc.height;
					depth = desc.depth;
				}
				else
				{
					type = TextureType::T_2D;
					height = desc.height;
					depth = 1;
				}
			}
		}
	}

	void X::GetImageInfo(File const & file, Render::TextureType& type,
		uint16& width, uint16& height, uint16& depth,
		uint8& num_mipmaps, uint8& array_size,Render::EFormat& format,
		std::vector<Render::ElementInitData> & init_data,
		std::vector<uint8>& data_block)
	{}


	Render::TexturePtr LoadDDSTexture(File && file, uint32 access) {
		return asset::SyncLoad<dds::DDSLoadingDesc>(std::move(file), access);
	}

	Render::TexturePtr X::LoadTexture(X::path const& texpath, uint32 access) {
		return LoadDDSTexture({ texpath.wstring(), platform::File::kToRead }, access);
	}


	using namespace Render::IFormat;
	using namespace bc;
	using namespace etc;
	void EncodeTexture(void* dst_data, uint32_t dst_row_pitch, uint32_t dst_slice_pitch, EFormat dst_format,
		void const * src_data, uint32_t src_row_pitch, uint32_t src_slice_pitch, EFormat src_format,
		uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		LAssert(IsCompressedFormat(dst_format) && !IsCompressedFormat(src_format),"format does not meet the compress requirements");

		TexCompressionPtr codec;
		switch (dst_format)
		{
		case EF_BC1:
		case EF_BC1_SRGB:
		case EF_SIGNED_BC1:
			codec = std::make_shared<TexCompressionBC1>();
			break;

		case EF_BC2:
		case EF_BC2_SRGB:
		case EF_SIGNED_BC2:
			codec = std::make_shared<TexCompressionBC2>();
			break;

		case EF_BC3:
		case EF_BC3_SRGB:
		case EF_SIGNED_BC3:
			codec = std::make_shared<TexCompressionBC3>();
			break;

		case EF_BC4:
		case EF_BC4_SRGB:
		case EF_SIGNED_BC4:
			codec = std::make_shared<TexCompressionBC4>();
			break;

		case EF_BC5:
		case EF_BC5_SRGB:
		case EF_SIGNED_BC5:
			codec = std::make_shared<TexCompressionBC5>();
			break;

		case EF_BC6:
			codec = std::make_shared<TexCompressionBC6U>();
			break;

		case EF_SIGNED_BC6:
			codec = std::make_shared<TexCompressionBC6S>();
			break;

		case EF_BC7:
		case EF_BC7_SRGB:
			codec = std::make_shared<TexCompressionBC7>();
			break;

		case EF_ETC1:
			codec = std::make_shared<TexCompressionETC1>();
			break;

		case EF_ETC2_BGR8:
		case EF_ETC2_BGR8_SRGB:
			codec = std::make_shared<TexCompressionETC2RGB8>();
			break;

		case EF_ETC2_A1BGR8:
		case EF_ETC2_A1BGR8_SRGB:
			codec = std::make_shared<TexCompressionETC2RGB8A1>();
			break;

		case EF_ETC2_ABGR8:
		case EF_ETC2_ABGR8_SRGB:
			// TODO
			throw leo::unimplemented(false);
			break;

		case EF_ETC2_R11:
		case EF_SIGNED_ETC2_R11:
			// TODO
			throw leo::unimplemented(false);
			break;

		case EF_ETC2_GR11:
		case EF_SIGNED_ETC2_GR11:
			// TODO
			throw leo::unimplemented(false);
			break;

		default:
			LAssert(false,"format out of range");
			break;
		}

		uint8_t const * src = static_cast<uint8_t const *>(src_data);
		uint8_t* dst = static_cast<uint8_t*>(dst_data);
		for (uint32_t z = 0; z < src_depth; ++z)
		{
			codec->EncodeMem(src_width, src_height, dst, dst_row_pitch, dst_slice_pitch,
				src, src_row_pitch, src_slice_pitch, TCM_Quality);

			src += src_slice_pitch;
			dst += dst_slice_pitch;
		}
	}

	void DecodeTexture(std::vector<uint8_t>& dst_data_block, uint32_t& dst_row_pitch, uint32_t& dst_slice_pitch, EFormat& dst_format,
		void const * src_data, uint32_t src_row_pitch, uint32_t src_slice_pitch, EFormat src_format,
		uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		LAssert(IsCompressedFormat(src_format),"format does not meet the compress requirements");

		switch (src_format)
		{
		case EF_BC1:
		case EF_BC2:
		case EF_BC3:
		case EF_BC7:
		case EF_ETC1:
		case EF_ETC2_BGR8:
		case EF_ETC2_A1BGR8:
		case EF_ETC2_ABGR8:
			dst_format = EF_ARGB8;
			break;

		case EF_BC4:
		case EF_ETC2_R11:
			dst_format = EF_R8;
			break;

		case EF_BC5:
		case EF_ETC2_GR11:
			dst_format = EF_GR8;
			break;

		case EF_SIGNED_BC1:
		case EF_SIGNED_BC2:
		case EF_SIGNED_BC3:
		case EF_SIGNED_ETC2_R11:
			dst_format = EF_SIGNED_ABGR8;
			break;

		case EF_SIGNED_BC4:
			dst_format = EF_SIGNED_R8;
			break;

		case EF_SIGNED_BC5:
			dst_format = EF_SIGNED_GR8;
			break;

		case EF_BC1_SRGB:
		case EF_BC2_SRGB:
		case EF_BC3_SRGB:
		case EF_BC4_SRGB:
		case EF_BC5_SRGB:
		case EF_BC7_SRGB:
		case EF_ETC2_BGR8_SRGB:
		case EF_ETC2_A1BGR8_SRGB:
		case EF_ETC2_ABGR8_SRGB:
			dst_format = EF_ARGB8_SRGB;
			break;

		case EF_BC6:
		case EF_SIGNED_BC6:
			dst_format = EF_ABGR16F;
			break;

		default:
			LAssert(false, "format out of range");
			dst_format = src_format;
			break;
		}

		TexCompressionPtr codec;
		switch (src_format)
		{
		case EF_BC1:
		case EF_BC1_SRGB:
		case EF_SIGNED_BC1:
			codec = std::make_shared<TexCompressionBC1>();
			break;

		case EF_BC2:
		case EF_BC2_SRGB:
		case EF_SIGNED_BC2:
			codec = std::make_shared<TexCompressionBC2>();
			break;

		case EF_BC3:
		case EF_BC3_SRGB:
		case EF_SIGNED_BC3:
			codec = std::make_shared<TexCompressionBC3>();
			break;

		case EF_BC4:
		case EF_BC4_SRGB:
		case EF_SIGNED_BC4:
			codec = std::make_shared<TexCompressionBC4>();
			break;

		case EF_BC5:
		case EF_BC5_SRGB:
		case EF_SIGNED_BC5:
			codec = std::make_shared<TexCompressionBC5>();
			break;

		case EF_BC6:
			codec = std::make_shared<TexCompressionBC6U>();
			break;

		case EF_SIGNED_BC6:
			codec = std::make_shared<TexCompressionBC6S>();
			break;

		case EF_BC7:
		case EF_BC7_SRGB:
			codec = std::make_shared<TexCompressionBC7>();
			break;

		case EF_ETC1:
			codec = std::make_shared<TexCompressionETC1>();
			break;

		case EF_ETC2_BGR8:
		case EF_ETC2_BGR8_SRGB:
			codec = std::make_shared<TexCompressionETC2RGB8>();
			break;

		case EF_ETC2_A1BGR8:
		case EF_ETC2_A1BGR8_SRGB:
			codec = std::make_shared<TexCompressionETC2RGB8A1>();
			break;

		case EF_ETC2_ABGR8:
		case EF_ETC2_ABGR8_SRGB:
			// TODO
			throw leo::unimplemented(false);
			break;

		case EF_ETC2_R11:
		case EF_SIGNED_ETC2_R11:
			// TODO
			throw leo::unimplemented(false);
			break;

		case EF_ETC2_GR11:
		case EF_SIGNED_ETC2_GR11:
			// TODO
			throw leo::unimplemented(false);
			break;

		default:
			throw leo::unimplemented(false);
			break;
		}

		dst_row_pitch = src_width * NumFormatBytes(dst_format);
		dst_slice_pitch = dst_row_pitch * src_height;
		dst_data_block.resize(src_depth * dst_slice_pitch);

		uint8_t const * src = static_cast<uint8_t const *>(src_data);
		uint8_t* dst = static_cast<uint8_t*>(&dst_data_block[0]);
		for (uint32_t z = 0; z < src_depth; ++z)
		{
			codec->DecodeMem(src_width, src_height, dst, dst_row_pitch, dst_slice_pitch,
				src, src_row_pitch, src_slice_pitch);

			src += src_slice_pitch;
			dst += dst_slice_pitch;
		}
	}


	void X::ResizeTexture(void* dst_data, uint32 dst_row_pitch, uint32 dst_slice_pitch, Render::EFormat dst_format,
		uint16 dst_width, uint16 dst_height, uint16 dst_depth,
		void const * src_data, uint32 src_row_pitch, uint32 src_slice_pitch, Render::EFormat src_format,
		uint16 src_width, uint16 src_height, uint16 src_depth,
		bool linear) {
		std::vector<uint8> src_cpu_data_block;
		void* src_cpu_data;
		uint32 src_cpu_row_pitch;
		uint32 src_cpu_slice_pitch;
		Render::EFormat src_cpu_format;
		if (IsCompressedFormat(src_format))
		{
			DecodeTexture(src_cpu_data_block, src_cpu_row_pitch, src_cpu_slice_pitch, src_cpu_format,
				src_data, src_row_pitch, src_slice_pitch, src_format, src_width, src_height, src_depth);
			src_cpu_data = static_cast<void*>(&src_cpu_data_block[0]);
		}
		else
		{
			src_cpu_data = const_cast<void*>(src_data);
			src_cpu_row_pitch = src_row_pitch;
			src_cpu_slice_pitch = src_slice_pitch;
			src_cpu_format = src_format;
		}

		std::vector<uint8> dst_cpu_data_block;
		void* dst_cpu_data;
		uint32 dst_cpu_row_pitch;
		uint32 dst_cpu_slice_pitch;
		Render::EFormat dst_cpu_format;
		if (IsCompressedFormat(dst_format))
		{
			switch (dst_format)
			{
			case EF_BC1:
			case EF_BC2:
			case EF_BC3:
			case EF_BC7:
			case EF_ETC1:
			case EF_ETC2_BGR8:
			case EF_ETC2_A1BGR8:
			case EF_ETC2_ABGR8:
				dst_cpu_format = EF_ARGB8;
				break;

			case EF_BC4:
			case EF_ETC2_R11:
				dst_cpu_format = EF_R8;
				break;

			case EF_BC5:
			case EF_ETC2_GR11:
				dst_cpu_format = EF_GR8;
				break;

			case EF_SIGNED_BC1:
			case EF_SIGNED_BC2:
			case EF_SIGNED_BC3:
				dst_cpu_format = EF_SIGNED_ABGR8;
				break;

			case EF_SIGNED_BC4:
			case EF_SIGNED_ETC2_R11:
				dst_cpu_format = EF_SIGNED_R8;
				break;

			case EF_SIGNED_BC5:
				dst_cpu_format = EF_SIGNED_GR8;
				break;

			case EF_BC1_SRGB:
			case EF_BC2_SRGB:
			case EF_BC3_SRGB:
			case EF_BC4_SRGB:
			case EF_BC5_SRGB:
			case EF_BC7_SRGB:
			case EF_ETC2_BGR8_SRGB:
			case EF_ETC2_A1BGR8_SRGB:
			case EF_ETC2_ABGR8_SRGB:
				dst_cpu_format = EF_ARGB8_SRGB;
				break;

			case EF_BC6:
			case EF_SIGNED_BC6:
				dst_cpu_format = EF_ABGR16F;
				break;

			default:
				LAssert(false,"format out of range");
				dst_cpu_format = src_format;
				break;
			}

			dst_cpu_row_pitch = dst_width * NumFormatBytes(src_cpu_format);
			dst_cpu_slice_pitch = dst_cpu_row_pitch * dst_height;
			dst_cpu_data_block.resize(dst_depth * dst_cpu_slice_pitch);
			dst_cpu_data = &dst_cpu_data_block[0];
		}
		else
		{
			dst_cpu_data = const_cast<void*>(dst_data);
			dst_cpu_row_pitch = dst_row_pitch;
			dst_cpu_slice_pitch = dst_slice_pitch;
			dst_cpu_format = dst_format;
		}

		uint8 const * src_ptr = static_cast<uint8 const *>(src_cpu_data);
		uint8* dst_ptr = static_cast<uint8*>(dst_cpu_data);
		uint32 const src_elem_size = NumFormatBytes(src_cpu_format);
		uint32 const dst_elem_size = NumFormatBytes(dst_cpu_format);

		if (!linear && (src_cpu_format == dst_cpu_format))
		{
			for (uint32 z = 0; z < dst_depth; ++z)
			{
				float fz = static_cast<float>(z) / dst_depth * src_depth;
				uint32 sz = std::min<uint32>(static_cast<uint32>(fz + 0.5f), src_depth - 1);

				for (uint32 y = 0; y < dst_height; ++y)
				{
					float fy = static_cast<float>(y) / dst_height * src_height;
					uint32 sy = std::min<uint32>(static_cast<uint32>(fy + 0.5f), src_height - 1);

					uint8 const * src_p = src_ptr + sz * src_cpu_slice_pitch + sy * src_cpu_row_pitch;
					uint8* dst_p = dst_ptr + z * dst_cpu_slice_pitch + y * dst_cpu_row_pitch;

					if (src_width == dst_width)
					{
						std::memcpy(dst_p, src_p, src_width * src_elem_size);
					}
					else
					{
						for (uint32 x = 0; x < dst_width; ++x, dst_p += dst_elem_size)
						{
							float fx = static_cast<float>(x) / dst_width * src_width;
							uint32 sx = std::min<uint32>(static_cast<uint32>(fx + 0.5f), src_width - 1);
							std::memcpy(dst_p, src_p + sx * src_elem_size, src_elem_size);
						}
					}
				}
			}
		}
		else
		{
			std::vector<M::Color> src_32f(src_width * src_height * src_depth);
			{
				for (uint32 z = 0; z < src_depth; ++z)
				{
					for (uint32 y = 0; y < src_height; ++y)
					{
						M::ConvertToABGR32F(src_cpu_format, src_ptr + z * src_cpu_slice_pitch + y * src_cpu_row_pitch,
							src_width, &src_32f[(z * src_height + y) * src_width]);
					}
				}
			}

			std::vector<M::Color> dst_32f(dst_width * dst_height * dst_depth);
			if (linear)
			{
				for (uint32 z = 0; z < dst_depth; ++z)
				{
					float fz = static_cast<float>(z) / dst_depth * src_depth;
					uint32 sz0 = static_cast<uint32>(fz);
					uint32 sz1 = clamp<uint32>(sz0 + 1, 0, src_depth - 1);
					float weight_z = fz - sz0;

					for (uint32 y = 0; y < dst_height; ++y)
					{
						float fy = static_cast<float>(y) / dst_height * src_height;
						uint32 sy0 = static_cast<uint32>(fy);
						uint32 sy1 = clamp<uint32>(sy0 + 1, 0, src_height - 1);
						float weight_y = fy - sy0;

						for (uint32 x = 0; x < dst_width; ++x)
						{
							float fx = static_cast<float>(x) / dst_width * src_width;
							uint32 sx0 = static_cast<uint32>(fx);
							uint32 sx1 = clamp<uint32>(sx0 + 1, 0, src_width - 1);
							float weight_x = fx - sx0;
							M::Color clr_x00 = lerp(src_32f[(sz0 * src_height + sy0) * src_width + sx0],
								src_32f[(sz0 * src_height + sy0) * src_width + sx1], weight_x);
							M::Color clr_x01 = lerp(src_32f[(sz0 * src_height + sy1) * src_width + sx0],
								src_32f[(sz0 * src_height + sy1) * src_width + sx1], weight_x);
							M::Color clr_y0 = lerp(clr_x00, clr_x01, weight_y);
							M::Color clr_x10 = lerp(src_32f[(sz1 * src_height + sy0) * src_width + sx0],
								src_32f[(sz1 * src_height + sy0) * src_width + sx1], weight_x);
							M::Color clr_x11 = lerp(src_32f[(sz1 * src_height + sy1) * src_width + sx0],
								src_32f[(sz1 * src_height + sy1) * src_width + sx1], weight_x);
							M::Color clr_y1 = lerp(clr_x10, clr_x11, weight_y);
							dst_32f[(z * dst_height + y) * dst_width + x] = lerp(clr_y0, clr_y1, weight_z);
						}
					}
				}
			}
			else
			{
				for (uint32 z = 0; z < dst_depth; ++z)
				{
					float fz = static_cast<float>(z) / dst_depth * src_depth;
					uint32 sz = std::min<uint32>(static_cast<uint32>(fz + 0.5f), src_depth - 1);

					for (uint32 y = 0; y < dst_height; ++y)
					{
						float fy = static_cast<float>(y) / dst_height * src_height;
						uint32 sy = std::min<uint32>(static_cast<uint32>(fy + 0.5f), src_height - 1);

						for (uint32 x = 0; x < dst_width; ++x)
						{
							float fx = static_cast<float>(x) / dst_width * src_width;
							uint32 sx = std::min<uint32>(static_cast<uint32>(fx + 0.5f), src_width - 1);
							dst_32f[(z * dst_height + y) * dst_width + x] = src_32f[(sz * src_height + sy) * src_width + sx];
						}
					}
				}
			}

			{
				for (uint32 z = 0; z < dst_depth; ++z)
				{
					for (uint32 y = 0; y < dst_height; ++y)
					{
						ConvertFromABGR32F(dst_cpu_format, &dst_32f[(z * dst_height + y) * dst_width], dst_width, dst_ptr + z * dst_cpu_slice_pitch + y * dst_cpu_row_pitch);
					}
				}
			}
		}

		if (IsCompressedFormat(dst_format))
		{
			EncodeTexture(dst_data, dst_row_pitch, dst_slice_pitch, dst_format,
				dst_cpu_data, dst_cpu_row_pitch, dst_cpu_slice_pitch, dst_cpu_format,
				dst_width, dst_height, dst_depth);
		}
	}
}
