/*!	\file Timer.h
\ingroup LFrameWork/LCLib
\brief 平台相关的计时器接口。
*/

#ifndef LFrameWork_LCLib_Timer_h
#define LFrameWork_LCLib_Timer_h 1

#include <LFramework/LCLib/FCommon.h>

namespace platform {
	/*!
	\brief 取 tick 数。
	\note 单位为毫秒。
	\warning 首次调用 StartTicks 前非线程安全。
	*/
	LF_API std::uint32_t
		GetTicks() lnothrow;

	/*!
	\brief 取高精度 tick 数。
	\note 单位为纳秒。
	\warning 首次调用 StartTicks 前非线程安全。
	*/
	LF_API std::uint64_t
		GetHighResolutionTicks() lnothrow;
}

#endif