/*!	\file LClock.h
\ingroup LFrameWork/Core
\brief ʱ�ӽӿ�
*/

#ifndef LFrameWork_Core_LClock_h
#define LFrameWork_Core_LClock_h 1

#include <LFramework/Core/LShellDefinition.h>
#include <LFramework/LCLib/Timer.h>
#include <chrono>

namespace leo {
	namespace Timers {
		/*!
		\brief �߾���ʱ�ӡ�
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

			//! \warning �״ε���ǰ���̰߳�ȫ��
			static PDefH(time_point, now, ) lnothrow
				ImplRet(time_point(std::chrono::nanoseconds(platform::GetHighResolutionTicks())))
		};


		/*!
		\brief �߾���ʱ������
		\note ��λΪ���롣
		*/
		using Duration = HighResolutionClock::duration;

		/*!
		\brief ʱ�̡�
		*/
		using TimePoint = HighResolutionClock::time_point;

		/*!
		\brief �;���ʱ������
		\note ��λΪ���롣
		*/
		using TimeSpan = std::chrono::duration<Duration::rep, std::milli>;
	}
}


#endif