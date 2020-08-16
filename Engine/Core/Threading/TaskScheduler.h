#pragma once

#include <LBase/ldef.h>
#include "../Coroutine/Task.h"
#include "../Coroutine/ThreadScheduler.h"
#include "../Coroutine/IOScheduler.h"

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

		leo::coroutine::IOScheduler& GetIOScheduler() noexcept;

		[[nodiscard]] leo::coroutine::IOScheduler::schedule_operation schedule_io() noexcept
		{
			return GetIOScheduler().schedule();
		}
	private:
		class Scheduler;

		Scheduler* scheduler_impl;
	};
}