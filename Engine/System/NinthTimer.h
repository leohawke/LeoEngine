/*! \file System\NinthTimer.h
\ingroup Engine
\brief ��Ϸ���������õ�ʱ�ӡ�
*/

#ifndef LE_System_TimeValue_H
#define LE_System_TimeValue_H 1

#include "TimeValue.h"

namespace platform::chrono {
	class NinthTimer {
	public:
		enum class TimerType {
			Normal =0, //!< Pausable, serialized, frametime is smoothed/scaled/clamped.
			Monotonic ,//!< Non-pausable, non-serialized, frametime unprocessed.
		};
	};

}

#endif