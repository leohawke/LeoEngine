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

namespace platform::Render {
	HardwareShader::~HardwareShader() = default;

	Context& Context::Instance() {
		return platform_ex::Windows::D3D12::GetContext();
	}

	void Context::SetFrame(const std::shared_ptr<FrameBuffer>& framebuffer)
	{
		if (!framebuffer && curr_frame_buffer)
			curr_frame_buffer->OnUnBind();

		if (framebuffer) {
			curr_frame_buffer = framebuffer;

			DoBindFrameBuffer(curr_frame_buffer);

			curr_frame_buffer->OnBind();
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

	GraphicsBuffer* CreateConstantBuffer(const void* Contents, Buffer::Usage Usage, const ShaderParametersMetadata& Layout)
	{
		auto& Device = Context::Instance().GetDevice();

		Usage = Buffer::Usage::Static;

		return Device.CreateConstanBuffer(Usage, 0,Layout.GetSize(), EFormat::EF_Unknown,Contents);
	}
}
