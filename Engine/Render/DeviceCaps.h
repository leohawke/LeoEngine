/*! \file Engine\Render\DeviceCaps.h
\ingroup Engine
\brief 设备能力集。
*/
#ifndef LE_RENDER_DeviceCaps_h
#define LE_RENDER_DeviceCaps_h 1

#include "IFormat.hpp"

#include <functional>

namespace platform{
	namespace Render {
		struct Caps {
			enum class Type {
				D3D12
			};

			Type type;

			uint16 max_texture_depth;

			std::function<bool(EFormat)> TextureFormatSupport;
			std::function<bool(EFormat)> VertexFormatSupport;
			std::function<bool(EFormat, SampleDesc)>	RenderTargetMSAASupport;
		};
	}
}

#endif