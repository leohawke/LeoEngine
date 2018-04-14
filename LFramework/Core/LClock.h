/*!	\file LClock.h
\ingroup LFrameWork/Core
\brief 时钟接口
*/

#ifndef LFrameWork_Core_LClock_h
#define LFrameWork_Core_LClock_h 1

#include <LFramework/Core/LShellDefinition.h>
#include <LFramework/LCLib/Timer.h>
#include <chrono>

namespace leo {
	namespace Timers {
		/*!
		\brief 高精度时钟。
		*/
		class LF_API HighResolutionClock
		{
		public:
			using duration = std::chrono::duration<
				leo::make_signed_t<std::chrono::nanoseconds::rep>, std::nano>;
			using rep = duration::rep;
			using period = duration::period;
			using time_point = std::chrono::time_point<HighResolutionClock,
				std::chrono::nanoseconds>;

			static lconstexpr const bool is_steady = {};

			//! \warning 首次调用前非线程安全。
			static PDefH(time_point, now, ) lnothrow
				ImplRet(time_point(std::chrono::nanoseconds(platform::GetHighResolutionTicks())))
		};


		/*!
		\brief 高精度时间间隔。
		\note 单位为纳秒。
		*/
		using Duration = HighResolutionClock::duration;

		/*!
		\brief 时刻。
		*/
		using TimePoint = HighResolutionClock::time_point;

		/*!
		\brief 低精度时间间隔。
		\note 单位为毫秒。
		*/
		using TimeSpan = std::chrono::duration<Duration::rep, std::milli>;
	}
}


#endif