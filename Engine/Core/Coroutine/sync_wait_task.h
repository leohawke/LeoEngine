#pragma once

#include "../Threading/ManualResetEvent.h"
#include <experimental/coroutine>

namespace leo::coroutine::details {
	template<typename RESULT>
	class sync_wait_task_promise;

	template<>
	class sync_wait_task_promise<void>
	{
		using coroutine_handle_t = std::experimental::coroutine_handle<sync_wait_task_promise<void>>;

	public:

		sync_wait_task_promise() noexcept
		{}

		void start(leo::threading::manual_reset_event& InEvent)
		{
			event = &InEvent;
			coroutine_handle_t::from_promise(*this).resume();
		}

		auto get_return_object() noexcept
		{
			return coroutine_handle_t::from_promise(*this);
		}

		std::experimental::suspend_always initial_suspend() noexcept
		{
			return{};
		}

		auto final_suspend() noexcept
		{
			class completion_notifier
			{
			public:

				bool await_ready() const noexcept { return false; }

				void await_suspend(coroutine_handle_t coroutine) const noexcept
				{
					coroutine.promise().event->set();
				}

				void await_resume() noexcept {}
			};

			return completion_notifier{};
		}

		void return_void() {}

		void unhandled_exception()
		{
			exception = std::current_exception();
		}

		void result()
		{
			if (exception)
			{
				std::rethrow_exception(exception);
			}
		}

	private:

		leo::threading::manual_reset_event* event;
		std::exception_ptr exception;
	};

	template<typename RESULT>
	class sync_wait_task final
	{
	public:
		using promise_type = sync_wait_task_promise<RESULT>;

		using coroutine_handle_t = std::experimental::coroutine_handle<promise_type>;

		sync_wait_task(coroutine_handle_t coroutine) noexcept
			: m_coroutine(coroutine)
		{}

		sync_wait_task(sync_wait_task&& other) noexcept
			: m_coroutine(std::exchange(other.m_coroutine, coroutine_handle_t{}))
		{}

		~sync_wait_task()
		{
			if (m_coroutine) m_coroutine.destroy();
		}

		sync_wait_task(const sync_wait_task&) = delete;
		sync_wait_task& operator=(const sync_wait_task&) = delete;

		void start(leo::threading::manual_reset_event& event) noexcept
		{
			m_coroutine.promise().start(event);
		}

		decltype(auto) result()
		{
			return m_coroutine.promise().result();
		}

	private:

		coroutine_handle_t m_coroutine;

	};

	template<
		typename AWAITABLE,
		std::enable_if_t<std::is_void_v<void>, int> = 0>
		sync_wait_task<void> make_sync_wait_task(AWAITABLE& awaitable)
	{
		co_await static_cast<AWAITABLE&&>(awaitable);
	}
}