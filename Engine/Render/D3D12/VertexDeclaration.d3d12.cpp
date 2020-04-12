#include "VertexDeclaration.h"
#include "Convert.h"
using namespace platform_ex::Windows::D3D12;

namespace std
{
	template<>
	struct hash< D3D12_INPUT_ELEMENT_DESC>
	{
		std::size_t operator()(const D3D12_INPUT_ELEMENT_DESC& desc) const
		{
			static_assert(sizeof(desc) % sizeof(unsigned int) == 0);

			auto first = reinterpret_cast<const unsigned int*>(&desc);
			auto last = first + sizeof(desc)/sizeof(unsigned int);
			return leo::hash(first, last);
		}
	};
}

struct VertexDeclarationKey
{
	VertexElementsType VertexElements;

	uint32 Hash;

	explicit VertexDeclarationKey(const platform::Render::VertexDeclarationElements& InElements)
	{
		for (unsigned ElementIndex = 0; ElementIndex != InElements.size(); ++ElementIndex)
		{
			auto& Element = InElements[ElementIndex];

			D3D12_INPUT_ELEMENT_DESC D3DElement  = { 0 };

			D3DElement.InputSlot = Element.StreamIndex;
			D3DElement.AlignedByteOffset = Element.Offset;

			D3DElement.Format = Convert(Element.Format);
			D3DElement.SemanticName = SemanticName(Element.Usage);
			D3DElement.SemanticIndex = Element.UsageIndex;

			//todo:support instance
			D3DElement.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			D3DElement.InstanceDataStepRate = 0;

			VertexElements.emplace_back(D3DElement);
		}

		constexpr unsigned StreamLimitSize = std::numeric_limits<std::uint16_t>::max();

		std::sort(VertexElements.begin(), VertexElements.end(), [](const D3D12_INPUT_ELEMENT_DESC& A, const D3D12_INPUT_ELEMENT_DESC& B)
			{
				return (A.AlignedByteOffset + A.InputSlot * StreamLimitSize) < (B.AlignedByteOffset + B.InputSlot * StreamLimitSize);
			});

		Hash = leo::hash(VertexElements.begin(), VertexElements.end()) & 0XFFFFFFFF;
	}
};

VertexDeclaration* platform_ex::Windows::D3D12::CreateVertexDeclaration(const platform::Render::VertexDeclarationElements& Elements)
{
	return nullptr;
}

const char* platform_ex::Windows::D3D12::SemanticName(platform::Render::Vertex::Usage usage)
{
	using namespace platform::Render::Vertex;

	switch (usage) {
	case Usage::Position:
		return "POSITION";
	case Usage::Normal:
		return "NORMAL";
	case Usage::Binoraml:
		return "BINORMAL";
	case Usage::Tangent:
		return "TANGENT";

	case Usage::Diffuse:
	case Usage::Specular:
		return "COLOR";

	case Usage::BlendIndex:
		return "BLENDINDEX";

	case Usage::BlendWeight:
		return "BLENDWEIGHT";

	case Usage::TextureCoord:
		return "TEXCOORD";
	}

	return "ATTRIBUTE";
}
