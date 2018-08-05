#include "NinthTimer.h"

using  namespace platform::chrono;
using namespace std::chrono;

NinthTimer::NinthTimer()
{
	Reset();
}

void NinthTimer::Reset()
{
	start_point = HighResolutionClock::now();

	last_duration = 0ms;
	offset_duration = 0ms;

	frame_time = 0;
	real_frametime = 0;

	paused_normal_timer = false;
	normal_pasued_duration = 0ms;
}

void NinthTimer::UpdateOnFrameStart()
{
	if (!enable)
		return;

	++frame_counter;

	const auto now = HighResolutionClock::now();

	real_frametime = GetAdapterDurationCount<float>(now - start_point - last_duration);

	frame_time = std::min(real_frametime,0.25f);

	LAssert(frame_time >= 0, "Time can only go forward.");


	auto current_duration = now - start_point;

	last_duration = current_duration;
	
}

float NinthTimer::GetFrameTime(TimerType type)
{
	if (!enable)
		return 0;
	if (type == TimerType::Normal)
		return !paused_normal_timer ? frame_time : 0;
	return frame_time;
}

float NinthTimer::GetRealFrameTime() const
{
	return !paused_normal_timer ? real_frametime : 0;
}



bool NinthTimer::IsPaused()
{
	return paused_normal_timer;
}

const TimeValue & NinthTimer::GetFrameStartTime(TimerType type) const
{
	return timespans[(unsigned int)type];
}


//when pause last_duration + offset_duration
//when continue last_duration' = last_duration + dt
//last_duration' + offset_duration' = last_duration + offset_duration
//=>offset_duration' = (last_duration + offset_duration) - last_duration'

bool NinthTimer::Pasue()
{
	if (paused_normal_timer)
		return false;

	paused_normal_timer = true;

	normal_pasued_duration = last_duration +offset_duration;

	return true;
}

bool NinthTimer::Continue()
{
	if (!paused_normal_timer)
		return false;

	paused_normal_timer = false;
	OffsetToGameTime(normal_pasued_duration);

	normal_pasued_duration = 0ms;
	return true;
}

void NinthTimer::OffsetToGameTime(Duration pasue_duration)
{

	offset_duration = pasue_duration - last_duration;

	//RefreshNormalTime

	if (paused_normal_timer) {
		normal_pasued_duration = pasue_duration;
	}
}
