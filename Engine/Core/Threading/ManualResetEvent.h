#pragma once

#include <atomic>
#include <cstdint>

namespace leo::threading {
	class manual_reset_event
	{
	public:
		manual_reset_event(bool initiallySet = false);

		~manual_reset_event();

		void set() noexcept;

		void reset() noexcept;

		void wait() noexcept;

	private:
		std::atomic<std::uint8_t> value;
	};
}