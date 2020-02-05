/*! \file Engine\Render\IRayContext.h
\ingroup Engine
\brief ����ִ�нӿ��ࡣ
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
