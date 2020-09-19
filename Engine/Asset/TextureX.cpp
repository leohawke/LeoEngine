#include "TextureX.h"
#include "DDSX.h"

#include "CompressionBC.hpp"
#include "CompressionETC.hpp"

#include "../Render/Color_T.hpp"
#include "../Render/IContext.h"

#include "../Core/AssetResourceScheduler.h"

#include <LFramework/LCLib/Debug.h>
#include <LBase/smart_ptr.hpp>
#include <fstream>

namespace platform {
	using namespace Render::IFormat;
	using Render::TextureType;
	using Render::Texture;
	uint64 X::GetImageInfo(File const& file, Render::TextureType& type,
		uint16& width, uint16& height, uint16& depth,
		uint8& num_mipmaps, uint8& array_size, Render::EFormat& format,
		uint32& row_pitch, uint32& slice_pitch)
	{
		FileRead tex_res{ file };

		uint32 magic;
		tex_res.Read(&magic, sizeof(magic));
		lassume(dds::header_magic == magic);

		dds::SURFACEDESC2 desc;
		tex_res.Read(&desc, sizeof(desc));

		dds::HEADER_DXT10 desc10;
		if (asset::four_cc_v<'D', 'X', '1', '0'> == desc.pixel_format.four_cc) {
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

		format = dds::Convert(desc, desc10);

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


		if ((asset::four_cc<'D', 'X', '1', '0'>::value == desc.pixel_format.four_cc))
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

		return tex_res.GetOffset();
	}

	void X::GetImageInfo(File const& file, Render::TextureType& type,
		uint16& width, uint16& height, uint16& depth,
		uint8& num_mipmaps, uint8& array_size, Render::EFormat& format,
		std::vector<Render::ElementInitData>& init_data,
		std::vector<uint8>& data_block)
	{
		uint32 row_pitch, slice_pitch;
		auto offset = GetImageInfo(file, type, width, height, depth, num_mipmaps, array_size, format,
			row_pitch, slice_pitch);

		FileRead tex_res{ file };
		tex_res.SkipTo(offset);

		auto const fmt_size = NumFormatBytes(format);
		bool padding = false;
		if (!IsCompressedFormat(format)) {
			if (row_pitch != width * fmt_size) {
				LAssert(row_pitch == ((width + 3) & ~3) * fmt_size, "padding error");
				padding = true;
			}
		}

		std::vector<size_t> base;
		switch (type)
		{
		case TextureType::T_1D:
		{
			init_data.resize(array_size * num_mipmaps);
			base.resize(array_size * num_mipmaps);
			for (uint32 array_index = 0; array_index < array_size; ++array_index)
			{
				uint32 the_width = width;
				for (uint32 level = 0; level < num_mipmaps; ++level)
				{
					size_t const index = array_index * num_mipmaps + level;
					uint32 image_size;
					if (IsCompressedFormat(format))
					{
						uint32 const block_size = NumFormatBytes(format) * 4;
						image_size = ((the_width + 3) / 4) * block_size;
					}
					else
					{
						image_size = (padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
					}

					base[index] = data_block.size();
					data_block.resize(base[index] + image_size);
					init_data[index].row_pitch = image_size;
					init_data[index].slice_pitch = image_size;

					tex_res.Read(&data_block[base[index]], static_cast<std::streamsize>(image_size));

					the_width = std::max<uint32>(the_width / 2, 1);
				}
			}
		}
		break;

		case TextureType::T_2D:
		{
			init_data.resize(array_size * num_mipmaps);
			base.resize(array_size * num_mipmaps);
			for (uint32 array_index = 0; array_index < array_size; ++array_index)
			{
				uint32 the_width = width;
				uint32 the_height = height;
				for (uint32 level = 0; level < num_mipmaps; ++level)
				{
					size_t const index = array_index * num_mipmaps + level;
					if (IsCompressedFormat(format))
					{
						uint32 const block_size = NumFormatBytes(format) * 4;
						uint32 image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;

						base[index] = data_block.size();
						data_block.resize(base[index] + image_size);
						init_data[index].row_pitch = (the_width + 3) / 4 * block_size;
						init_data[index].slice_pitch = image_size;

						tex_res.Read(&data_block[base[index]], static_cast<std::streamsize>(image_size));
					}
					else
					{
						init_data[index].row_pitch = (padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
						init_data[index].slice_pitch = init_data[index].row_pitch * the_height;
						base[index] = data_block.size();
						data_block.resize(base[index] + init_data[index].slice_pitch);

						tex_res.Read(&data_block[base[index]], static_cast<std::streamsize>(init_data[index].slice_pitch));
					}

					the_width = std::max<uint32>(the_width / 2, 1);
					the_height = std::max<uint32>(the_height / 2, 1);
				}
			}
		}
		break;

		case TextureType::T_3D:
		{
			init_data.resize(array_size * num_mipmaps);
			base.resize(array_size * num_mipmaps);
			for (uint32 array_index = 0; array_index < array_size; ++array_index)
			{
				uint32 the_width = width;
				uint32 the_height = height;
				uint32 the_depth = depth;
				for (uint32 level = 0; level < num_mipmaps; ++level)
				{
					size_t const index = array_index * num_mipmaps + level;
					if (IsCompressedFormat(format))
					{
						uint32 const block_size = NumFormatBytes(format) * 4;
						uint32 image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * the_depth * block_size;

						base[index] = data_block.size();
						data_block.resize(base[index] + image_size);
						init_data[index].row_pitch = (the_width + 3) / 4 * block_size;
						init_data[index].slice_pitch = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;

						tex_res.Read(&data_block[base[index]], static_cast<std::streamsize>(image_size));
					}
					else
					{
						init_data[index].row_pitch = (padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
						init_data[index].slice_pitch = init_data[index].row_pitch * the_height;
						base[index] = data_block.size();
						data_block.resize(base[index] + init_data[index].slice_pitch * the_depth);

						tex_res.Read(&data_block[base[index]], static_cast<std::streamsize>(init_data[index].slice_pitch * the_depth));
					}

					the_width = std::max<uint32>(the_width / 2, 1);
					the_height = std::max<uint32>(the_height / 2, 1);
					the_depth = std::max<uint32>(the_depth / 2, 1);
				}
			}
		}
		break;

		case TextureType::T_Cube:
		{
			init_data.resize(array_size * 6 * num_mipmaps);
			base.resize(array_size * 6 * num_mipmaps);
			for (uint32 array_index = 0; array_index < array_size; ++array_index)
			{
				for (uint32 face = (uint8)Texture::CubeFaces::Positive_X; face <= (uint8)Texture::CubeFaces::Negative_Z; ++face)
				{
					uint32 the_width = width;
					uint32 the_height = height;
					for (uint32 level = 0; level < num_mipmaps; ++level)
					{
						size_t const index = (array_index * 6 + face - Texture::CubeFaces::Positive_X) * num_mipmaps + level;
						if (IsCompressedFormat(format))
						{
							uint32 const block_size = NumFormatBytes(format) * 4;
							uint32 image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;

							base[index] = data_block.size();
							data_block.resize(base[index] + image_size);
							init_data[index].row_pitch = (the_width + 3) / 4 * block_size;
							init_data[index].slice_pitch = image_size;

							tex_res.Read(&data_block[base[index]], static_cast<std::streamsize>(image_size));
						}
						else
						{
							init_data[index].row_pitch = (padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
							init_data[index].slice_pitch = init_data[index].row_pitch * the_width;
							base[index] = data_block.size();
							data_block.resize(base[index] + init_data[index].slice_pitch);

							tex_res.Read(&data_block[base[index]], static_cast<std::streamsize>(init_data[index].slice_pitch));
						}

						the_width = std::max<uint32>(the_width / 2, 1);
						the_height = std::max<uint32>(the_height / 2, 1);
					}
				}
			}
		}
		break;
		}

		for (size_t i = 0; i < base.size(); ++i)
		{
			init_data[i].data = &data_block[base[i]];
		}
	}


	Render::TexturePtr LoadDDSTexture(X::path const& texpath, uint32 access) {
		auto pAsset = platform::AssetResourceScheduler::Instance().SyncLoad<dds::DDSLoadingDesc>(texpath);
		auto& device = Render::Context::Instance().GetDevice();
		switch (pAsset->GetTextureType()) {
		case TextureType::T_1D:
			return leo::share_raw(device.CreateTexture(pAsset->GetWidth(), pAsset->GetMipmapSize(), pAsset->GetArraySize(),
				pAsset->GetFormat(), access, { 1,0 }, pAsset->GetElementInitDatas().data()));
		case TextureType::T_2D:
			return leo::share_raw(device.CreateTexture(pAsset->GetWidth(), pAsset->GetHeight(), pAsset->GetMipmapSize(), pAsset->GetArraySize(),
				pAsset->GetFormat(), access, { 1,0 }, pAsset->GetElementInitDatas().data()));
		case TextureType::T_3D:
		{
			Render::Texture3DInitializer Initializer;
			Initializer.Width = pAsset->GetWidth();
			Initializer.Height = pAsset->GetHeight();
			Initializer.Depth = pAsset->GetDepth();
			Initializer.NumMipmaps = pAsset->GetMipmapSize();
			Initializer.ArraySize = pAsset->GetArraySize();
			Initializer.Format = pAsset->GetFormat();
			Initializer.Access = access;

			return leo::share_raw(device.CreateTexture(Initializer,pAsset->GetElementInitDatas().data()));
		}
		case TextureType::T_Cube:
			return leo::share_raw(device.CreateTextureCube(pAsset->GetWidth(), pAsset->GetMipmapSize(), pAsset->GetArraySize(),
				pAsset->GetFormat(), access, { 1,0 }, pAsset->GetElementInitDatas().data()));
		}

		LAssert(false, "Out of TextureType");
		return {};
	}

	Render::TexturePtr X::LoadTexture(X::path const& texpath, uint32 access) {
		return LoadDDSTexture(texpath, access);
	}


	using namespace bc;
	using namespace etc;
	void EncodeTexture(void* dst_data, uint32 dst_row_pitch, uint32 dst_slice_pitch, EFormat dst_format,
		void const* src_data, uint32 src_row_pitch, uint32 src_slice_pitch, EFormat src_format,
		uint32 src_width, uint32 src_height, uint32 src_depth)
	{
		LAssert(IsCompressedFormat(dst_format) && !IsCompressedFormat(src_format), "format does not meet the compress requirements");

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
			throw leo::unimplemented();
			break;

		case EF_ETC2_R11:
		case EF_SIGNED_ETC2_R11:
			// TODO
			throw leo::unimplemented();
			break;

		case EF_ETC2_GR11:
		case EF_SIGNED_ETC2_GR11:
			// TODO
			throw leo::unimplemented();
			break;

		default:
			LAssert(false, "format out of range");
			break;
		}

		uint8_t const* src = static_cast<uint8_t const*>(src_data);
		uint8_t* dst = static_cast<uint8_t*>(dst_data);
		for (uint32 z = 0; z < src_depth; ++z)
		{
			codec->EncodeMem(src_width, src_height, dst, dst_row_pitch, dst_slice_pitch,
				src, src_row_pitch, src_slice_pitch, TCM_Quality);

			src += src_slice_pitch;
			dst += dst_slice_pitch;
		}
	}

	void DecodeTexture(std::vector<uint8_t>& dst_data_block, uint32& dst_row_pitch, uint32& dst_slice_pitch, EFormat& dst_format,
		void const* src_data, uint32 src_row_pitch, uint32 src_slice_pitch, EFormat src_format,
		uint32 src_width, uint32 src_height, uint32 src_depth)
	{
		LAssert(IsCompressedFormat(src_format), "format does not meet the compress requirements");

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
			throw leo::unimplemented();
			break;

		case EF_ETC2_R11:
		case EF_SIGNED_ETC2_R11:
			// TODO
			throw leo::unimplemented();
			break;

		case EF_ETC2_GR11:
		case EF_SIGNED_ETC2_GR11:
			// TODO
			throw leo::unimplemented();
			break;

		default:
			throw leo::unimplemented();
			break;
		}

		dst_row_pitch = src_width * NumFormatBytes(dst_format);
		dst_slice_pitch = dst_row_pitch * src_height;
		dst_data_block.resize(src_depth * dst_slice_pitch);

		uint8_t const* src = static_cast<uint8_t const*>(src_data);
		uint8_t* dst = static_cast<uint8_t*>(&dst_data_block[0]);
		for (uint32 z = 0; z < src_depth; ++z)
		{
			codec->DecodeMem(src_width, src_height, dst, dst_row_pitch, dst_slice_pitch,
				src, src_row_pitch, src_slice_pitch);

			src += src_slice_pitch;
			dst += dst_slice_pitch;
		}
	}


	void X::ResizeTexture(void* dst_data, uint32 dst_row_pitch, uint32 dst_slice_pitch, Render::EFormat dst_format,
		uint16 dst_width, uint16 dst_height, uint16 dst_depth,
		void const* src_data, uint32 src_row_pitch, uint32 src_slice_pitch, Render::EFormat src_format,
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
				LAssert(false, "format out of range");
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

		uint8 const* src_ptr = static_cast<uint8 const*>(src_cpu_data);
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

					uint8 const* src_p = src_ptr + sz * src_cpu_slice_pitch + sy * src_cpu_row_pitch;
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
					uint32 sz1 = clamp<uint32>(0, src_depth - 1, sz0 + 1);
					float weight_z = fz - sz0;

					for (uint32 y = 0; y < dst_height; ++y)
					{
						float fy = static_cast<float>(y) / dst_height * src_height;
						uint32 sy0 = static_cast<uint32>(fy);
						uint32 sy1 = clamp<uint32>(0, src_height - 1, sy0 + 1);
						float weight_y = fy - sy0;

						for (uint32 x = 0; x < dst_width; ++x)
						{
							float fx = static_cast<float>(x) / dst_width * src_width;
							uint32 sx0 = static_cast<uint32>(fx);
							uint32 sx1 = clamp<uint32>(0, src_width - 1, sx0 + 1);
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

	void X::SaveTexture(path const& path, Render::TextureType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, Render::EFormat format, leo::span<Render::ElementInitData> init_data)
	{
		using namespace dds;
		std::ofstream file(path, std::ios_base::binary);
		if (!file)
			throw path;

		uint32_t magic = (asset::four_cc<'D', 'D', 'S', ' '>::value);
		file.write(reinterpret_cast<char*>(&magic), sizeof(magic));

		dds::SURFACEDESC2 desc;
		std::memset(&desc, 0, sizeof(desc));

		desc.size = sizeof(desc);

		desc.flags |= DDSD_WIDTH;
		desc.flags |= DDSD_HEIGHT;

		desc.width = width;
		desc.height = height;

		if (numMipMaps != 0)
		{
			desc.flags |= DDSD_MIPMAPCOUNT;
			desc.mip_map_count = numMipMaps;
		}

		desc.pixel_format.size = sizeof(desc.pixel_format);

		if (IsSRGB(format))
		{
			desc.reserved1[0] = 1;
		}

		if (array_size != 1)
		{
			desc.pixel_format.flags |= DDSPF_FOURCC;
			desc.pixel_format.four_cc = asset::four_cc<'D', 'X', '1', '0'>::value;
		}
		else
		{
			if ((EF_ABGR16 == format)
				|| IsFloatFormat(format) || IsCompressedFormat(format))
			{
				desc.pixel_format.flags |= DDSPF_FOURCC;

				switch (format)
				{
				case EF_ABGR16:
					desc.pixel_format.four_cc = 36;
					break;

				case EF_SIGNED_ABGR16:
					desc.pixel_format.four_cc = 110;
					break;

				case EF_R16F:
					desc.pixel_format.four_cc = 111;
					break;

				case EF_GR16F:
					desc.pixel_format.four_cc = 112;
					break;

				case EF_ABGR16F:
					desc.pixel_format.four_cc = 113;
					break;

				case EF_R32F:
					desc.pixel_format.four_cc = 114;
					break;

				case EF_GR32F:
					desc.pixel_format.four_cc = 115;
					break;

				case EF_ABGR32F:
					desc.pixel_format.four_cc = 116;
					break;

				case EF_BC1:
				case EF_BC1_SRGB:
					desc.pixel_format.four_cc = asset::four_cc<'D', 'X', 'T', '1'>::value;
					break;

				case EF_BC2:
				case EF_BC2_SRGB:
					desc.pixel_format.four_cc = asset::four_cc<'D', 'X', 'T', '3'>::value;
					break;

				case EF_BC3:
				case EF_BC3_SRGB:
					desc.pixel_format.four_cc = asset::four_cc<'D', 'X', 'T', '5'>::value;
					break;

				case EF_BC4:
				case EF_BC4_SRGB:
					desc.pixel_format.four_cc = asset::four_cc<'B', 'C', '4', 'U'>::value;
					break;

				case EF_SIGNED_BC4:
					desc.pixel_format.four_cc = asset::four_cc<'B', 'C', '4', 'S'>::value;
					break;

				case EF_BC5:
				case EF_BC5_SRGB:
					desc.pixel_format.four_cc = asset::four_cc<'B', 'C', '5', 'U'>::value;
					break;

				case EF_SIGNED_BC5:
					desc.pixel_format.four_cc = asset::four_cc<'B', 'C', '5', 'S'>::value;
					break;

				case EF_B10G11R11F:
				case EF_BC6:
				case EF_SIGNED_BC6:
				case EF_BC7:
				case EF_BC7_SRGB:
				case EF_ETC1:
				case EF_ETC2_R11:
				case EF_SIGNED_ETC2_R11:
				case EF_ETC2_GR11:
				case EF_SIGNED_ETC2_GR11:
				case EF_ETC2_BGR8:
				case EF_ETC2_BGR8_SRGB:
				case EF_ETC2_A1BGR8:
				case EF_ETC2_A1BGR8_SRGB:
				case EF_ETC2_ABGR8:
				case EF_ETC2_ABGR8_SRGB:
					desc.pixel_format.four_cc = asset::four_cc<'D', 'X', '1', '0'>::value;
					break;

				default:
					throw "Invalid element format";
				}
			}
			else
			{
				switch (format)
				{
				case EF_ARGB4:
					desc.pixel_format.flags |= DDSPF_RGB;
					desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
					desc.pixel_format.rgb_bit_count = 16;

					desc.pixel_format.rgb_alpha_bit_mask = 0x0000F000;
					desc.pixel_format.r_bit_mask = 0x00000F00;
					desc.pixel_format.g_bit_mask = 0x000000F0;
					desc.pixel_format.b_bit_mask = 0x0000000F;
					break;

				case EF_GR8:
					desc.pixel_format.flags |= DDSPF_FOURCC;
					desc.pixel_format.four_cc = asset::four_cc<'D', 'X', '1', '0'>::value;
					break;

				case EF_SIGNED_GR8:
					desc.pixel_format.flags |= DDSPF_BUMPDUDV;
					desc.pixel_format.rgb_bit_count = 16;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x000000FF;
					desc.pixel_format.g_bit_mask = 0x0000FF00;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_SIGNED_R16:
					desc.pixel_format.flags |= DDSPF_BUMPDUDV;
					desc.pixel_format.rgb_bit_count = 16;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x0000FFFF;
					desc.pixel_format.g_bit_mask = 0x00000000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_ARGB8:
				case EF_ARGB8_SRGB:
					desc.pixel_format.flags |= DDSPF_RGB;
					desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0xFF000000;
					desc.pixel_format.r_bit_mask = 0x00FF0000;
					desc.pixel_format.g_bit_mask = 0x0000FF00;
					desc.pixel_format.b_bit_mask = 0x000000FF;
					break;

				case EF_ABGR8:
				case EF_ABGR8_SRGB:
					desc.pixel_format.flags |= DDSPF_RGB;
					desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0xFF000000;
					desc.pixel_format.r_bit_mask = 0x000000FF;
					desc.pixel_format.g_bit_mask = 0x0000FF00;
					desc.pixel_format.b_bit_mask = 0x00FF0000;
					break;

				case EF_SIGNED_ABGR8:
					desc.pixel_format.flags |= DDSPF_BUMPDUDV;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0xFF000000;
					desc.pixel_format.r_bit_mask = 0x000000FF;
					desc.pixel_format.g_bit_mask = 0x0000FF00;
					desc.pixel_format.b_bit_mask = 0x00FF0000;
					break;

				case EF_A2BGR10:
					desc.pixel_format.flags |= DDSPF_RGB;
					desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0xC0000000;
					desc.pixel_format.r_bit_mask = 0x000003FF;
					desc.pixel_format.g_bit_mask = 0x000FFC00;
					desc.pixel_format.b_bit_mask = 0x3FF00000;
					break;

				case EF_SIGNED_A2BGR10:
					desc.pixel_format.flags |= DDSPF_BUMPDUDV;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0xC0000000;
					desc.pixel_format.r_bit_mask = 0x000003FF;
					desc.pixel_format.g_bit_mask = 0x000FFC00;
					desc.pixel_format.b_bit_mask = 0x3FF00000;
					break;

				case EF_GR16:
					desc.pixel_format.flags |= DDSPF_RGB;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x0000FFFF;
					desc.pixel_format.g_bit_mask = 0xFFFF0000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_SIGNED_GR16:
					desc.pixel_format.flags |= DDSPF_BUMPDUDV;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x0000FFFF;
					desc.pixel_format.g_bit_mask = 0xFFFF0000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_R8:
					desc.pixel_format.flags |= DDSPF_LUMINANCE;
					desc.pixel_format.rgb_bit_count = 8;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x000000FF;
					desc.pixel_format.g_bit_mask = 0x00000000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_R16:
					desc.pixel_format.flags |= DDSPF_LUMINANCE;
					desc.pixel_format.rgb_bit_count = 16;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x0000FFFF;
					desc.pixel_format.g_bit_mask = 0x00000000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_A8:
					desc.pixel_format.flags |= DDSPF_ALPHA;
					desc.pixel_format.rgb_bit_count = 8;

					desc.pixel_format.rgb_alpha_bit_mask = 0x000000FF;
					desc.pixel_format.r_bit_mask = 0x00000000;
					desc.pixel_format.g_bit_mask = 0x00000000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				default:
					throw "Invalid element format" ;
				}
			}
		}

		if (desc.pixel_format.four_cc != asset::four_cc<'D', 'X', '1', '0'>::value)
		{
			desc.flags |= DDSD_CAPS;
			desc.flags |= DDSD_PIXELFORMAT;

			desc.dds_caps.caps1 = DDSCAPS_TEXTURE;
			if (numMipMaps != 1)
			{
				desc.dds_caps.caps1 |= DDSCAPS_MIPMAP;
				desc.dds_caps.caps1 |= DDSCAPS_COMPLEX;
			}
			if (TextureType::T_3D == type)
			{
				desc.dds_caps.caps1 |= DDSCAPS_COMPLEX;
				desc.dds_caps.caps2 |= DDSCAPS2_VOLUME;
				desc.flags |= DDSD_DEPTH;
				desc.depth = depth;
			}
			if (TextureType::T_Cube == type)
			{
				desc.dds_caps.caps1 |= DDSCAPS_COMPLEX;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_POSITIVEX;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEX;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_POSITIVEY;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEY;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_POSITIVEZ;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEZ;
			}
		}

		uint32_t format_size = NumFormatBytes(format);
		if (IsCompressedFormat(format))
		{
			uint32_t const block_size = NumFormatBytes(format) * 4;
			uint32_t image_size = ((width + 3) / 4) * ((height + 3) / 4) * block_size;

			desc.flags |= DDSD_LINEARSIZE;
			desc.linear_size = image_size;
		}

		dds::SURFACEDESC2 desc_le = desc;
		desc_le.size = (desc_le.size);
		desc_le.flags = (desc_le.flags);
		desc_le.height = (desc_le.height);
		desc_le.width = (desc_le.width);
		desc_le.pitch = (desc_le.pitch);
		desc_le.depth = (desc_le.depth);
		desc_le.mip_map_count = (desc_le.mip_map_count);
		for (uint32_t i = 0; i < std::size(desc_le.reserved1); ++i)
		{
			desc_le.reserved1[i] = (desc_le.reserved1[i]);
		}
		desc_le.pixel_format.size = (desc_le.pixel_format.size);
		desc_le.pixel_format.flags = (desc_le.pixel_format.flags);
		desc_le.pixel_format.four_cc = (desc_le.pixel_format.four_cc);
		desc_le.pixel_format.rgb_bit_count = (desc_le.pixel_format.rgb_bit_count);
		desc_le.pixel_format.r_bit_mask = (desc_le.pixel_format.r_bit_mask);
		desc_le.pixel_format.g_bit_mask = (desc_le.pixel_format.g_bit_mask);
		desc_le.pixel_format.b_bit_mask = (desc_le.pixel_format.b_bit_mask);
		desc_le.pixel_format.rgb_alpha_bit_mask = (desc_le.pixel_format.rgb_alpha_bit_mask);
		desc_le.dds_caps.caps1 = (desc_le.dds_caps.caps1);
		desc_le.dds_caps.caps2 = (desc_le.dds_caps.caps2);
		for (uint32_t i = 0; i < std::size(desc_le.dds_caps.reserved); ++i)
		{
			desc_le.dds_caps.reserved[i] = (desc_le.dds_caps.reserved[i]);
		}
		desc_le.reserved2 = (desc_le.reserved2);
		file.write(reinterpret_cast<char*>(&desc_le), sizeof(desc_le));

		if (asset::four_cc<'D', 'X', '1', '0'>::value == desc.pixel_format.four_cc)
		{
			dds::HEADER_DXT10 desc10;
			desc10.dxgi_format = ToDXGIFormat(format);
			desc10.misc_flag = 0;
			switch (type)
			{
			case TextureType::T_1D:
				desc10.resource_dim = D3D_RESOURCE_DIMENSION_TEXTURE1D;
				break;

			case TextureType::T_2D:
				desc10.resource_dim = D3D_RESOURCE_DIMENSION_TEXTURE2D;
				break;

			case TextureType::T_3D:
				desc10.resource_dim = D3D_RESOURCE_DIMENSION_TEXTURE3D;
				break;

			case TextureType::T_Cube:
				desc10.resource_dim = D3D_RESOURCE_DIMENSION_TEXTURE2D;
				desc10.misc_flag = D3D_RESOURCE_MISC_TEXTURECUBE;
				break;

			default:
				throw "Invalid texture type";
			}
			desc10.array_size = array_size;
			if (TextureType::T_Cube == type)
			{
				desc10.array_size *= 6;
			}
			desc10.reserved = 0;

			desc10.dxgi_format = (desc10.dxgi_format);
			desc10.resource_dim = (desc10.resource_dim);
			desc10.misc_flag = (desc10.misc_flag);
			desc10.array_size = (desc10.array_size);
			desc10.reserved = (desc10.reserved);
			desc10.array_size = (desc10.array_size);
			file.write(reinterpret_cast<char*>(&desc10), sizeof(desc10));
		}

		switch (type)
		{
		case TextureType::T_1D:
		{
			for (uint32_t array_index = 0; array_index < array_size; ++array_index)
			{
				uint32_t the_width = width;
				for (uint32_t level = 0; level < desc.mip_map_count; ++level)
				{
					size_t const index = array_index * desc.mip_map_count + level;
					uint32_t image_size;
					if (IsCompressedFormat(format))
					{
						uint32_t const block_size = NumFormatBytes(format) * 4;
						image_size = ((the_width + 3) / 4) * block_size;
					}
					else
					{
						image_size = the_width * format_size;
					}

					file.write(reinterpret_cast<char const*>(init_data[index].data), static_cast<std::streamsize>(image_size));

					the_width = std::max<uint32_t>(the_width / 2, 1);
				}
			}
		}
		break;

		case TextureType::T_2D:
		{
			for (uint32_t array_index = 0; array_index < array_size; ++array_index)
			{
				uint32_t the_width = width;
				uint32_t the_height = height;
				for (uint32_t level = 0; level < desc.mip_map_count; ++level)
				{
					size_t const index = array_index * desc.mip_map_count + level;
					uint32_t image_size;
					if (IsCompressedFormat(format))
					{
						uint32_t const block_size = NumFormatBytes(format) * 4;
						image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;
					}
					else
					{
						image_size = the_width * the_height * format_size;
					}

					file.write(reinterpret_cast<char const*>(init_data[index].data), static_cast<std::streamsize>(image_size));

					the_width = std::max<uint32_t>(the_width / 2, 1);
					the_height = std::max<uint32_t>(the_height / 2, 1);
				}
			}
		}
		break;

		case TextureType::T_3D:
		{
			for (uint32_t array_index = 0; array_index < array_size; ++array_index)
			{
				uint32_t the_width = width;
				uint32_t the_height = height;
				uint32_t the_depth = depth;
				for (uint32_t level = 0; level < desc.mip_map_count; ++level)
				{
					size_t const index = array_index * desc.mip_map_count + level;
					uint32_t image_size;
					if (IsCompressedFormat(format))
					{
						uint32_t const block_size = NumFormatBytes(format) * 4;
						image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * the_depth * block_size;
					}
					else
					{
						image_size = the_width * the_height * the_depth * format_size;
					}

					file.write(reinterpret_cast<char const*>(init_data[index].data), static_cast<std::streamsize>(image_size));

					the_width = std::max<uint32_t>(the_width / 2, 1);
					the_height = std::max<uint32_t>(the_height / 2, 1);
					the_depth = std::max<uint32_t>(the_depth / 2, 1);
				}
			}
		}
		break;

		case TextureType::T_Cube:
		{
			for (uint32_t array_index = 0; array_index < array_size; ++array_index)
			{
				for (uint32_t face = (uint8)TextureCubeFaces::Positive_X; face <= (uint8)TextureCubeFaces::Negative_Z; ++face)
				{
					uint32_t the_width = width;
					for (uint32_t level = 0; level < desc.mip_map_count; ++level)
					{
						size_t const index = (array_index * 6 + face - (uint8)TextureCubeFaces::Positive_X) * numMipMaps + level;
						uint32_t image_size;
						if (IsCompressedFormat(format))
						{
							uint32_t const block_size = NumFormatBytes(format) * 4;
							image_size = ((the_width + 3) / 4) * ((the_width + 3) / 4) * block_size;
						}
						else
						{
							image_size = the_width * the_width * format_size;
						}

						file.write(reinterpret_cast<char const*>(init_data[index].data), static_cast<std::streamsize>(image_size));

						the_width = std::max<uint32_t>(the_width / 2, 1);
					}
				}
			}
		}
		break;
		}

	}
}
