/*! \file System\NinthTimer.h
\ingroup Engine
\brief 游戏引擎中所用的时钟。
*/

#ifndef LE_System_NinthTimer_H
#define LE_System_NinthTimer_H 1

#include "TimeValue.h"

namespace platform::chrono {
	class NinthTimer {
	public:
		enum class TimerType {
			//! \brief Pausable, serialized, frametime is smoothed/scaled/clamped.
			Normal =0, 
			//! \brief Non-pausable, non-serialized, frametime unprocessed.
			Monotonic ,
		};

		NinthTimer();
		~NinthTimer();

	public:
		void Reset();

	public:
		//! \brief Updates the timer every frame, needs to be called by the system.
		void UpdateOnFrameStart();
		float GetFrameTime(TimerType type = TimerType::Normal);
		float GetRealFrameTime() const;

		/*! 
		\brief try to pause/unpause the TimerType::Normal timer returns true if successfully paused/unpaused, false otherwise
		*/
		//@{
		bool Pasue();
		bool Continue();
		//@}

		//! \brief determine if the TimerType::Normal timer is paused returns true if paused, false otherwise
		bool IsPaused();
	public:
		const TimeValue& GetFrameStateTime(TimerType type = TimerType::Normal) const;

	private:
		const unsigned int TimerTypeCount = 2;
		using AdapterDuration = std::chrono::duration<double, std::chrono::seconds::period>;
	};

}

#endif