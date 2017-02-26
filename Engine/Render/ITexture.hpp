/*! \file Engine\Render\IFormat.hpp
\ingroup Engine
\brief GPU纹理。
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
			enum class MapAccess {
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

			enum class CubeFaces : uint8 {
				Positive_X = 0,
				Negative_X = 1,
				Positive_Y = 2,
				Negative_Y = 3,
				Positive_Z = 4,
				Negative_Z = 5,
			};

			friend uint8 operator+(const uint8& lhs,const CubeFaces& face) {
				return lhs + static_cast<uint8>(face);
			}
			friend uint8 operator-(const uint8& lhs, const CubeFaces& face) {
				return lhs - static_cast<uint8>(face);
			}
		protected:
			//\brief encode = UTF-8
			virtual std::string  Description() const = 0;
		public:
			explicit Texture(Type type, uint8 numMipMaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info);

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

			

			virtual void BuildMipSubLevels() = 0;

			virtual void HWResourceCreate(ElementInitData const *  init_data) = 0;
			virtual void HWResourceDelete() = 0;
			virtual bool HWResourceReady() const = 0;
		protected:
			uint8		mipmap_size;
			uint8		array_size;

			EFormat	format;
			Type		type;
			SampleDesc		sample_info;
			uint32		access_mode;
		};

		using TextureMapAccess = Texture::MapAccess;
		using TextureType = Texture::Type;
		using TextureCubeFaces = Texture::CubeFaces;

		class LE_API Texture1D : public Texture {
		public:
			explicit Texture1D(uint8 numMipMaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info);

			virtual ~Texture1D();

			// Returns the width of the texture.
			virtual uint16 GetWidth(uint8 level) const = 0;

			struct Sub1D {
				uint8 array_index;
				uint8 level;

				Sub1D(uint8 arrary_index_,uint8 level_)
					:array_index(array_index),level(level_)
				{}

				Sub1D() = default;
			};

			struct Box1D : Sub1D {
				uint16 x_offset;
				uint16 width;

				Box1D(const Sub1D& sub,uint16 xoffset_,uint16 width_)
					:Sub1D(sub),x_offset(xoffset_),width(width_)
				{}

				Box1D(uint8 array_index_, uint8 level_, uint16 xoffset_, uint16 width_)
					:Sub1D(array_index_,level_), x_offset(xoffset_), width(width_)
				{}

				explicit Box1D(uint16 width_)
					:Sub1D(),x_offset(0),width(width_)
				{}
			};

			virtual void Map(TextureMapAccess tma,
				void*& data,const Box1D& box) = 0;
			virtual void UnMap(const Sub1D& sub) = 0;

			virtual void CopyToTexture(Texture1D& target) = 0;

			virtual void CopyToSubTexture(Texture1D& target,const Box1D& dst,const Box1D& src) = 0;
		protected:
			virtual void Resize(Texture1D& target,const Box1D& dst,
				const Box1D& src,
				bool linear) = 0;

			void Resize(const Box1D& dst,
				const Box1D& src,
				bool linear);
		};

		class LE_API Texture2D : public Texture {
		public:
			explicit Texture2D(uint8 numMipMaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info);

			virtual ~Texture2D();

			// Returns the width of the texture.
			virtual uint16 GetWidth(uint8 level) const = 0;
			// Returns the height of the texture.
			virtual uint16 GetHeight(uint8 level) const = 0;

			using Sub1D = Texture1D::Sub1D;
			struct Box2D :Texture1D::Box1D {
				uint16 y_offset;
				uint16 height;

				Box2D(const Box1D& box, uint16 yoffset_, uint16 height_)
					:Box1D(box), y_offset(yoffset_), height(height_)
				{}

				explicit Box2D(uint16 width_,uint16 height_)
					:Box1D(width_),height(height_)
				{}

				Box2D(uint8 array_index_,uint8 level_,uint16 x_offset_,uint16 y_offset_,uint16 width_,uint16 height_)
					:Box1D({array_index_,level_},x_offset_,width_),y_offset(y_offset_),height(height_)
				{}
			};

			virtual void Map(TextureMapAccess tma,
				void*& data, uint32& row_pitch,const Box2D&) = 0;

			virtual void UnMap(const Sub1D&) = 0;

			virtual void CopyToTexture(Texture2D& target) = 0;
			
			virtual void CopyToSubTexture(Texture2D& target,
				const Box2D& dst,
				const Box2D& src) = 0;
		protected:
			virtual void Resize(Texture2D& target,const Box2D& dst,
				const Box2D& src,
				bool linear) = 0;

			void Resize(const Box2D& dst,
				const Box2D& src,
				bool linear);
		};

		class LE_API Texture3D : public Texture {
		public:
			explicit Texture3D(uint8 numMipMaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info);

			virtual ~Texture3D();

			// Returns the width of the texture.
			virtual uint16 GetWidth(uint8 level) const = 0;
			// Returns the height of the texture.
			virtual uint16 GetHeight(uint8 level) const = 0;
			// Returns the depth of the texture.
			virtual uint16 GetDepth(uint8 level) const = 0;

			using Sub1D = Texture1D::Sub1D;
			struct Box3D :Texture2D::Box2D {
				uint16 z_offset;
				uint16 depth;

				Box3D(const Box2D& box, uint16 zoffset_, uint16 depth_)
					:Box2D(box), z_offset(zoffset_), depth(depth_)
				{}

				Box3D(uint8 array_index_, uint8 level_, 
					uint16 x_offset_, uint16 y_offset_, uint16 z_offset_, 
					uint16 width_, uint16 height_, uint16 depth_)
					:Box2D(array_index_,level_ , x_offset_,y_offset_ ,width_,height_), z_offset(z_offset_), depth(depth_)
				{}
			};

			virtual void Map(TextureMapAccess tma,
				void*& data, uint32& row_pitch, uint32& slice_pitch,const Box3D&) = 0;

			virtual void UnMap(const Sub1D&) = 0;

			virtual void CopyToTexture(Texture3D& target) = 0;
			
			virtual void CopyToSubTexture(Texture3D& target,
				const Box3D& dst,
				const Box3D& src) = 0;
		protected:
			virtual void Resize(Texture3D& target,const Box3D& dst,
				const Box3D& src,
				bool linear) = 0;

			void Resize(const Box3D& dst,
				const Box3D& src,
				bool linear);
		};

		class LE_API TextureCube : public Texture {
		public:
			explicit TextureCube(uint8 numMipMaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info);

			virtual ~TextureCube();

			// Returns the width of the texture.
			virtual uint16 GetWidth(uint8 level) const = 0;
			// Returns the height of the texture.
			virtual uint16 GetHeight(uint8 level) const = 0;

			using Sub1D = Texture1D::Sub1D;
			struct BoxCube:Texture2D::Box2D{
				CubeFaces face;

				BoxCube(const Box2D& box,CubeFaces face_)
					:Box2D(box),face(face)
				{}
			};

			virtual void Map(TextureMapAccess tma,
				void*& data, uint32& row_pitch,const BoxCube&) = 0;

			virtual void UnMap(const Sub1D&,CubeFaces face) = 0;

			virtual void CopyToTexture(TextureCube& target) = 0;
			
			virtual void CopyToSubTexture(TextureCube& target,
				const BoxCube& dst,
				const BoxCube& src) = 0;

		protected:
			virtual void Resize(TextureCube& target,const BoxCube&,
				const BoxCube& dst,
				bool linear) = 0;

			void Resize(const BoxCube&,
				const BoxCube&,
				bool linear);
		};

		class LE_API Mapper : leo::noncopyable
		{
			friend class Texture;

		public:
			Mapper(Texture1D& tex,TextureMapAccess tma,const Texture1D::Box1D& box)
				: TexRef(tex),
			
				finally([=](Texture& TexRef) {
				static_cast<Texture1D&>(TexRef).UnMap(box);
			}
					)
			{
				tex.Map(tma, pSysMem,box);
				RowPitch = SlicePitch = box.width * NumFormatBytes(tex.GetFormat());
			}
			Mapper(Texture2D& tex,TextureMapAccess tma,
				const Texture2D::Box2D& box)
				: TexRef(tex),
				
				finally([=](Texture& TexRef) {
				static_cast<Texture2D&>(TexRef).UnMap(box);
			}
					)
			{
				tex.Map(tma, pSysMem, RowPitch,box);
				SlicePitch = RowPitch * box.height;
			}
			Mapper(Texture3D& tex, TextureMapAccess tma,
				const Texture3D::Box3D& box)
				: TexRef(tex),
				
				finally([=](Texture& TexRef) {
				static_cast<Texture3D&>(TexRef).UnMap(box);
			}
					)
			{
				tex.Map(tma,pSysMem, RowPitch, SlicePitch,box);
			}
			Mapper(TextureCube& tex,TextureMapAccess tma,
				const TextureCube::BoxCube& box)
				: TexRef(tex),
				finally([=](Texture& TexRef) {
				static_cast<TextureCube&>(TexRef).UnMap(box,box.face);
			}
					)
			{
				tex.Map(tma, pSysMem, RowPitch,box);
				SlicePitch = RowPitch * box.height;
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

			std::function<void(Texture&)> finally;
		};

	}
}

#endif
