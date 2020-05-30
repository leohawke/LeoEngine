#include "IFrameBuffer.h"
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
		HardwareShader::~HardwareShader() = default;

		Context& Context::Instance() {
			return platform_ex::Windows::D3D12::GetContext();
		}

		void Context::SetFrame(const std::shared_ptr<FrameBuffer>& framebuffer)
		{
			if(!framebuffer && curr_frame_buffer)
				curr_frame_buffer->OnUnBind();

			if (framebuffer) {
				curr_frame_buffer = framebuffer;
				curr_frame_buffer->OnBind();

				DoBindFrameBuffer(curr_frame_buffer);
			}
		}
		const std::shared_ptr<FrameBuffer>& Context::GetCurrFrame() const
		{
			return curr_frame_buffer;
		}
		const std::shared_ptr<FrameBuffer>& Context::GetScreenFrame() const
		{
			return screen_frame_buffer;
		}
	}
}
