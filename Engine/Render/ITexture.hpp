/*! \file Engine\Render\IFormat.hpp
\ingroup Engine
\brief GPUŒ∆¿Ì°£
*/
#ifndef LE_RENDER_ITexture_hpp
#define LE_RENDER_ITexture_hpp 1

#include <LBase/memory.hpp>
#include <LBase/sutility.h>
#include <LBase/functional.hpp>
#include "../emacro.h"
#include "IFormat.hpp"


namespace platform {
	namespace Render {
		class Texture;
		class Texture1D;
		class Texture2D;
		class Texture3D;
		class TexureCube;

		using TexturePtr = std::shared_ptr<Texture>;

		class LE_API Texture {
		public:
			enum class TextureMapAccess {
				ReadOnly,
				WriteOnly,
				ReadWrite,
			};

			enum class Type {
				T_1D,
				T_2D,
				T_3D,
				T_Cube
			};

			enum class CubeFaces {
				Positive_X = 0,
				Negative_X = 1,
				Positive_Y = 2,
				Negative_Y = 3,
				Positive_Z = 4,
				Negative_Z = 5,
			};
		protected:
			//\brief encode = UTF-8
			virtual std::string const & Description() const = 0;
		public:
			explicit Texture(Type type, uint32 access, SampleDesc sample_info = {});

			virtual ~Texture();

			// Gets the number of mipmaps to be used for this texture.
			uint8 GetNumMipMaps() const;
			// Gets the size of texture array
			uint8 GetArraySize() const;

			// Returns the pixel format for the texture surface.
			EFormat GetFormat() const;

			// Returns the texture type of the texture.
			Type GetType() const;

			SampleDesc GetSampleInfo() const;
			uint32 GetSampleCount() const;
			uint32 GetSampleQuality() const;

			uint32 GetAccessMode() const;

			// Copies (and maybe scales to fit) the contents of this texture to another texture.
			virtual void CopyToTexture(Texture& target) = 0;

			virtual void BuildMipSubLevels() = 0;

			virtual void HWResourceCreate(ElementInitData const * init_data) = 0;
			virtual void HWResourceDelete() = 0;
			virtual void HWResourceReady() const = 0;
		protected:
			uint8		mipmap_size;
			uint8		array_size;

			EFormat	format;
			Type		type;
			SampleDesc		sample_info;
			uint32		access_mode;
		};

		using TextureMapAccess = Texture::TextureMapAccess;
		using TextureType = Texture::Type;
		using TextureCubeFaces = Texture::CubeFaces;

		class LE_API Texture1D : public Texture {
		public:
			explicit Texture1D(uint32 access, SampleDesc sample_info = {});

			virtual ~Texture1D();

			// Returns the width of the texture.
			virtual uint16 GetWidth(uint8 level) const = 0;

			virtual void Map(uint32 array_index, uint32 level, TextureMapAccess tma,
				uint32 x_offset, uint32 width,
				void*& data) = 0;
			virtual void UnMap(uint32 array_index, uint32 level) = 0;

			virtual void CopyToSubTexture(Texture1D& target,
				uint32 dst_array_index, uint32 dst_level, uint32 dst_x_offset, uint32 dst_width,
				uint32 src_array_index, uint32 src_level, uint32 src_x_offset, uint32 src_width) = 0;
		protected:
			void Resize(uint32 dst_array_index, uint32 dst_level, uint32 dst_x_offset, uint32 dst_width,
				uint32 src_array_index, uint32 src_level, uint32 src_x_offset, uint32 src_width,
				bool linear);
		};

		class LE_API Texture2D : public Texture {
		public:
			explicit Texture2D(uint32 access, SampleDesc sample_info = {});

			virtual ~Texture2D();

			// Returns the width of the texture.
			virtual uint16 GetWidth(uint8 level) const = 0;
			// Returns the height of the texture.
			virtual uint32 GetHeight(uint32 level) const = 0;

			virtual void Map(uint32 array_index, uint32 level, TextureMapAccess tma,
				uint32 x_offset, uint32 y_offset, uint32 width, uint32 height,
				void*& data, uint32& row_pitch) = 0;

			virtual void UnMap(uint32 array_index, uint32 level) = 0;

			virtual void CopyToSubTexture(Texture2D& target,
				uint32 dst_array_index, uint32 dst_level, uint32 dst_x_offset, uint32 dst_y_offset, uint32 dst_width, uint32 dst_height,
				uint32 src_array_index, uint32 src_level, uint32 src_x_offset, uint32 src_y_offset, uint32 src_width, uint32 src_height) = 0;
		protected:
			void Resize(uint32 dst_array_index, uint32 dst_level, uint32 dst_x_offset, uint32 dst_y_offset, uint32 dst_width, uint32 dst_height,
				uint32 src_array_index, uint32 src_level, uint32 src_x_offset, uint32 src_y_offset, uint32 src_width, uint32 src_height,
				bool linear);
		};

		class LE_API Texture3D : public Texture {
		public:
			explicit Texture3D(uint32 access, SampleDesc sample_info = {});

			virtual ~Texture3D();

			// Returns the width of the texture.
			virtual uint16 GetWidth(uint8 level) const = 0;
			// Returns the height of the texture.
			virtual uint32 GetHeight(uint32 level) const = 0;
			// Returns the depth of the texture (only for 3D texture).
			virtual uint32 Depth(uint32 level) const = 0;

			virtual void Map(uint32 array_index, uint32 level, TextureMapAccess tma,
				uint32 x_offset, uint32 y_offset, uint32 z_offset,
				uint32 width, uint32 height, uint32 depth,
				void*& data, uint32& row_pitch, uint32& slice_pitch) = 0;

