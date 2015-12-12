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
#include "HUD/HUDGraphics.h"

namespace leo {
	namespace X {
		TexturePtr MakeTexture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size,
			EFormat format, SampleDesc sample_info, uint32 access, uint8 const * init_data);

		std::unique_ptr<HUD::IImage> MakeIImage(uint16, uint16);
	}
}

#endif
