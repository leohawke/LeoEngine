#pragma once

#include "is_awaiter.h"

namespace leo::coroutine::details {

	struct any
	{
		template<typename T>
		any(T&&) noexcept
		{}
	};

	template<typename T>
	auto get_awaiter_impl(T&& value, int)
		noexcept(noexcept(static_cast<T&&>(value).operator co_await()))
		-> decltype(static_cast<T&&>(value).operator co_await())
	{
		return static_cast<T&&>(value).operator co_await();
	}

	template<typename T>
	auto get_awaiter_impl(T&& value, long)
		noexcept(noexcept(operator co_await(static_cast<T&&>(value))))
		-> decltype(operator co_await(static_cast<T&&>(value)))
	{
		return operator co_await(static_cast<T&&>(value));
	}

	template<
		typename T,
		std::enable_if_t<is_awaiter<T&&>::value, int> = 0>
		T&& get_awaiter_impl(T&& value, leo::coroutine::details::any) noexcept
	{
		return static_cast<T&&>(value);
	}

	template<typename T>
	auto get_awaiter(T&& value)
		noexcept(noexcept(details::get_awaiter_impl(static_cast<T&&>(value), 123)))
		-> decltype(details::get_awaiter_impl(static_cast<T&&>(value), 123))
	{
		return details::get_awaiter_impl(static_cast<T&&>(value), 123);
	}
}