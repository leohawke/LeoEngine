/*!	\file ShellHelper.h
\ingroup LFrameWorkHelper
\brief Shell °ïÖúÄ£¿é¡£
*/

#ifndef LFrameWork_Helper_ShellHelper_h
#define LFrameWork_Helper_ShellHelper_h 1

#include <LFramework/Core/LShell.h>
#include <LFramework/LCLib/Debug.h>
#include <LFramework/Core/LClock.h>
#include <LFramework/Adaptor/LAdaptor.h>

namespace leo {
#ifndef NDEBUG
	class LF_API DebugTimer {
	protected:
		string event_info;
		Timers::HighResolutionClock::time_point base_tick;
	public:
		DebugTimer(string_view = "");
		~DebugTimer();
	};
#	define LFL_DEBUG_DECL_TIMER(_name, ...) leo::DebugTimer _name(__VA_ARGS__);
#else
#	define LFL_DEBUG_DECL_TIMER(...)
#endif
}

#endif
