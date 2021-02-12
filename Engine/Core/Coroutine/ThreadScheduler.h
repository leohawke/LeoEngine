#pragma once

#include <LBase/ldef.h>
#include <coroutine>
#include <thread>

namespace leo::threading
{
	class TaskScheduler;
}

namespace leo::coroutine {
	class ThreadScheduler
	{
	public:
		class schedule_operation
		{
		public:
			schedule_operation(ThreadScheduler* ts) noexcept : scheduler(ts), any_scheduler(nullptr), next_oper(nullptr){}
			schedule_operation(leo::threading::TaskScheduler* ts) noexcept : scheduler(nullptr),any_scheduler(ts), next_oper(nullptr) {}

			~schedule_operation();

			bool await_ready() noexcept { return false; }
			void await_suspend(std::coroutine_handle<> continuation) noexcept;
			void await_resume() noexcept {}
		public:
			schedule_operation* next_oper;
		private:

			friend class ThreadScheduler;

			ThreadScheduler* scheduler;
			leo::threading::TaskScheduler* any_scheduler;
			std::coroutine_handle<> continuation_handle;
		};

		[[nodiscard]]
		schedule_operation schedule() noexcept { return schedule_operation{ this }; }

		ThreadScheduler(const std::wstring& name);
		ThreadScheduler();
	private:
		void schedule_impl(schedule_operation* operation) noexcept;

		void wake_up() noexcept;

		void run() noexcept;

		class thread_state;

		thread_state* current_state;

		static thread_local thread_state* thread_local_state;

		std::thread::native_handle_type native_handle;

		friend class leo::threading::TaskScheduler;
	};
}