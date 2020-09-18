#pragma once

#include "when_all_counter.h"
#include <coroutine>
#include <tuple>

namespace leo::coroutine::details {
	template<typename TASK_CONTAINER>
	class when_all_ready_awaitable;

	template<>
	class when_all_ready_awaitable<std::tuple<>>
	{
	public:

		constexpr when_all_ready_awaitable() noexcept {}
		explicit constexpr when_all_ready_awaitable(std::tuple<>) noexcept {}

		constexpr bool await_ready() const noexcept { return true; }
		void await_suspend(std::coroutine_handle<>) noexcept {}
		std::tuple<> await_resume() const noexcept { return {}; }
	};

	template<typename... TASKS>
	class when_all_ready_awaitable<std::tuple<TASKS...>>
	{
	public:

		explicit when_all_ready_awaitable(TASKS&&... InTasks)
			noexcept(std::conjunction_v<std::is_nothrow_move_constructible<TASKS>...>)
			: counter(sizeof...(TASKS))
			, tasks(std::move(InTasks)...)
		{}

		explicit when_all_ready_awaitable(std::tuple<TASKS...>&& InTasks)
			noexcept(std::is_nothrow_move_constructible_v<std::tuple<TASKS...>>)
			: counter(sizeof...(TASKS))
			, tasks(std::move(InTasks))
		{}

		when_all_ready_awaitable(when_all_ready_awaitable&& other) noexcept
			: counter(sizeof...(TASKS))
			, tasks(std::move(other.tasks))
		{}

		auto operator co_await() & noexcept
		{
			struct awaiter
			{
				awaiter(when_all_ready_awaitable& awaitable) noexcept
					: awaitable(awaitable)
				{}

				bool await_ready() const noexcept
				{
					return awaitable.is_ready();
				}

				bool await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept
				{
					return awaitable.try_await(awaitingCoroutine);
				}

				std::tuple<TASKS...>& await_resume() noexcept
				{
					return awaitable.tasks;
				}

			private:

				when_all_ready_awaitable& awaitable;

			};

			return awaiter{ *this };
		}

		auto operator co_await() && noexcept
		{
			struct awaiter
			{
				awaiter(when_all_ready_awaitable& awaitable) noexcept
					: awaitable(awaitable)
				{}

				bool await_ready() const noexcept
				{
					return awaitable.is_ready();
				}

				bool await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept
				{
					return awaitable.try_await(awaitingCoroutine);
				}

				std::tuple<TASKS...>&& await_resume() noexcept
				{
					return std::move(awaitable.tasks);
				}

			private:

				when_all_ready_awaitable& awaitable;

			};

			return awaiter{ *this };
		}

	private:

		bool is_ready() const noexcept
		{
			return counter.is_ready();
		}

		bool try_await(std::coroutine_handle<> awaitingCoroutine) noexcept
		{
			start_tasks(std::make_integer_sequence<std::size_t, sizeof...(TASKS)>{});
			return counter.try_await(awaitingCoroutine);
		}

		template<std::size_t... INDICES>
		void start_tasks(std::integer_sequence<std::size_t, INDICES...>) noexcept
		{
			(void)std::initializer_list<int>{
				(std::get<INDICES>(tasks).start(counter), 0)...
			};
		}

		when_all_counter counter;
		std::tuple<TASKS...> tasks;

	};

	template<typename TASK_CONTAINER>
	class when_all_ready_awaitable
	{
	public:

		explicit when_all_ready_awaitable(TASK_CONTAINER&& InTasks) noexcept
			: counter(InTasks.size())
			, tasks(std::forward<TASK_CONTAINER>(InTasks))
		{}

		when_all_ready_awaitable(when_all_ready_awaitable&& other)
			noexcept(std::is_nothrow_move_constructible_v<TASK_CONTAINER>)
			: counter(other.tasks.size())
			, tasks(std::move(other.tasks))
		{}

		when_all_ready_awaitable(const when_all_ready_awaitable&) = delete;
		when_all_ready_awaitable& operator=(const when_all_ready_awaitable&) = delete;

		auto operator co_await() & noexcept
		{
			class awaiter
			{
			public:

				awaiter(when_all_ready_awaitable& awaitable)
					: awaitable(awaitable)
				{}

				bool await_ready() const noexcept
				{
					return awaitable.is_ready();
				}

				bool await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept
				{
					return awaitable.try_await(awaitingCoroutine);
				}

				TASK_CONTAINER& await_resume() noexcept
				{
					return awaitable.tasks;
				}

			private:

				when_all_ready_awaitable& awaitable;

			};

			return awaiter{ *this };
		}


		auto operator co_await() && noexcept
		{
			class awaiter
			{
			public:

				awaiter(when_all_ready_awaitable& awaitable)
					: awaitable(awaitable)
				{}

				bool await_ready() const noexcept
				{
					return awaitable.is_ready();
				}

				bool await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept
				{
					return awaitable.try_await(awaitingCoroutine);
				}

				TASK_CONTAINER&& await_resume() noexcept
				{
					return std::move(awaitable.tasks);
				}

			private:
				when_all_ready_awaitable& awaitable;
			};

			return awaiter{ *this };
		}

	private:

		bool is_ready() const noexcept
		{
			return counter.is_ready();
		}

		bool try_await(std::coroutine_handle<> awaitingCoroutine) noexcept
		{
			for (auto&& task : tasks)
			{
				task.start(counter);
			}

			return counter.try_await(awaitingCoroutine);
		}

		when_all_counter counter;
		TASK_CONTAINER tasks;

	};
}