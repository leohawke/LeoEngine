
#pragma warning(push)
#pragma warning(disable:4005)
#include "D3D11RenderSystem.hpp"
#include "D3D11Texture.hpp"
#pragma warning(pop)


leo::TexturePtr leo::D3D11RenderFactory::MakeTexture1D(uint16 width, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access, ElementInitData const *init_data)
{
	return std::make_shared<D3D11Texture1D>(width, numMipMaps, array_size, format, access, init_data);
}

leo::TexturePtr leo::D3D11RenderFactory::MakeTexture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size, EFormat format, SampleDesc sample_info, uint32 access, ElementInitData const *init_data)
{
	return std::make_shared<D3D11Texture2D>(width, height, numMipMaps, array_size, format, access, sample_info, init_data);
}

leo::TexturePtr leo::D3D11RenderFactory::MakeTextureCube(uint16 size, uint8 numMipMaps, uint8 array_size,
	EFormat format, uint32_t access, ElementInitData const * init_data) {
	return std::make_shared<D3D11TextureCube>(size,numMipMaps, array_size, format, access, init_data);
}

D3D_FEATURE_LEVEL leo::D3D11Engine::GetCoreFeatureLevel()
{
	return D3D_FEATURE_LEVEL_11_0;
}

leo::RenderEngine& leo::GetRenderEngine() {
	static D3D11Engine mD3D11Engine;
	return mD3D11Engine;
}



