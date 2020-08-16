#pragma once

#include <LBase/ldef.h>
#include <experimental/coroutine>
#include <type_traits>
#include <optional>
#include <atomic>

namespace leo::coroutine {

	template<typename T> class Task;

	namespace details
	{

		class task_promise_base
		{
			friend struct final_awaitable;

			struct final_awaitable
			{
				bool await_ready() const noexcept { return false; }

				template<typename PROMISE>
				void await_suspend(std::experimental::coroutine_handle<PROMISE> coroutine)
				{
					task_promise_base& promise = coroutine.promise();

					// Use 'release' memory semantics in case we finish before the
					// awaiter can suspend so that the awaiting thread sees our
					// writes to the resulting value.
					// Use 'acquire' memory semantics in case the caller registered
					// the continuation before we finished. Ensure we see their write
					// to m_continuation.
					if (promise.state.exchange(true, std::memory_order_acq_rel))
					{
						promise.continuation_handle.resume();
					}
				}

				void await_resume() noexcept {}
			};

		public:
			task_promise_base() noexcept
				:state(false)
			{}

			auto initial_suspend() noexcept
			{
				return std::experimental::suspend_always{};
			}

			auto final_suspend() noexcept
			{
				return final_awaitable{};
			}

			bool set_continuation(std::experimental::coroutine_handle<> continuation) noexcept
			{
				continuation_handle = continuation;
				return !state.exchange(true, std::memory_order_acq_rel);
			}

		private:
			std::experimental::coroutine_handle<> continuation_handle;

			std::atomic<bool> state;
		};

		template<typename T>
		class task_promise final :public task_promise_base
		{
		public:
			task_promise() noexcept {}

			~task_promise()
			{

			}

			Task<T> get_return_object() noexcept;

			void unhandled_exception() { std::terminate(); }

			template<
				typename VALUE,
				typename = std::enable_if_t<std::is_convertible_v<VALUE&&, T>>>
				void return_value(VALUE&& value) noexcept(std::is_nothrow_constructible_v<T, VALUE&&>)
			{
				promise_value.emplace(std::forward<VALUE>(value));
			}

			const T& value()
			{
				return promise_value.value();
			}
		private:
			std::optional<T> promise_value;
		};

		template<>
		class task_promise<void> :public task_promise_base
		{
		public:
			task_promise() noexcept {}

			Task<void> get_return_object() noexcept;

			void unhandled_exception() { std::terminate(); }

			void return_void() noexcept
			{}

			void value() noexcept
			{}
		};
	}

	template<typename T = void>
	class [[nodiscard]] Task
	{
	public:
		using promise_type = details::task_promise<T>;

		using value_type = T;
	private:
		struct awaitable_base
		{
			std::experimental::coroutine_handle<promise_type> coroutine_handle;

			explicit awaitable_base(std::experimental::coroutine_handle<promise_type> coroutine)
				: coroutine_handle(coroutine)
			{}

			bool await_ready() const noexcept
			{
				return !coroutine_handle || coroutine_handle.done();
			}

			bool await_suspend(
				std::experimental::coroutine_handle<> awaitingCoroutine) noexcept
			{
				coroutine_handle.resume();
				return coroutine_handle.promise().set_continuation(awaitingCoroutine);
			}
		};
	public:

		explicit Task(std::experimental::coroutine_handle<promise_type> coroutine)
			: coroutine_handle(coroutine)
		{}

		Task(Task&& t) noexcept
			: coroutine_handle(t.coroutine_handle)
		{
			t.coroutine_handle = nullptr;
		}

		~Task()
		{
			if (coroutine_handle)
			{
				coroutine_handle.destroy();
			}
		}

		/// Disable copy construction/assignment.
		Task(const Task&) = delete;
		Task& operator=(const Task&) = delete;

		Task& operator=(Task&& other) noexcept
		{
			if (std::addressof(other) != this)
			{
				if (coroutine_handle)
				{
					coroutine_handle.destroy();
				}

				coroutine_handle = other.coroutine_handle;
				other.coroutine_handle = nullptr;
			}

			return *this;
		}

		bool is_ready() const noexcept
		{
			return !coroutine_handle || coroutine_handle.done();
		}

		auto operator co_await() const& noexcept
		{
			struct awaitable : awaitable_base
			{
				using awaitable_base::awaitable_base;

				decltype(auto) await_resume()
				{
					return this->coroutine_handle.promise().value();
				}
			};

			return awaitable{ coroutine_handle };
		}

		auto operator co_await() const&& noexcept
		{
			struct awaitable : awaitable_base
			{
				using awaitable_base::awaitable_base;

				decltype(auto) await_resume()
				{
					return std::move(this->coroutine_handle.promise()).value();
				}
			};

			return awaitable{ coroutine_handle };
		}

	private:
		std::experimental::coroutine_handle<promise_type> coroutine_handle;
	};

	namespace details
	{
		template<typename T>
		Task<T> task_promise<T>::get_return_object() noexcept
		{
			return Task<T>{ std::experimental::coroutine_handle<task_promise>::from_promise(*this) };
		}

		inline Task<void> task_promise<void>::get_return_object() noexcept
		{
			return Task<void>{ std::experimental::coroutine_handle<task_promise>::from_promise(*this) };
		}
	}

}