#include "AutoResetEvent.h"
#include <LFramework/LCLib/NativeAPI.h>


namespace leo::threading {
	auto_reset_event::auto_reset_event(bool initiallySet)
		: event(::CreateEventW(NULL, FALSE, initiallySet ? TRUE : FALSE, NULL))
	{
	}

	auto_reset_event::~auto_reset_event()
	{
	}

	void auto_reset_event::set()
	{
		BOOL ok = ::SetEvent(event);
	}

	void auto_reset_event::wait()
	{
		DWORD result = ::WaitForSingleObjectEx(event, INFINITE, FALSE);
	}
}