/*! \file Engine\Render\IDisplay.h
\ingroup Engine
\brief 显示逻辑相关,平台UI层的桥接。
*/

#ifndef LE_RENDER_IDisplay_h
#define LE_RENDER_IDisplay_h 1

#include <LBase/sutility.h>

namespace platform::Render {

	/* \warning 非虚析构
	*/
	class Display :private limpl(leo::noncopyable), private limpl(leo::nonmovable) {
	public:
		virtual void SwapBuffers() = 0;
		virtual void WaitOnSwapBuffers() = 0;
	};
}

#endif