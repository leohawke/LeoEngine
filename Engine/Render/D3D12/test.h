
#include <LBase/NativeAPI.h>
#include <LBase/linttype.hpp>
#include <LBase/memory.hpp>

#include <LBase/FContainer.h>

#include "../../emacro.h"
#include "../../Win32/COM.h"

#include "Adapter.h"

#include "d3d12_dxgi.h"


namespace platform {
	namespace Window {
		enum WindowRotation {
			WR_Rotate90,
			WR_Rotate270
		};
	}

	namespace Render {
		
	}
}

namespace platform_ex {
	 namespace Windows {
		namespace D3D12 {
		}
	}
}

IDXGISwapChain* LE_API Create(HWND hwnd);