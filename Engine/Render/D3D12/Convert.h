/*! \file Engine\Render\D3D12\Convert.h
\ingroup Engine
\brief 把上层转换至D3D12层。
*/

#ifndef LE_RENDER_D3D12_Convert_h
#define LE_RENDER_D3D12_Convert_h 1

#include "../IFormat.hpp"
#include <dxgiformat.h>

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {
			DXGI_FORMAT Convert(platform::Render::EFormat format);
			platform::Render::EFormat Convert(DXGI_FORMAT format);
		}
	}
}

#endif