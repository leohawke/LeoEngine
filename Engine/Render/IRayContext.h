/*! \file Engine\Render\IRayContext.h
\ingroup Engine
\brief 射线执行接口类。
*/
#ifndef LE_RENDER_IRayContext_h
#define LE_RENDER_IRayContext_h 1

#include "IRayDevice.h"

namespace platform::Render {
	class RayContext
	{
	public:
		virtual RayDevice& GetDevice() = 0;

	};
}

#endif
