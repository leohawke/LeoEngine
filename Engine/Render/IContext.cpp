#include "IContext.h"


enum ContextType {
	Context_D3D12
};

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {
			bool Support();
			platform::Render::Context& GetContext();
		}
	}
}

namespace platform {
	namespace Render {
		Context& Context::Instance() {
			return platform_ex::Windows::D3D12::GetContext();
		}
	}
}
