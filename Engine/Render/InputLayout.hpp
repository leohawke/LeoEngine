/*! \file Engine\Render\InputLayout.hpp
\ingroup Engine\Render
\输入的布局信息 包括 顶点流布局信息 索引信息
*/
#pragma once
#ifndef LE_RENDER_INPUTLATOYT_HPP
#define LE_RENDER_INPUTLATOYT_HPP 1

#include <LBase/linttype.hpp>
#include <LBase/lmacro.h>
#include <LBase/memory.hpp>
#include <vector>
#include "IGraphicsBuffer.hpp"

#include "IFormat.hpp"

namespace platform::Render {
	namespace Vertex {
		enum Usage {
			Position,//位置

			Normal,//法线
			Tangent,
			Binoraml,

			Diffuse,//顶点色
			Specular,//顶点高光
			BlendWeight,
			BlendIndex,

			TextureCoord,

		};

		struct Element {
			Element() = default;

			Element(Usage usage_, leo::uint8 index, EFormat format_)
				:usage(usage_), usage_index(index), format(format_)
			{}

			DefGetter(const lnothrow, uint16, ElementSize, NumFormatBytes(format))

			friend bool operator==(const Element& lhs, const Element& rhs) {
				return lhs.format == rhs.format &&
					lhs.usage == rhs.usage &&
					lhs.usage_index == rhs.usage_index;
			}

			Usage usage;
			leo::uint8 usage_index;
			EFormat format;
		};

		struct Stream {
			enum Type {
				Geomerty,
				//Instance,
			} type;

			std::shared_ptr<GraphicsBuffer> stream;
			std::vector<Element> elements;
			uint32 vertex_size;

			leo::uint32 instance_freq;
		};

		enum InputLayoutFormat :uint8 {
			Empty,

			P3F_N3F_T2F	= 1, //!1.4 
			P3F_N4B_T2F,	//!TODO
		};
	}

	enum class PrimtivteType
	{
		TriangleList,
		TriangleStrip,
		LineList,
		PointList,

		ControlPoint_0,
		ControlPoint_1 = ControlPoint_0 + 1,
		ControlPoint_32 = ControlPoint_0+32,
	};

	inline uint32 GetPrimitiveTypeFactor(PrimtivteType type)
	{
		switch (type)
		{
		case platform::Render::PrimtivteType::TriangleList:
			return 3;
		case platform::Render::PrimtivteType::TriangleStrip:
			return 1;
		case platform::Render::PrimtivteType::LineList:
			return 2;
		case platform::Render::PrimtivteType::PointList:
			return 1;
		default:
			break;
		}

		return type >= PrimtivteType::ControlPoint_1 ? (uint32)type - (uint32)PrimtivteType::ControlPoint_0 : 1;
	}

	struct VertexElement
	{
		uint8 StreamIndex;
		uint8 Offset;
		Vertex::Usage Usage;
		leo::uint8 UsageIndex;
		EFormat Format;
		uint16 Stride;
	};

	using VertexDeclarationElements = std::vector<VertexElement>;

	constexpr unsigned MaxVertexElementCount = 16;

	//fixme:error class name
	class InputLayout :public leo::noncopyable {
	public:
		InputLayout();
		virtual ~InputLayout();

		DefGetter(const lnothrow, PrimtivteType, TopoType, topology_type)
			DefSetter(, PrimtivteType, TopoType, topology_type)

		uint32 GetNumVertices() const lnothrow;

		void BindVertexStream(const std::shared_ptr<GraphicsBuffer> & buffer, std::initializer_list<Vertex::Element> elements, Vertex::Stream::Type type = Vertex::Stream::Geomerty, uint32 instance_freq = 1);

		DefGetter(const lnothrow, uint32, VertexStreamsSize, static_cast<uint32>(vertex_streams.size()))


		const Vertex::Stream& GetVertexStream(uint32 index) const;

		Vertex::Stream& GetVertexStreamRef(uint32 index);

		void BindIndexStream(const std::shared_ptr<GraphicsBuffer> & buffer, EFormat format);

		DefGetter(const lnothrow, const std::shared_ptr<GraphicsBuffer>&, IndexStream, index_stream)
		DefGetter(const lnothrow,EFormat,IndexFormat,index_format)

		uint32 GetNumIndices() const;


		DefGetter(const lnothrow, uint32, VertexStart, start_vertex_location)
		DefGetter(lnothrow, uint32&, VertexStartRef, start_vertex_location)

		DefGetter(const lnothrow, uint32, IndexStart, start_index_location)
		DefGetter(lnothrow, uint32&, IndexStartRef, start_index_location)
	protected:
		void Dirty();

	protected:
		PrimtivteType topology_type;

		std::vector<Vertex::Stream> vertex_streams;
		//Vertex::Stream instance_stream;

		std::shared_ptr<GraphicsBuffer> index_stream;
		EFormat index_format;

		uint32 start_vertex_location;
		uint32 start_index_location;

		//int32 base_vertex_location;
		//uint32 start_instance_location;

		bool stream_dirty;
	};
}

#endif