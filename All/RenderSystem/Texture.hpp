////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   RenderSystem/Texture.hpp
//  Version:     v1.00
//  Created:     12/12/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: Œ∆¿Ì∂‘œÛ
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////

#ifndef ShaderSystem_Texture_Hpp
#define ShaderSystem_Texture_Hpp

#include <memory>
#include <string>
#include <unordered_map>

#include <ldef.h>
#include <utility.hpp>
#include <leoint.hpp>
#include <BaseMacro.h>
#include <LAssert.h>
#include <id.hpp>

#include "EFormat.hpp"

namespace leo {


	class Texture;

	using TexturePtr = ::std::shared_ptr<Texture>;

	class LB_API Texture{
	public:
		//R = Read
		//W = Write
		//O = Only
		enum MapAccess {
			MA_RO,//Read_Only
			MA_WO,//Write_Only
			MA_RW,//Read_Write
		};

		//Dimension_Type
		enum Dis_Type {
			DT_1D,
			DT_2D,
			DT_3D,
			DT_Cube
		};

		//P = Positive
		//N = Negative
		enum CubeFaces {
			CF_P_X =0,//Positive_X
			CF_N_X =1,//Negative_X
			CF_P_Y =2,//Positive_Y
			CF_N_Y =3,//Negative_Y
			CF_P_Z =4,//Positive_Z
			CF_N_Z =5,//Negative_Z
		};

	public:
		class Mapper:noncopyable{
			friend class Texture;
		};

	public:
		explicit Texture(Dis_Type type, uint32 access, SampleDesc sample_info = {});

		virtual ~Texture();

		static TexturePtr NullTexture;

		virtual std::string const & Name() const = 0;

		// Gets the number of mipmaps to be used for this texture.
		uint8 NumMipMaps() const;
		// Gets the size of texture array
		uint8 ArraySize() const;

		// Returns the width of the texture.
		virtual uint16 Width(uint8 level) const = 0;
		// Returns the height of the texture.
		virtual uint16 Height(uint8 level) const = 0;
		// Returns the depth of the texture (only for 3D texture).
		virtual uint16 Depth(uint8 level) const = 0;

		// Returns the pixel format for the texture surface.
		EFormat Format() const;

		// Returns the texture type of the texture.
		Dis_Type Type() const;

		SampleDesc SampleInfo() const;

		uint32_t Access() const;

		virtual void ReclaimHWResource(ElementInitData const * init_data) = 0;
	protected:
		uint8		mNumMipMaps;
		uint8		mArraySize;

		EFormat	mFormat;
		Dis_Type		mDimension;
		SampleDesc		mSampleInfo;
		uint32_t		mAccess;
	};

	
}

#endif
