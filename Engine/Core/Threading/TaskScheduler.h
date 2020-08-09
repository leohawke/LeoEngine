#pragma once

#include "../Coroutine/Task.h"
#include "../Coroutine/ThreadScheduler.h"

namespace leo::threading {
	class TaskScheduler
	{
	public:
		[[nodiscard]]
		leo::coroutine::ThreadScheduler::schedule_operation schedule() noexcept;

		TaskScheduler();

		template<typename AWAITABLE>
		leo::coroutine::Task<void> Schedule(AWAITABLE awaitable)
		{
			co_await schedule();

			co_return co_await awaitable;
		}
	private:
		class Scheduler;

		Scheduler* scheduler_impl;
	};
}