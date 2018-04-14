#include "ShellHelper.h"


namespace leo {
#ifndef NDEBUG
	DebugTimer::DebugTimer(platform::string_view str)
		: event_info((Nonnull(str.data()), str)), base_tick()
	{
		TraceDe(0xE0, "Start tick of [%s] :", event_info.c_str());
		base_tick = Timers::HighResolutionClock::now();
	}
	DebugTimer::~DebugTimer()
	{
		const double t((Timers::HighResolutionClock::now() - base_tick).count()
			/ 1e6);

		TraceDe(0xE0, "Performed [%s] in: %f milliseconds.", event_info.c_str(),
			t);
	}
#endif
}