			virtual void UnMap(uint32 array_index, uint32 level) = 0;

			virtual void CopyToSubTexture(Texture& target,
				uint32 dst_array_index, uint32 dst_level, uint32 dst_x_offset, uint32 dst_y_offset, uint32 dst_z_offset, uint32 dst_width, uint32 dst_height, uint32 dst_depth,
				uint32 src_array_index, uint32 src_level, uint32 src_x_offset, uint32 src_y_offset, uint32 src_z_offset, uint32 src_width, uint32 src_height, uint32 src_depth) = 0;
		protected:
			void ResizeTexture(uint32 dst_array_index, uint32 dst_level, uint32 dst_x_offset, uint32 dst_y_offset, uint32 dst_z_offset, uint32 dst_width, uint32 dst_height, uint32 dst_depth,
				uint32 src_array_index, uint32 src_level, uint32 src_x_offset, uint32 src_y_offset, uint32 src_z_offset, uint32 src_width, uint32 src_height, uint32 src_depth,
				bool linear);
		};

		class LE_API TextureCube : public Texture {
		public:
			explicit TextureCube(uint32 access, SampleDesc sample_info = {});

			virtual ~TextureCube();

			virtual void Map(uint32 array_index, CubeFaces face, uint32 level, TextureMapAccess tma,
				uint32 x_offset, uint32 y_offset, uint32 width, uint32 height,
				void*& data, uint32& row_pitch) = 0;

			virtual void UnMap(uint32 array_index, CubeFaces face, uint32 level) = 0;

			virtual void CopyToSubTexture(TextureCube& target,
				uint32 dst_array_index, CubeFaces dst_face, uint32 dst_level, uint32 dst_x_offset, uint32 dst_y_offset, uint32 dst_width, uint32 dst_height,
				uint32 src_array_index, CubeFaces src_face, uint32 src_level, uint32 src_x_offset, uint32 src_y_offset, uint32 src_width, uint32 src_height) = 0;

		protected:
			void Resize(uint32 dst_array_index, CubeFaces dst_face, uint32 dst_level, uint32 dst_x_offset, uint32 dst_y_offset, uint32 dst_width, uint32 dst_height,
				uint32 src_array_index, CubeFaces src_face, uint32 src_level, uint32 src_x_offset, uint32 src_y_offset, uint32 src_width, uint32 src_height,
				bool linear);
		};

		class LE_API Mapper : leo::noncopyable
		{
			friend class Texture;

		public:
			Mapper(Texture1D& tex, uint8 array_index, uint8 level, TextureMapAccess tma,
				uint16 x_offset, uint16 width)
				: TexRef(tex),
				MappedArrayIndex(array_index),
				MappedLevel(level),
				finally([=](Texture& TexRef) {
				static_cast<Texture1D&>(TexRef).UnMap(MappedArrayIndex, MappedLevel);
			}
					)
			{
				tex.Map(array_index, level, tma, x_offset, width, pSysMem);
				RowPitch = SlicePitch = width * NumFormatBytes(tex.GetFormat());
			}
			Mapper(Texture2D& tex, uint8 array_index, uint8 level, TextureMapAccess tma,
				uint16 x_offset, uint16 y_offset,
				uint16 width, uint16 height)
				: TexRef(tex),
				MappedArrayIndex(array_index),
				MappedLevel(level),
				finally([=](Texture& TexRef) {
				static_cast<Texture2D&>(TexRef).UnMap(MappedArrayIndex, MappedLevel);
			}
					)
			{
				tex.Map(array_index, level, tma, x_offset, y_offset, width, height, pSysMem, RowPitch);
				SlicePitch = RowPitch * height;
			}
			Mapper(Texture3D& tex, uint8 array_index, uint8 level, TextureMapAccess tma,
				uint16 x_offset, uint16 y_offset, uint32 z_offset,
				uint16 width, uint16 height, uint32 depth)
				: TexRef(tex),
				MappedArrayIndex(array_index),
				MappedLevel(level),
				finally([=](Texture& TexRef) {
				static_cast<Texture3D&>(TexRef).UnMap(MappedArrayIndex, MappedLevel);
			}
					)
			{
				tex.Map(array_index, level, tma, x_offset, y_offset, z_offset, width, height, depth, pSysMem, RowPitch, SlicePitch);
			}
			Mapper(TextureCube& tex, uint8 array_index, TextureCubeFaces face, uint8 level, TextureMapAccess tma,
				uint16 x_offset, uint16 y_offset,
				uint16 width, uint16 height)
				: TexRef(tex),
				MappedArrayIndex(array_index),
				MappedFace(face),
				MappedLevel(level),
				finally([=](Texture& TexRef) {
				static_cast<TextureCube&>(TexRef).UnMap(MappedArrayIndex, MappedFace, MappedLevel);
			}
					)
			{
				tex.Map(array_index, face, level, tma, x_offset, y_offset, width, height, pSysMem, RowPitch);
				SlicePitch = RowPitch * height;
			}

			~Mapper()
			{
				leo::invoke(finally,TexRef);
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

			uint32 GetRowPitch() const
			{
				return RowPitch;
			}

			uint32 GetSlicePitch() const
			{
				return SlicePitch;
			}
		public:
			void* pSysMem;
			uint32 RowPitch, SlicePitch;
		private:
			Texture& TexRef;

			uint8 MappedArrayIndex;
			TextureCubeFaces MappedFace;
			uint8 MappedLevel;

			std::function<void(Texture&)> finally;
		};

	}
}

#endif
