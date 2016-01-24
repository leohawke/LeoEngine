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
#include <base.h>
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
		//Only Support DT_2D Map
		class Mapper : noncopyable
		{
			friend class Texture;

		public:
			Mapper(Texture& tex, uint8 array_index, uint8 level, MapAccess tma,
				uint16 x_offset, uint16 width)
				: TexRef(tex),
				MappedArrayIndex(array_index),
				MappedLevel(level)
			{
				TexRef.Map1D(array_index, level, tma, x_offset, width, pSysMem);
				RowPitch = SlicePitch = width * NumFormatBytes(tex.Format());
			}
			Mapper(Texture& tex, uint8 array_index, uint8 level, MapAccess tma,
				uint16 x_offset, uint16 y_offset,
				uint16 width, uint16 height)
				: TexRef(tex),
				MappedArrayIndex(array_index),
				MappedLevel(level)
			{
				TexRef.Map2D(array_index, level, tma, x_offset, y_offset, width, height, pSysMem, RowPitch);
				SlicePitch = RowPitch * height;
			}
			/*Mapper(Texture& tex, uint8 array_index, uint8 level, TextureMapAccess tma,
				uint16 x_offset, uint16 y_offset, uint32_t z_offset,
				uint16 width, uint16 height, uint32_t depth)
				: TexRef(tex),
				MappedArrayIndex(array_index),
				MappedLevel(level)
			{
				TexRef.Map3D(array_index, level, tma, x_offset, y_offset, z_offset, width, height, depth, pSysMem, RowPitch, SlicePitch);
			}*/
			Mapper(Texture& tex, uint8 array_index, CubeFaces face, uint8 level, MapAccess tma,
				uint16 x_offset, uint16 y_offset,
				uint16 width, uint16 height)
				: TexRef(tex),
				MappedArrayIndex(array_index),
				MappedFace(face),
				MappedLevel(level)
			{
				TexRef.MapCube(array_index, face, level, tma, x_offset, y_offset, width, height, pSysMem, RowPitch);
				SlicePitch = RowPitch * height;
			}

			~Mapper()
			{
				switch (TexRef.Type())
				{
				case DT_1D:
					TexRef.Unmap1D(MappedArrayIndex, MappedLevel);
					break;

				case DT_2D:
					TexRef.Unmap2D(MappedArrayIndex, MappedLevel);
					break;

				/*case TT_3D:
					TexRef.Unmap3D(MappedArrayIndex, MappedLevel);
					break;*/

				case DT_Cube:
					TexRef.UnmapCube(MappedArrayIndex, MappedFace, MappedLevel);
					break;
				}
			}

			template <typename T>
			const T* Pointer() const
			{
				return static_cast<T*>(pSysMem);
			}
			template <typename T>
			T* Pointer()
			{
				return static_cast<T*>(pSysMem);
			}

			uint32_t GetRowPitch() const
			{
				return RowPitch;
			}

			uint32_t GetSlicePitch() const
			{
				return SlicePitch;
			}
		public:
			void* pSysMem;
			uint32_t RowPitch, SlicePitch;
		private:
			Texture& TexRef;

			uint8 MappedArrayIndex;
			CubeFaces MappedFace;
			uint8 MappedLevel;
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

		virtual void Map1D(uint8 array_index, uint8 level, MapAccess tma,
			uint16 x_offset, uint16 width,
			void*& data) = 0;
		virtual void Map2D(uint8 array_index, uint8 level, MapAccess tma,
			uint16 x_offset, uint16 y_offset, uint16 width, uint16 height,
			void*& data, uint32_t& row_pitch) = 0;
		virtual void MapCube(uint8 array_index, CubeFaces face, uint8 level, MapAccess tma,
			uint16 x_offset, uint16 y_offset, uint16 width, uint16 height,
			void*& data, uint32_t& row_pitch) = 0;

		virtual void Unmap1D(uint8 array_index, uint8 level) = 0;
		virtual void Unmap2D(uint8 array_index, uint8 level) = 0;
		virtual void UnmapCube(uint8 array_index, CubeFaces face, uint8 level) = 0;

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
