/*! \file Engine\Render\IDevice.h
\ingroup Engine
\brief 创建接口类。
*/
#ifndef LE_RENDER_IDevice_h
#define LE_RENDER_IDevice_h 1

#include "DeviceCaps.h"
#include "ITexture.hpp"
#include "Effect/Effect.hpp"

namespace asset {
	class ShaderBlobAsset;
}

namespace platform {
	namespace Render {
		class Device {
		public:
			virtual Caps& GetCaps() = 0;

			virtual Texture1D* CreateTexture(uint16 width, uint8 num_mipmaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info,std::optional<ElementInitData const *> init_data = nullptr) = 0;

			virtual Texture2D* CreateTexture(uint16 width, uint16 height, uint8 num_mipmaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data = nullptr) = 0;

			virtual Texture3D* CreateTexture(uint16 width, uint16 height, uint16 depth, uint8 num_mipmaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data = nullptr) = 0;

			virtual TextureCube* CreateTextureCube(uint16 size, uint8 num_mipmaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data = nullptr) = 0;
		
			virtual ShaderCompose* CreateShaderCompose(std::unordered_map<ShaderCompose::Type, leo::observer_ptr<const asset::ShaderBlobAsset>> pShaderBlob, leo::observer_ptr<Effect::Effect> pEffect) = 0;
		};
	}
}

#endif