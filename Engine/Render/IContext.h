/*! \file Engine\Render\IContext.h
\ingroup Engine
\brief 绘制创建接口类。
*/
#ifndef LE_RENDER_IContext_h
#define LE_RENDER_IContext_h 1

#include "IDevice.h"

namespace platform {
	namespace Render {

		class Context {
		public:
			virtual Device& GetDevice() = 0;

			virtual void Push(const PipleState&) = 0;
		private:
			virtual void CreateDeviceAndDisplay() = 0;
		public:
			static Context& Instance();
		};
	}
}

#endif