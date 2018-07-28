#include "Context.h"

#define TEST_CODE 1
#if TEST_CODE
extern HWND g_hwnd;
#endif

namespace platform_ex::Windows::D3D11 {
	Context & Context::Instance()
	{
		static Context context;
		return context;
	}
}

namespace platform_ex {
	namespace Windows {
		namespace D3D11 {
			platform::Render::Context& GetContext() {
				return Context::Instance();
			}
		}
	}
}