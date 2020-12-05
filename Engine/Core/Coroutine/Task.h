#pragma once

#include <LBase/ldef.h>
#include <coroutine>
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
				std::coroutine_handle<> await_suspend(std::coroutine_handle<PROMISE> coroutine)
				{
					return coroutine.promise().continuation_handle;
				}

				void await_resume() noexcept {}
			};

		public:
			task_promise_base() noexcept
			{}

			auto initial_suspend() noexcept
			{
				return std::suspend_always{};
			}

			auto final_suspend() noexcept
			{
				return final_awaitable{};
			}

			bool set_continuation(std::coroutine_handle<> continuation) noexcept
			{
				continuation_handle = continuation;
			}

		private:
			std::coroutine_handle<> continuation_handle;
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

		template<typename T>
		class task_promise<T&> : public task_promise_base
		{
		public:
			task_promise() noexcept = default;

			Task<T&> get_return_object() noexcept;

			void unhandled_exception() noexcept
			{
				m_exception = std::current_exception();
			}

			void return_value(T & value) noexcept
			{
				m_value = std::addressof(value);
			}

			T& value()
			{
				if (m_exception)
				{
					std::rethrow_exception(m_exception);
				}

				return *m_value;
			}
		private:
			T* m_value = nullptr;
			std::exception_ptr m_exception;
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
			std::coroutine_handle<promise_type> coroutine_handle;

			explicit awaitable_base(std::coroutine_handle<promise_type> coroutine)
				: coroutine_handle(coroutine)
			{}

			bool await_ready() const noexcept
			{
				return !coroutine_handle || coroutine_handle.done();
			}

			std::coroutine_handle<> await_suspend(
				std::coroutine_handle<> awaitingCoroutine) noexcept
			{
				coroutine_handle.promise().set_continuation(awaitingCoroutine);
				return coroutine_handle;
			}
		};
	public:

		explicit Task(std::coroutine_handle<promise_type> coroutine)
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
		std::coroutine_handle<promise_type> coroutine_handle;
	};

	namespace details
	{
		template<typename T>
		Task<T> task_promise<T>::get_return_object() noexcept
		{
			return Task<T>{ std::coroutine_handle<task_promise>::from_promise(*this) };
		}

		inline Task<void> task_promise<void>::get_return_object() noexcept
		{
			return Task<void>{ std::coroutine_handle<task_promise>::from_promise(*this) };
		}

		template<typename T>
		Task<T&> task_promise<T&>::get_return_object() noexcept
		{
			return Task<T&>{ std::coroutine_handle<task_promise>::from_promise(*this) };
		}
	}

}