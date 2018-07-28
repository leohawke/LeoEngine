#include "NinthTimer.h"

using  namespace platform::chrono;

NinthTimer::NinthTimer()
{
	Reset();
}

void NinthTimer::Reset()
{

}

void NinthTimer::UpdateOnFrameStart()
{
}

float NinthTimer::GetFrameTime(TimerType type)
{
	return 0.0f;
}

float NinthTimer::GetRealFrameTime() const
{
	return 0.0f;
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

	normal_pasued_duration =
		+offset_duration;

	return true;
}

bool NinthTimer::Continue()
{
	if (!paused_normal_timer)
		return false;

	paused_normal_timer = false;
	OffsetToGameTime(normal_pasued_duration);

	normal_pasued_duration = Duration::zero();
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
