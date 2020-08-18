#include "ThreadScheduler.h"
#include "../Threading/AutoResetEvent.h"
#include "../../System/SystemEnvironment.h"
#include <atomic>
#include <memory>
#include <thread>

namespace
{
	namespace local
	{
		// Keep each thread's local queue under 1MB
		constexpr std::size_t max_local_queue_size = 1024 * 1024 / sizeof(void*);
		constexpr std::size_t initial_local_queue_size = 256;
	}
}

namespace leo::coroutine {
	class ThreadScheduler::thread_state
	{
	public:
		constexpr static std::size_t mask = local::initial_local_queue_size - 1;

		thread_state()
			:queue(std::make_unique<std::atomic<schedule_operation*>[]>(
				local::initial_local_queue_size)),
			queue_head(0),
			queue_tail(0),
			sleeping(false)
		{}

		bool wake_up()
		{
			if (sleeping.load(std::memory_order_seq_cst))
			{
				if (sleeping.exchange(false, std::memory_order_seq_cst))
				{
					wakeup_event.set();
					return true;
				}
			}

			return false;
		}

		void notify_intent_to_sleep() noexcept
		{
			sleeping.store(true, std::memory_order_relaxed);
		}

		void sleep_until_woken() noexcept
		{
			wakeup_event.wait();
		}

		bool has_any_queued_work() noexcept
		{
			auto tail = queue_head.load(std::memory_order_relaxed);
			auto head = queue_tail.load(std::memory_order_seq_cst);
			return difference(head, tail) > 0;
		}

		bool try_local_enqueue(schedule_operation*& operation) noexcept
		{
			auto head = queue_head.load(std::memory_order_relaxed);

			queue[queue_head & mask].store(operation, std::memory_order_relaxed);
			queue_head.store(head + 1);

			return true;
		}

		schedule_operation* try_local_pop() noexcept
		{
			auto head = queue_head.load(std::memory_order_relaxed);
			auto tail = queue_tail.load(std::memory_order_relaxed);

			if (difference(head,tail) <= 0)
				return nullptr;

			auto new_head = head - 1;

			queue_head.store(new_head, std::memory_order_seq_cst);

			return queue[new_head & mask].load(std::memory_order_relaxed);
		}

	private:
		using offset_t = std::make_signed_t<std::size_t>;

		static constexpr offset_t difference(size_t a, size_t b)
		{
			return static_cast<offset_t>(a - b);
		}

		std::unique_ptr<std::atomic<schedule_operation*>[]> queue;
		std::atomic<std::size_t> queue_head;
		std::atomic<std::size_t> queue_tail;

		std::atomic<bool> sleeping;
		leo::threading::auto_reset_event wakeup_event;
	};

	void ThreadScheduler::schedule_operation::await_suspend(std::experimental::coroutine_handle<> continuation) noexcept
	{
		continuation_handle = continuation;
		scheduler->schedule_impl(this);
	}

	ThreadScheduler::ThreadScheduler()
		:current_state(new thread_state())
	{
		std::thread fire_forget([this] {this->run(); });

		fire_forget.detach();
	}


	void ThreadScheduler::run() noexcept
	{
		while (true)
		{
			// Process operations from the local queue.
			schedule_operation* op;

			while (true)
			{
				op = current_state->try_local_pop();
				if (op == nullptr)
					break;

				op->continuation_handle.resume();

				Environment->Scheduler->GetIOScheduler().process_one_pending_event();
			}

			current_state->notify_intent_to_sleep();
			current_state->sleep_until_woken();

			Environment->Scheduler->GetIOScheduler().process_one_pending_event();
		}
	}

	void ThreadScheduler::schedule_impl(schedule_operation* oper) noexcept
	{
		current_state->try_local_enqueue(oper);
		wake();
	}

	void ThreadScheduler::wake() noexcept
	{
		current_state->wake_up();
	}

}