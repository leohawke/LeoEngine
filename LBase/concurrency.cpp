#include "LBase/any.h"
#include <sstream>

#if __STDCPP_THREADS__ || (defined(__GLIBCXX__) \
	&& defined(_GLIBCXX_HAS_GTHREADS) && defined(_GLIBCXX_USE_C99_STDINT_TR1)) \
	|| (defined(_LIBCPP_VERSION) && !defined(_LIBCPP_HAS_NO_THREADS))
#include "LBase/concurrency.h"

namespace leo {

	std::string
		to_string(const std::thread::id& id)
	{
		std::ostringstream oss;

		oss << id;
		return oss.str();
	}

#	if !__GLIBCXX__ || (defined(_GLIBCXX_HAS_GTHREADS) \
	&& defined(_GLIBCXX_USE_C99_STDINT_TR1) && (ATOMIC_INT_LOCK_FREE > 1))
	thread_pool::thread_pool(size_t n, std::function<void()> on_enter,
		std::function<void()> on_exit)
	{
		workers.reserve(n);
		for (size_t i = 0; i < n; ++i)
			workers.emplace_back([=] {
			if (on_enter)
				on_enter();
			while (true)
			{
				std::unique_lock<std::mutex> lck(queue_mutex);

				condition.wait(lck, [this] {
					return stopped || !tasks.empty();
				});
				if (tasks.empty())
				{
					// NOTE: Do nothing for spurious wakeup.
					if (stopped)
						break;
				}
				else
				{
					auto task(std::move(tasks.front()));

					tasks.pop();
					lck.unlock();
					try
					{
						task();
					}
					catch (std::future_error&)
					{
						lassume(false);
					}
				}
			}
			if (on_exit)
				on_exit();
		});
	}
	thread_pool::~thread_pool() lnothrow
	{
		try
		{
			try
			{
				std::lock_guard<std::mutex> lck(queue_mutex);

				stopped = true;
			}
			catch (std::system_error&)
			{
				lassume(false);
			}
			condition.notify_all();
			for (auto& worker : workers) {
				try
				{
					worker.join();
				}
				catch (std::system_error&)
				{
				}
			}
		}
		catch (...)
		{
			lassume(false);
		}
	}

	size_t
		thread_pool::size() const
	{
		std::unique_lock<std::mutex> lck(queue_mutex);

		return size_unlocked();
	}

	void
		task_pool::reset(size_t tasks_num)
	{
		auto& threads(static_cast<thread_pool&>(*this));

		threads.~thread_pool();
		::new(&threads) thread_pool(tasks_num);
	}
#	endif
}

#endif