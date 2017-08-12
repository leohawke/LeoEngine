#include <LBase/Win32/Mingw32.h>
#include "Render/D3D12/test.h"
#include "Render/D3D12/Context.h"
#include "Asset/TextureX.h"


#define TEST_CODE 1

#if TEST_CODE
HWND g_hwnd = NULL;
#endif
IDXGISwapChain* LE_API Create(HWND hwnd) {
#if TEST_CODE
	g_hwnd = hwnd;
#endif

	using namespace platform::Descriptions;
	using namespace leo::inttype;
	try {
		LCL_CallF_Win32(LoadLibraryW, L"d3d12.dll");
		LCL_CallF_Win32(LoadLibraryW, L"dxgi.dll");

		platform_ex::Windows::D3D12::Context::Instance();
	}
	CatchRet(platform_ex::Windows::Win32Exception&, TraceDe(Warning, "d3d12 win32 op failed."), nullptr)
	CatchRet(platform_ex::COMException&, TraceDe(Warning, "d3d12 com op failed."), nullptr)

	

	return nullptr;
}

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {
			using namespace platform::Descriptions;







			//\brief Device
		}
	}
}