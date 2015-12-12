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
		TexturePtr MakeTexture2D(uint16, uint16, uint8, uint8, EFormat, uint8, uint8, uint32, uint8* data);

		std::unique_ptr<HUD::IImage> MakeIImage(uint16, uint16);
	}
}

#endif
