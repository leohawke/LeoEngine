#include "Timer.h"
#include "NativeAPI.h"
#include <system_error>
#include <chrono>

namespace platform {

	std::uint32_t GetTicks() lnothrow
	{
		return std::uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count());
	}

	std::uint64_t GetHighResolutionTicks() lnothrow {
		return std::uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count());
	}
}