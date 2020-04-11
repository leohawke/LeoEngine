/*! \file Engine\Render\IDevice.h
\ingroup Engine
\brief 创建接口类。
*/
#ifndef LE_RENDER_IDevice_h
#define LE_RENDER_IDevice_h 1

#include <LBase/ldef.h>
#include "DeviceCaps.h"
#include "ITexture.hpp"
#include "IGraphicsBuffer.hpp"
#include "InputLayout.hpp"
#include "IGPUResourceView.h"
#include "PipleState.h"
#include "IShader.h"
#include "IGraphicsPipelineState.h"
#include "IShaderCompose.h"
#include "ShaderCore.h"
#include <optional>

namespace asset {
	class ShaderBlobAsset;
}

namespace platform::Render::Effect {
	class Effect;
	class Technique;
}

namespace platform::Render {

	class GraphicsPipelineStateInitializer
	{
	public:
		RasterizerDesc RasterizerState;
		DepthStencilDesc DepthStencilState;
		BlendDesc BlendState;

		PrimtivteType Primitive;

		ShaderPass* ShaderState;
	};


	class Device {
	public:
		virtual Caps& GetCaps() = 0;

		virtual Texture1D* CreateTexture(uint16 width, uint8 num_mipmaps, uint8 array_size,
			EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *> init_data = nullptr) = 0;

		virtual Texture2D* CreateTexture(uint16 width, uint16 height, uint8 num_mipmaps, uint8 array_size,
			EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data = nullptr) = 0;

		virtual Texture3D* CreateTexture(uint16 width, uint16 height, uint16 depth, uint8 num_mipmaps, uint8 array_size,
			EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data = nullptr) = 0;

		virtual TextureCube* CreateTextureCube(uint16 size, uint8 num_mipmaps, uint8 array_size,
			EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data = nullptr) = 0;

		virtual ShaderCompose* CreateShaderCompose(std::unordered_map<ShaderType,const asset::ShaderBlobAsset*> pShaderBlob, Effect::Effect* pEffect) = 0;

		virtual GraphicsBuffer* CreateConstanBuffer(Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const *>  init_data = nullptr) = 0;
		virtual GraphicsBuffer* CreateVertexBuffer(Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const *>  init_data = nullptr) = 0;
		virtual GraphicsBuffer* CreateIndexBuffer(Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const *>  init_data = nullptr) = 0;

		virtual PipleState* CreatePipleState(const PipleState& state) = 0;

		virtual InputLayout* CreateInputLayout() = 0;

		virtual UnorderedAccessView* CreateUnorderedAccessView(Texture2D* InTexture) =0;

		virtual GraphicsPipelineState* CreateGraphicsPipelineState(const GraphicsPipelineStateInitializer& initializer) =0;
	};
}

#endif