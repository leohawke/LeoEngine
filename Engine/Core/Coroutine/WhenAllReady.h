#pragma once

#include "AwaitableTraits.h"
#include "is_awaiter.h"
#include "when_all_ready_awaitable.h"
#include "when_all_task.h"
#include <tuple>
#include <utility>
#include <vector>
#include <type_traits>

namespace leo::coroutine {
	namespace details
	{
		template<typename T>
		struct unwrap_reference
		{
			using type = T;
		};

		template<typename T>
		struct unwrap_reference<std::reference_wrapper<T>>
		{
			using type = T;
		};

		template<typename T>
		using unwrap_reference_t = typename unwrap_reference<T>::type;
	}

	template<
		typename AWAITABLE,
		typename RESULT = typename AwaitableTraits<details::unwrap_reference_t<AWAITABLE>>::await_result_t>
		[[nodiscard]] auto WhenAllReady(std::vector<AWAITABLE> awaitables)
	{
		std::vector<details::when_all_task<RESULT>> tasks;

		tasks.reserve(awaitables.size());

		for (auto& awaitable : awaitables)
		{
			tasks.emplace_back(details::make_when_all_task(std::move(awaitable)));
		}

		return details::when_all_ready_awaitable<std::vector<details::when_all_task<RESULT>>>(
			std::move(tasks));
	}
}
