/*! \file Engine\Render\IContext.h
\ingroup Engine
\brief 绘制创建接口类。
*/
#ifndef LE_RENDER_IContext_h
#define LE_RENDER_IContext_h 1

#include "IDevice.h"

namespace platform::Render {

	class FrameBuffer;

	class Context {
	public:
		virtual Device& GetDevice() = 0;

		virtual void Push(const PipleState&) = 0;
		virtual void Render(const Effect::Effect&, const Effect::Technique&, const InputLayout&) = 0;

	public:
		virtual void CreateDeviceAndDisplay() = 0;
	private:
		virtual void DoBindFrameBuffer(const std::shared_ptr<FrameBuffer>&) = 0;
	public:
		void SetFrame(const std::shared_ptr<FrameBuffer>&);
		const std::shared_ptr<FrameBuffer>& GetCurrFrame() const;
		const std::shared_ptr<FrameBuffer>& GetScreenFrame() const;

	public:
		static Context& Instance();

	private:
		std::shared_ptr<FrameBuffer> curr_frame_buffer;
		std::shared_ptr<FrameBuffer> screen_frame_buffer;
	};
}

#endif