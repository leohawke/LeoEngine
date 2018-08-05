/*! \file System\TimeValue.h
\ingroup Engine
\brief ʱ�������λ������������API��
*/

#ifndef LE_System_TimeValue_H
#define LE_System_TimeValue_H 1

#include <LFramework/Core/LClock.h>

namespace platform::chrono {
	
	using ::leo::Timers::HighResolutionClock;

	//������ʹ�õ����ʱ�侫��Ϊ10΢��
	using EngineTick = std::ratio<1, 100000>;
	using EngineTickType = leo::make_signed_t<std::chrono::nanoseconds::rep>;

	using Duration = leo::Timers::Duration;
	using TimePoint = leo::Timers::TimePoint;
	using Tick = Duration::period;
	using TickType = Duration::rep;

	class TimeValue : public std::chrono::duration<
		EngineTickType, EngineTick>
	{
	};
}

#endif