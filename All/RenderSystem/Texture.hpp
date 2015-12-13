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
#include <string>
#include <unordered_map>
#include <utility.hpp>
#include <leoint.hpp>
#include <BaseMacro.h>
#include <LAssert.h>

namespace leo {
	//Todo,define in a common header file
	/*
	\brief ElementFormat：元素数据及类型格式。
	*/
	enum class EFormat;

	/*
	\brief ElementAccess元素访问方式
	*/
	//C = CPU
	//G = GPU
	//R = Read
	//W = Write
	//U = Unordered
	//S = Structured
	//I = Immutable
	//R = Raw
	//A = Append
	//M = Generate_Mips
	//S= Counter
	//D = DrawIndirectArgs
	enum EAccess:uint32 {
		EA_C_R = 1U << 0,//ElementAccess_CPU_READ
		EA_C_W = 1U << 1,//ElementAccess_CPU_Write
		EA_G_R = 1U << 2,//ElementAccess_GPU_Read
		EA_G_W = 1U << 3,//ElementAccess_GPU_Write
		EA_G_U = 1U << 4,//ElementAccess_GPU_Unordered
		EA_I = 1U << 7,//ElementAccess_Immutable
	};

	struct SampleDesc {
		uint32 Count = 1;
		uint32 Quality = 0;
	};

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

	private:
		uint8		mNumMipMaps;
		uint8		mArraySize;

		EFormat	mFormat;
		Dis_Type		mDimension;
		SampleDesc		mSampleInfo;
		uint32_t		mAccess;
	};

	
}

#endif
