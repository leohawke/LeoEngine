/*! \file Engine\Render\IGraphicsBuffer.hpp
\ingroup Engine
\brief Buffer½Ó¿ÚÀà¡£
*/
#ifndef LE_RENDER_IGraphicsBuffer_hpp
#define LE_RENDER_IGraphicsBuffer_hpp 1

#include <LBase/linttype.hpp>
#include <LBase/sutility.h>
#include <LBase/lmacro.h>
#include "RenderObject.h"
#include "ShaderParametersMetadata.h"
namespace platform::Render {
	namespace Buffer {
		enum  Usage
		{
			Static = 0x0001,
			Dynamic = 0x0002,

			AccelerationStructure = 0x8000,

			SingleDraw = 0x8001,
			SingleFrame = 0x8002,
			MultiFrame = 0x8003,
		};

		enum Access {
			Read_Only,
			Write_Only,
			Read_Write,
			Write_No_Overwrite
		};

		class Mapper;
	}

	class GraphicsBuffer:public RObject {
	public:
		virtual ~GraphicsBuffer();


		DefGetter(const lnothrow, leo::uint32, Size, size_in_byte)

			DefGetter(const lnothrow, Buffer::Usage, Usage, usage)

			DefGetter(const lnothrow, leo::uint32, Access, access)


			virtual void CopyToBuffer(GraphicsBuffer& rhs) = 0;

		virtual void HWResourceCreate(void const * init_data) = 0;
		virtual void HWResourceDelete() = 0;

		virtual void UpdateSubresource(leo::uint32 offset, leo::uint32 size, void const * data) = 0;
	protected:
		GraphicsBuffer(Buffer::Usage usage, leo::uint32 access, leo::uint32 size_in_byte);

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
			friend class ::platform::Render::GraphicsBuffer;

		public:
			Mapper(GraphicsBuffer& InBuffer, Access ba)
				: buffer(InBuffer)
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

	GraphicsBuffer* CreateConstantBuffer(const void* Contents, Buffer::Usage Usage,const ShaderParametersMetadata& Layout);

	template<typename TBufferStruct>
	requires requires{ TBufferStruct::TypeInfo::GetStructMetadata(); }
	class GraphicsBufferRef
	{
	public:
		static GraphicsBufferRef<TBufferStruct> CreateGraphicsBuffeImmediate(const TBufferStruct& Value, Buffer::Usage Usage)
		{
			return GraphicsBufferRef<TBufferStruct>(CreateConstantBuffer(&Value, Usage, *TBufferStruct::TypeInfo::GetStructMetadata()));
		}

		operator std::shared_ptr<GraphicsBuffer>()
		{
			return buffer;
		}

		GraphicsBuffer* Get() const
		{
			return buffer.get();
		}
	private:
		GraphicsBufferRef(GraphicsBuffer* InBuffer)
			:buffer(InBuffer,RObjectDeleter())
		{
		}

		std::shared_ptr<GraphicsBuffer> buffer;
	};

	template<typename TBufferStruct>
	requires requires{ TBufferStruct::TypeInfo::GetStructMetadata(); }
	GraphicsBufferRef<TBufferStruct> CreateGraphicsBuffeImmediate(const TBufferStruct& Value, Buffer::Usage Usage)
	{
		return GraphicsBufferRef<TBufferStruct>::CreateGraphicsBuffeImmediate(Value, Usage);
	}
}

#endif