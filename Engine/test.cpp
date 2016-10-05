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
		LFL_CallWin32F(LoadLibraryW, L"d3d12.dll");
		LFL_CallWin32F(LoadLibraryW, L"dxgi.dll");

		platform_ex::Windows::D3D12::Context::Instance();
	}
	CatchRet(platform_ex::Windows::Win32Exception&, TraceDe(Warning, "d3d12 win32 op failed."), nullptr)
	CatchRet(platform_ex::COMException&, TraceDe(Warning, "d3d12 com op failed."), nullptr)

	platform::Render::TextureType type;
	uint16 width, height,depth;
	uint8 num_mipmaps,array_size;
	platform::Render::EFormat format; 
	uint32 row_pitch, slice_pitch;

	platform::File blend_dds(L"blend.dds", platform::File::kToRead);
	platform::X::GetImageInfo(blend_dds, type, width, height, depth, num_mipmaps, array_size, format, row_pitch, slice_pitch);

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