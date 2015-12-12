////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   RenderSystem/D3D11/D3D11Texture.hpp
//  Version:     v1.00
//  Created:     12/13/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: D3D11Œ∆¿Ì∂‘œÛ
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////

#ifndef D3D11_Texture_Hpp
#define D3D11_Texture_Hpp

#include "../Texture.hpp"

namespace leo {
	class D3D11Texture :public Texture
	{
	public:
		explicit D3D11Texture(Dis_Type type, uint32 access, SampleDesc sample_info = {});

		virtual ~D3D11Texture();
	};
}

#endif
