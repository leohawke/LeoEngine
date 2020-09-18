#pragma once

#include <coroutine>
#include <atomic>
#include <cstdint>

namespace leo::coroutine::details {
	class when_all_counter
	{
	public:

		when_all_counter(std::size_t InCount) noexcept
			: count(InCount + 1)
			, coroutine(nullptr)
		{}

		bool is_ready() const noexcept
		{
			// We consider this complete if we're asking whether it's ready
			// after a coroutine has already been registered.
			return static_cast<bool>(coroutine);
		}

		bool try_await(std::coroutine_handle<> awaitingCoroutine) noexcept
		{
			coroutine = awaitingCoroutine;
			return count.fetch_sub(1, std::memory_order_acq_rel) > 1;
		}

		void notify_awaitable_completed() noexcept
		{
			if (count.fetch_sub(1, std::memory_order_acq_rel) == 1)
			{
				coroutine.resume();
			}
		}

	protected:

		std::atomic<std::size_t> count;
		std::coroutine_handle<> coroutine;

	};
}