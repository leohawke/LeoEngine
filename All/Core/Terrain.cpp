#include "Terrain.hpp"

namespace leo
{
	namespace InputLayoutDesc
	{
		extern const D3D11_INPUT_ELEMENT_DESC Terrain[1] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R16G16_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	}
}