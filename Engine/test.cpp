#include <LBase/Win32/Mingw32.h>
#include "Render/D3D12/test.h"
#include "Render/D3D12/Context.h"
#include "Render/D3D12/RenderView.h"

#define TEST_CODE 1

#if TEST_CODE
HWND g_hwnd = NULL;
#endif
IDXGISwapChain* LE_API Create(HWND hwnd) {
#if TEST_CODE
	g_hwnd = hwnd;
#endif

	using namespace platform::Descriptions;
	try {
		LFL_CallWin32F(LoadLibraryW, L"d3d12.dll");
		LFL_CallWin32F(LoadLibraryW, L"dxgi.dll");

		platform_ex::Windows::D3D12::Context::Instance();
	}
	CatchRet(platform_ex::Win32Exception&, TraceDe(Warning, "d3d12 win32 op failed."), nullptr)
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