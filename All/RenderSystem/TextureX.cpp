#include "TextureX.hpp"
#include "RenderSystem.hpp"
#include "D3D11/D3D11RenderSystem.hpp"
#include "D3D11/D3D11Texture.hpp"

#include "DirectXTex.h"
#include "D3DX11.hpp"
#include "file.hpp"
#include "DeviceMgr.h"

namespace {
	auto context = [] {return leo::DeviceMgr().GetDeviceContext(); };
}
namespace leo {
	namespace X {

		TexturePtr MakeTexture1D(uint16 width, uint8 numMipMaps, uint8 array_size,
			EFormat format, uint32_t access, ElementInitData const * init_data)
		{
			return GetRenderEngine().GetFactory().MakeTexture1D(
				width,
				numMipMaps, array_size,
				format, access,
				init_data);
		}

		TexturePtr MakeTexture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size, EFormat format, SampleDesc sample_info, uint32 access, ElementInitData const * init_data)
		{
			return GetRenderEngine().GetFactory().MakeTexture2D(
				width, height, 
				numMipMaps, array_size, 
				format, sample_info, access, 
				init_data);
		}

		TexturePtr MakeTextureCube(uint16 size, uint8 numMipMaps, uint8 array_size,
			EFormat format, uint32_t access, ElementInitData const * init_data)
		{
			return GetRenderEngine().GetFactory().MakeTextureCube(
				size,
				numMipMaps, array_size,
				format, access,
				init_data);
		}

		TexturePtr SyncLoadTexture(const path& tex_name, uint32 access) {
			static DirectX::TexMetadata metadata;
			static DirectX::ScratchImage image;

			auto filename = tex_name.generic_wstring();

			switch (leo::win::file::GetFileExt(tex_name.generic_wstring()))
			{
			case win::file::FILE_TYPE::DDS:
				dxcall(DirectX::LoadFromDDSFile(filename.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, image));
				break;
			case win::file::FILE_TYPE::TGA:
				dxcall(DirectX::LoadFromTGAFile(filename.c_str(), &metadata, image));
				break;
			case win::file::FILE_TYPE::OTHER_TEX_BEGIN:
			case win::file::FILE_TYPE::OTHER_TEX_END:
				dxcall(DirectX::LoadFromWICFile(filename.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, image));
				break;
			default:
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "不支持的数据格式");
				break;
			}

			const DirectX::Image* srcImages = image.GetImages();
			size_t nimages = image.GetImageCount();

			if (!metadata.mipLevels || !metadata.arraySize)
				Raise_Error_Exception(E_INVALIDARG,"无效的贴图");

#ifdef PLATFORM_64BIT
			if ((metadata.width > 0xFFFFFFFF) || (metadata.height > 0xFFFFFFFF)
				|| (metadata.mipLevels > 0xFFFFFFFF) || (metadata.arraySize > 0xFFFFFFFF))
				return Raise_Error_Exception(E_INVALIDARG,"数值错误");
#endif

			auto init_data = std::make_unique<ElementInitData[]>(metadata.mipLevels * metadata.arraySize);

