#include "TextureX.hpp"
#include "RenderSystem.hpp"
namespace leo {
	namespace X {
		TexturePtr MakeTexture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size, EFormat format, SampleDesc sample_info, uint32 access, ElementInitData init_data)
		{
			return GetRenderEngine().GetFactory().MakeTexture2D(
				width, height, 
				numMipMaps, array_size, 
				format, sample_info, access, 
				init_data);
		}
	}

}