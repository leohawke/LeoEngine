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

			co_return co_await std::move(awaitable);
		}

		leo::coroutine::IOScheduler& GetIOScheduler() noexcept;

		[[nodiscard]] leo::coroutine::ThreadScheduler::schedule_operation schedule_render() noexcept;

		bool is_render_schedule() const noexcept;
	private:
		friend class leo::coroutine::ThreadScheduler;

		void schedule_impl(leo::coroutine::ThreadScheduler::schedule_operation* operation) noexcept;

		void remote_enqueue(leo::coroutine::ThreadScheduler::schedule_operation* operation) noexcept;

		void wake_one_thread() noexcept;

		leo::coroutine::ThreadScheduler::schedule_operation* get_remote() noexcept;

		class Scheduler;

		Scheduler* scheduler_impl;
	};
}