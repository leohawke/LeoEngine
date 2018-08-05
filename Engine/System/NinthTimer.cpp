#include "NinthTimer.h"
#include <LFramework/Core/LException.h>

using  namespace platform::chrono;
using namespace std::chrono;


namespace {
	NinthTimer* TimerPtr;
}

NinthTimer::NinthTimer()
{
	LAssert(!TimerPtr, "Duplicate instance found.");
	TimerPtr = this;

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

	RefreshMonotonicTime(current_duration);
	if (!paused_normal_timer)
		RefreshNormalTime(current_duration);

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

	RefreshNormalTime(last_duration);

	if (paused_normal_timer) {
		normal_pasued_duration = pasue_duration;
	}
}

//这里发生了数值类型的转变和精度误差
void NinthTimer::RefreshNormalTime(Duration durration) {
	timespans[(unsigned int)TimerType::Normal].SetSecondDuration(duration_cast<AdapterDuration>(durration + offset_duration));
}

void NinthTimer::RefreshMonotonicTime(Duration durration) {
	timespans[(unsigned int)TimerType::Monotonic].SetSecondDuration(duration_cast<AdapterDuration>(durration));
}

NinthTimer& FetchGlobalTimer() {
	if (TimerPtr)
		return *TimerPtr;
	throw leo::GeneralEvent("Timer instance is not ready.");
}

