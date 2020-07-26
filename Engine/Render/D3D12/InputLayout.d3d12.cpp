#include "InputLayout.hpp"
#include "Convert.h"
#include "GraphicsBuffer.hpp"
#include "Context.h"
namespace platform_ex::Windows::D3D12 {

	InputLayout::InputLayout() = default;

	const std::vector<D3D12_INPUT_ELEMENT_DESC>& InputLayout::GetInputDesc() const
	{
		if (vertex_elems.empty()) {
			std::vector<D3D12_INPUT_ELEMENT_DESC> elems;

			WORD input_slot = 0;
			for (auto &vertex_stream : vertex_streams) {
				auto stream_elems = Convert(vertex_stream);
				for (auto& elem : stream_elems)
					elem.InputSlot = input_slot;
				++input_slot;
				elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
			}

			vertex_elems.swap(elems);
		}
		return vertex_elems;
	}

	platform::Render::VertexDeclarationElements InputLayout::GetVertexDeclaration() const
	{
		platform::Render::VertexDeclarationElements elems;
		WORD input_slot = 0;

		for (auto& vertex_stream : vertex_streams) {
			uint16 elem_offset = 0;
			for (auto& element : vertex_stream.elements)
			{
				platform::Render::VertexElement ve;
				ve.Format = element.format;
				ve.Offset = elem_offset;
				ve.StreamIndex = input_slot;
				ve.Stride = vertex_stream.vertex_size;
				ve.Usage = element.usage;
				ve.UsageIndex = element.usage_index;
				elem_offset += element.GetElementSize();

				elems.emplace_back(ve);
			}
			++input_slot;
		}

		return elems;
	}

	void InputLayout::Active() const
	{
	}
}