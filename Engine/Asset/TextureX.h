/*! \file Engine\Asset\COM.h
\ingroup Engine
\brief Texture IO/Infomation ...
*/
#ifndef LE_ASSET_TEXTURE_X_H
#define LE_ASSET_TEXTURE_X_Hs 1

#include "../Render/ITexture.hpp"
#include "../Core/LFile.h"

namespace platform {
	namespace X {
		void GetImageInfo(File const & file, Render::TextureType& type,
			uint16& width, uint16& height, uint16& depth, uint8& num_mipmaps, uint8& array_size,
			Render::EFormat& format, uint32& row_pitch, uint32& slice_pitch);
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