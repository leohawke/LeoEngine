#include "InputLayout.hpp"

namespace platform::Render {
	InputLayout::InputLayout()
		:topology_type(PrimtivteType::TriangleList),
		index_format(EF_Unknown),
		start_vertex_location(0),
		start_index_location(0),
		stream_dirty(true)
	{
		vertex_streams.reserve(4);
	}

	InputLayout::~InputLayout() = default;

	void InputLayout::BindVertexStream(const std::shared_ptr<GraphicsBuffer>& buffer, std::initializer_list<Vertex::Element> elements, Vertex::Stream::Type type, uint32 instance_freq)
	{
		uint32 size = 0;
		for (auto & element : elements)
			size += element.GetElementSize();

		if (type == Vertex::Stream::Geomerty) {
			for (auto & vertex_stream : vertex_streams) {
				if (std::equal(vertex_stream.elements.begin(),vertex_stream.elements.end(),elements.begin())){
					vertex_stream.stream = buffer;
					vertex_stream.vertex_size = size;
					vertex_stream.type = type;
					vertex_stream.instance_freq = instance_freq;
					Dirty();
					return;
				}
			}

			Vertex::Stream vertex_stream;
			vertex_stream.stream = buffer;
			vertex_stream.vertex_size = size;
			vertex_stream.type = type;
			vertex_stream.instance_freq = instance_freq;
			vertex_stream.elements.assign(elements);
			vertex_streams.emplace_back(std::move(vertex_stream));
		}
	}

	VertexDeclarationElements InputLayout::GetVertexDeclaration() const
	{
		if (!vertex_decl.empty() && vertex_decl.back().StreamIndex == static_cast<uint8> (vertex_streams.size() - 1))
			return vertex_decl;
		else
		{
			auto& elems = vertex_decl;
			uint8 input_slot = 0;

			for (auto& vertex_stream : vertex_streams) {
				uint16 elem_offset = 0;
				for (auto& element : vertex_stream.elements)
				{
					platform::Render::VertexElement ve;
					ve.Format = element.format;
					ve.Offset = static_cast<uint8>(elem_offset);
					ve.StreamIndex = input_slot;
					ve.Stride = vertex_stream.vertex_size;
					ve.Usage = element.usage;
					ve.UsageIndex = element.usage_index;
					elem_offset += element.GetElementSize();

					elems.emplace_back(ve);
				}
				++input_slot;
			}

			return vertex_decl;
		}
	}

	const Vertex::Stream & InputLayout::GetVertexStream(uint32 index) const
	{
		return vertex_streams.at(index);
	}

	Vertex::Stream & InputLayout::GetVertexStreamRef(uint32 index)
	{
		return vertex_streams.at(index);
	}

	void InputLayout::BindIndexStream(const std::shared_ptr<GraphicsBuffer>& buffer, EFormat format)
	{
		index_stream = buffer;
		index_format = format;
	}

	uint32 InputLayout::GetNumIndices() const
	{
		if (index_stream)
			return index_stream->GetSize() / NumFormatBytes(index_format);
		return uint32();
	}

	void InputLayout::Dirty()
	{
		stream_dirty = true;
	}

	
	uint32 InputLayout::GetNumVertices() const lnothrow
	{
		return vertex_streams.at(0).stream->GetSize() / vertex_streams.at(0).vertex_size;
	}
}