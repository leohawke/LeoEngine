#pragma once

#include "get_awaiter.h"

namespace leo::coroutine {

	template<typename T, typename = void>
	struct AwaitableTraits
	{};

	template<typename T>
	struct AwaitableTraits<T, std::void_t<decltype(leo::coroutine::details::get_awaiter(std::declval<T>()))>>
	{
		using awaiter_t = decltype(leo::coroutine::details::get_awaiter(std::declval<T>()));

		using await_result_t = decltype(std::declval<awaiter_t>().await_resume());
	};
}