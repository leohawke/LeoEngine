#include "emacro.h"
#include "Render/D3D12/d3d12_dxgi.h"

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