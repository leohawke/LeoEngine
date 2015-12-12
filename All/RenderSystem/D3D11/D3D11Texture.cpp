#include "D3D11Texture.hpp"

namespace leo {
	D3D11Texture::D3D11Texture(Dis_Type type, uint32 access, SampleDesc sample_info)
		:Texture(type,access,sample_info)
	{
	}
	ImplDeDtor(D3D11Texture)
}