			// Fill out subresource array
			if (metadata.IsVolumemap())
			{
				//--- Volume case -------------------------------------------------------------
				if (!metadata.depth)
					Raise_Error_Exception(E_INVALIDARG, "无效的贴图");

#ifdef PLATFORM_64BIT
				if (metadata.depth > 0xFFFFFFFF)
					return Raise_Error_Exception(E_INVALIDARG, "数值错误");
#endif

				if (metadata.arraySize > 1)
					Raise_Error_Exception(E_INVALIDARG,"Direct3D 11 doesn't support arrays of 3D textures");

				size_t depth = metadata.depth;

				size_t idx = 0;
				for (size_t level = 0; level < metadata.mipLevels; ++level)
				{
					size_t index = metadata.ComputeIndex(level, 0, 0);
					if (index >= nimages)
						return nullptr;

					const auto& img = srcImages[index];

					if (img.format != metadata.format)
						return nullptr;

					if (!img.pixels)
						return nullptr;

					// Verify pixels in image 1 .. (depth-1) are exactly image->slicePitch apart
					// For 3D textures, this relies on all slices of the same miplevel being continous in memory
					// (this is how ScratchImage lays them out), which is why we just give the 0th slice to Direct3D 11
					const uint8_t* pSlice = img.pixels + img.slicePitch;
					for (size_t slice = 1; slice < depth; ++slice)
					{
						size_t tindex = metadata.ComputeIndex(level, 0, slice);
						if (tindex >= nimages)
							return nullptr;

						const auto& timg = srcImages[tindex];

						if (!timg.pixels)
							return nullptr;

						if (timg.pixels != pSlice
							|| timg.format != metadata.format
							|| timg.rowPitch != img.rowPitch
							|| timg.slicePitch != img.slicePitch)
							return nullptr;

						pSlice = timg.pixels + img.slicePitch;
					}

					assert(idx < (metadata.mipLevels * metadata.arraySize));

					init_data[idx].data = img.pixels;
					init_data[idx].row_pitch = static_cast<DWORD>(img.rowPitch);
					init_data[idx].slice_pitch = static_cast<DWORD>(img.slicePitch);
					++idx;

					if (depth > 1)
						depth >>= 1;
				}
			}
			else
			{
				//--- 1D or 2D texture case ---------------------------------------------------
				size_t idx = 0;
				for (size_t item = 0; item < metadata.arraySize; ++item)
				{
					for (size_t level = 0; level < metadata.mipLevels; ++level)
					{
						size_t index = metadata.ComputeIndex(level, item, 0);
						if (index >= nimages)
							return nullptr;

						const auto& img = srcImages[index];

						if (img.format != metadata.format)
							return nullptr;

						if (!img.pixels)
							return nullptr;

						assert(idx < (metadata.mipLevels * metadata.arraySize));

						init_data[idx].data = img.pixels;
						init_data[idx].row_pitch = static_cast<DWORD>(img.rowPitch);
						init_data[idx].slice_pitch = static_cast<DWORD>(img.slicePitch);
						++idx;
					}
				}
			}

			switch (metadata.dimension)
			{
			case DirectX::TEX_DIMENSION_TEXTURE1D:
			{
				return MakeTexture1D(static_cast<uint16>(metadata.width), static_cast<uint8>(metadata.mipLevels), static_cast<uint8>(metadata.arraySize), D3D11Mapping::MappingFormat(metadata.format), access, init_data.get());
			}
			break;

			case DirectX::TEX_DIMENSION_TEXTURE2D:
			{
				if (metadata.IsCubemap())
					return MakeTextureCube(static_cast<uint16>(metadata.width), static_cast<uint8>(metadata.mipLevels), static_cast<uint8>(metadata.arraySize), D3D11Mapping::MappingFormat(metadata.format), access, init_data.get());
				else
					return MakeTexture2D(static_cast<uint16>(metadata.width), static_cast<uint16>(metadata.height),static_cast<uint8>(metadata.mipLevels), static_cast<uint8>(metadata.arraySize),D3D11Mapping::MappingFormat(metadata.format),{},access, init_data.get());
			}
			break;

			case DirectX::TEX_DIMENSION_TEXTURE3D:
			{
				Raise_Error_Exception(E_INVALIDARG, "LeoEngine doesn't support 3D texture now");
			}
			break;
			}


			return nullptr;
		}

		bool SyncSaveTexture(const path& tex_path, TexturePtr tex) {
			D3D11Texture2D* tex_2d = static_cast<D3D11Texture2D*>(tex.get());
			auto path = tex_path.generic_wstring();
			dx::SaveDDSTextureToFile(context(), tex_2d->Resource(), path.c_str());
			return true;
		}
	}

}