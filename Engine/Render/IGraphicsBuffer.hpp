/*! \file Engine\Render\IGraphicsBuffer.hpp
\ingroup Engine
\brief Buffer½Ó¿ÚÀà¡£
*/
#ifndef LE_RENDER_IGraphicsBuffer_hpp
#define LE_RENDER_IGraphicsBuffer_hpp 1

#include <LBase/linttype.hpp>
#include <LBase/sutility.h>
#include <LBase/lmacro.h>
namespace platform {
	namespace Render {
		namespace Buffer {
			enum  Usage
			{
				Static,
				Dynamic
			};

			enum Access {
				Read_Only,
				Write_Only,
				Read_Write,
				Write_No_Overwrite
			};
			
			class Mapper;
		}

		class GraphicsBuffer {
		public:
			GraphicsBuffer(Buffer::Usage usage, leo::uint32 access, leo::uint32 size_in_byte);
			virtual ~GraphicsBuffer();


			DefGetter(const lnothrow, leo::uint32, Size, size_in_byte)
			
			DefGetter(const lnothrow, Buffer::Usage, usage)

			DefGetter(const lnothrow,leo::uint32,Access, access)
			

			virtual void CopyToBuffer(GraphicsBuffer& rhs) = 0;

			virtual void HWResourceCreate(void const * init_data) = 0;
			virtual void HWResourceDelete() = 0;

			virtual void UpdateSubresource(leo::uint32 offset, leo::uint32 size, void const * data) = 0;

		private:
			virtual void* Map(Buffer::Access ba) = 0;
			virtual void Unmap() = 0;

			friend class Buffer::Mapper;
		protected:
			Buffer::Usage usage;
			leo::uint32 access;

			leo::uint32 size_in_byte;
		};

		namespace Buffer {
			class Mapper : leo::noncopyable
			{
				friend class GraphicsBuffer;

			public:
				Mapper(GraphicsBuffer& buffer_,Access ba)
					: buffer(buffer)
				{
					data = buffer.Map(ba);
				}
				~Mapper()
				{
					buffer.Unmap();
				}

				template <typename T>
				const T* Pointer() const
				{
					return static_cast<T*>(data);
				}
				template <typename T>
				T* Pointer()
				{
					return static_cast<T*>(data);
				}

			private:
				GraphicsBuffer& buffer;
				void* data;
			};
		}
	}
}

#endif