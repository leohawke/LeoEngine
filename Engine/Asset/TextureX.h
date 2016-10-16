/*! \file Engine\Asset\Texture.h
\ingroup Engine
\brief Texture IO/Infomation ...
*/
#ifndef LE_ASSET_TEXTURE_X_H
#define LE_ASSET_TEXTURE_X_H 1

#include "../Render/ITexture.hpp"
#include "../Core/LFile.h"

#include <experimental/filesystem>

namespace platform {
	namespace X {
		using path = std::experimental::filesystem::path;

		template<unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3>
		struct four_cc {
			enum { value = (c0 << 0) + (c1 << 8) + (c2 << 16) + (c3 << 24)};
		};

		template<unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3>
		lconstexpr uint32 four_cc_v = four_cc<c0,c1,c2,c3>::value;


		void GetImageInfo(File const & file, Render::TextureType& type,
			uint16& width, uint16& height, uint16& depth, uint8& num_mipmaps, uint8& array_size,
			Render::EFormat& format, uint32& row_pitch, uint32& slice_pitch);

		void GetImageInfo(File const & file, Render::TextureType& type,
			uint16& width, uint16& height, uint16& depth, uint8& num_mipmaps, uint8& array_size,
			Render::EFormat& format,
			std::vector<Render::ElementInitData> & init_data,
			std::vector<uint8>& data_block);

		Render::TexturePtr LoadTexture(path const& texpath, uint32 access);

		void ResizeTexture(void* dst_data, uint32 dst_row_pitch, uint32 dst_slice_pitch,Render::EFormat dst_format,
			uint16 dst_width, uint16 dst_height, uint16 dst_depth,
			void const * src_data, uint32 src_row_pitch, uint32 src_slice_pitch, Render::EFormat src_format,
			uint16 src_width, uint16 src_height, uint16 src_depth,
			bool linear);
	}

#if ENGINE_TOOL
	namespace T {
		//对于大端平台，不使用运行时代码处理，而是将资源转换至逻辑匹配的资源格式
		void ImageLE2BE(File const & file) {

		}
	}
#endif
}
#endif