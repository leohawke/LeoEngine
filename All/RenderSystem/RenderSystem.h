////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   RenderSystem/RenderSystem.hpp
//  Version:     v1.00
//  Created:     12/13/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 由于隔离底层实现成[DX/OPENGL],以及前向声明
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////

#ifndef ShaderSystem_ShaderSystem_Hpp
#define ShaderSystem_ShaderSystem_Hpp

#include "Texture.hpp"

//PreDe
namespace leo {
	class LB_API RenderFactory : noncopyable {
	public:
		virtual ~RenderFactory();

		virtual std::string const & Name() const = 0;

		virtual TexturePtr MakeTexture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size,
			EFormat format, SampleDesc sample_info, uint32 access, uint8 const * init_data) = 0;
	};


	LB_API RenderFactory&  GetRenderFactory();


}


#endif
