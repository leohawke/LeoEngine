////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   RenderSystem/Texture.hpp
//  Version:     v1.00
//  Created:     12/12/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 纹理对象
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////

#ifndef ShaderSystem_Texture_Hpp
#define ShaderSystem_Texture_Hpp

#include <ldef.h>
#include <memory>
#include <leoint.hpp>

namespace leo {
	//Todo,define in a common header file
	/*
	\brief ElementFormat：元素数据及类型格式。
	*/
	enum class EFormat;


	class  Texture {
	};

	using TexturePtr = std::shared_ptr<Texture>;
}

#endif
