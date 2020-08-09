#include "ManualResetEvent.h"

#include <LFramework/LCLib/NativeAPI.h>

namespace leo::threading {
	manual_reset_event::manual_reset_event(bool initiallySet)
		:value(initiallySet?1:0)
	{}

	manual_reset_event::~manual_reset_event() = default;

	void manual_reset_event::set() noexcept
	{
		value.store(1, std::memory_order_release);
		::WakeByAddressAll(&value);
	}

	void manual_reset_event::reset() noexcept
	{
		value.store(0, std::memory_order_relaxed);
	}

	void manual_reset_event::wait() noexcept
	{
		// Wait in a loop as WaitOnAddress() can have spurious wake-ups.
		int value = this->value.load(std::memory_order_acquire);
		BOOL ok = TRUE;
		while (value == 0)
		{
			if (!ok)
			{
				// Previous call to WaitOnAddress() failed for some reason.
				// Put thread to sleep to avoid sitting in a busy loop if it keeps failing.
				::Sleep(1);
			}

			ok = ::WaitOnAddress(&this->value, &value, sizeof(this->value), INFINITE);
			value = this->value.load(std::memory_order_acquire);
		}
	}
}
