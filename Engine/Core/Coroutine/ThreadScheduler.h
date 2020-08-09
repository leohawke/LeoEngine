#pragma once

#include <experimental/coroutine>


namespace leo::coroutine {
	class ThreadScheduler
	{
	public:
		class schedule_operation
		{
		public:
			schedule_operation(ThreadScheduler* ts) noexcept : scheduler(ts) {}

			bool await_ready() noexcept { return false; }
			void await_suspend(std::experimental::coroutine_handle<> continuation) noexcept;
			void await_resume() noexcept {}

		private:

			friend class ThreadScheduler;

			ThreadScheduler* scheduler;
			std::experimental::coroutine_handle<> continuation_handle;
		};

		[[nodiscard]]
		schedule_operation schedule() noexcept { return schedule_operation{ this }; }

		ThreadScheduler();
	private:
		void schedule_impl(schedule_operation* operation) noexcept;

		void run() noexcept;

		class thread_state;

		thread_state* current_state;
	};
}