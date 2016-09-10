/*! \file Engine\Render\IContext.h
\ingroup Engine
\brief 绘制创建接口类。
*/
#ifndef LE_RENDER_IContext_h
#define LE_RENDER_IContext_h 1

namespace platform {
	namespace Render {
		class Device {};

		class Context {
		private:
			virtual void CreateDeviceAndDisplay() = 0;
		};
	}
}

#endif