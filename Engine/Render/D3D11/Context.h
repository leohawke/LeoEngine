#include "../IContext.h"
#include <UniversalDXSDK/d3d11.h>


namespace platform_ex {
	namespace Windows {
		namespace D3D11 {

			class Device final : platform::Render::Device {
			};

			class Context : public platform::Render::Context {
			public:
				static Context& Instance();
			};
		}
	}
}
