/*!	\file Input.h
\ingroup LFrameWork/LCLib
\brief 平台相关的扩展输入接口。
*/

#ifndef LFrameWork_LCLib_Input_h
#define LFrameWork_LCLib_Input_h 1


#include <LFramework/LCLib/FCommon.h>

namespace platform {

}

namespace platform_ex {
	/*!
	\brief 清除按键缓冲。
	*/
	LF_API void
		ClearKeyStates();
}

#endif