/*! \file Core\ViewPort.h
\ingroup Engine
\brief 视口结构。
*/
#ifndef LE_Core_ViewPort_H
#define LE_Core_ViewPort_H 1

#include <LBase/sutility.h>

namespace platform {
	struct ViewPort : leo::noncopyable
	{
		ViewPort() = default;

		int left;
		int top;
		int width;
		int height;
	};
}

#endif