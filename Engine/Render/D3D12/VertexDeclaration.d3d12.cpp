#include "VertexDeclaration.h"

using namespace platform_ex::Windows::D3D12;

struct VertexDeclarationKey
{
	VertexElementsType VertexElements;

	explicit VertexDeclarationKey(const platform::Render::VertexDeclarationElements& InElements)
	{

	}
};

VertexDeclaration* platform_ex::Windows::D3D12::CreateVertexDeclaration(const platform::Render::VertexDeclarationElements& Elements)
{
	return nullptr;
}
