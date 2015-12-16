#include "D3D11RenderSystem.hpp"

leo::TexturePtr leo::D3D11RenderFactory::MakeTexture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size, EFormat format, SampleDesc sample_info, uint32 access, uint8 const * init_data)
{
	return TexturePtr();
}

D3D_FEATURE_LEVEL leo::D3D11Engine::GetCoreFeatureLevel()
{
	return D3D_FEATURE_LEVEL_11_0;
}

leo::RenderEngine& leo::GetRenderEngine() {
	static D3D11Engine mD3D11Engine;
	return mD3D11Engine;
}



