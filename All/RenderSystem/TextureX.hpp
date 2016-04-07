////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   RenderSystem/TextureX.hpp
//  Version:     v1.00
//  Created:     12/12/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 纹理对象操作
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////

#ifndef ShaderSystem_TextureX_Hpp
#define ShaderSystem_TextureX_Hpp

#include "Texture.hpp"
#include "UI/Graphics.h"

#include <experimental/filesystem>

namespace leo {
	namespace X {
		using std::experimental::filesystem::path;

		LB_API TexturePtr MakeTexture1D(uint16 width, uint8 num_mip_maps, uint8 array_size,
			EFormat format, uint32_t access, ElementInitData const * init_data);
		LB_API TexturePtr MakeTexture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size,
			EFormat format, SampleDesc sample_info, uint32 access,ElementInitData const * init_data);
		LB_API TexturePtr MakeTextureCube(uint16 size, uint8 num_mip_maps, uint8 array_size,
			EFormat format, uint32_t access, ElementInitData const * init_data);

		std::shared_ptr<Drawing::IImage> MakeIImage(Drawing::Size);

		LB_API TexturePtr SyncLoadTexture(const path& tex_name, uint32 access);
		LB_API TexturePtr ASyncLoadTexture(const path& tex_name, uint32 access);

		LB_API bool SyncSaveTexture(const path& tex_path, TexturePtr tex);
	}
}

#endif
