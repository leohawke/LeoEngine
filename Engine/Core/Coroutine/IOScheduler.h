#pragma once

#include <experimental/coroutine>

namespace leo::coroutine {
	class IOScheduler
	{
	public:
		class schedule_operation;

		IOScheduler();

		IOScheduler(std::uint32_t concurrencyHint);


		/// Returns an operation that when awaited suspends the awaiting
		/// coroutine and reschedules it for resumption on an I/O thread
		/// associated with this io_service.
		[[nodiscard]]
		schedule_operation schedule() noexcept;

		std::uint64_t process_one_pending_event();

		void* native_iocp_handle() noexcept;
	private:
		friend class schedule_operation;
		void schedule_impl(schedule_operation* operation) noexcept;

		bool try_enter_event_loop() noexcept;

		void exit_event_loop() noexcept;

		bool try_process_one_event(bool waitForEvent);

		void try_reschedule_overflow_operations() noexcept;

		static constexpr std::uint32_t stop_requested_flag = 1;
		static constexpr std::uint32_t active_thread_count_increment = 2;

		// Bit 0: stop_requested_flag
		// Bit 1-31: count of active threads currently running the event loop
		std::atomic<std::uint32_t> thread_state;
	private:

		void* iocp_handle;

		// Head of a linked-list of schedule operations that are
		// ready to run but that failed to be queued to the I/O
		// completion port (eg. due to low memory).
		std::atomic<schedule_operation*> operations_head;
	};

	class IOScheduler::schedule_operation
	{
	public:
		schedule_operation(IOScheduler& InService) noexcept
			: service(InService),pNext(nullptr)
		{}

		bool await_ready() const noexcept { return false; }
		void await_suspend(std::experimental::coroutine_handle<> awaiter) noexcept;
		void await_resume() const noexcept {}

	private:
		friend class IOScheduler;

		IOScheduler& service;
		std::experimental::coroutine_handle<> continuation;
		schedule_operation* pNext;
	};
}