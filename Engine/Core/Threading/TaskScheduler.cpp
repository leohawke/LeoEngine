#include <LBase/cformat.h>
#include "TaskScheduler.h"
#include "AutoResetEvent.h"
#include <atomic>
#include <memory>
#include <thread>
#include <mutex>

namespace
{
	namespace local
	{
		// Keep each thread's local queue under 1MB
		constexpr std::size_t max_local_queue_size = 1024 * 1024 / sizeof(void*);
		constexpr std::size_t initial_local_queue_size = 256;
	}
}

namespace leo::threading {
	void SetThreadDescription(void* hThread, const wchar_t* lpThreadDescription);
}

thread_local leo::coroutine::ThreadScheduler* thread_local_scheduler = nullptr;
thread_local leo::coroutine::ThreadScheduler::thread_state* leo::coroutine::ThreadScheduler::thread_local_state = nullptr;

leo::threading::TaskScheduler* task_scheduler = nullptr;

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

		bool try_wake_up()
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

			if (difference(head, tail) <= 0)
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
		scheduler ? scheduler->schedule_impl(this): (any_scheduler->schedule_impl(this));
	}

	ThreadScheduler::ThreadScheduler()
		:current_state(new thread_state())
	{
		std::thread fire_forget(
		[this] {
			thread_local_scheduler = this;
			thread_local_state = current_state;
			this->run(); 
			}
		);

		static int thread_count = 0;
		auto descirption = leo::sfmt(L"Scheduler Thread %d", thread_count++);

		leo::threading::SetThreadDescription(fire_forget.native_handle(), descirption.c_str());

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
				{
					op = task_scheduler ? task_scheduler->get_remote():nullptr;
					if (op == nullptr)
						break;
				}

				op->continuation_handle.resume();
			}

			current_state->notify_intent_to_sleep();
			current_state->sleep_until_woken();
		}
	}

	void ThreadScheduler::schedule_impl(schedule_operation* oper) noexcept
	{
		current_state->try_local_enqueue(oper);
		wake_up();
	}

	void ThreadScheduler::wake_up() noexcept
	{
		current_state->try_wake_up();
	}

}

namespace leo::threading {
	class TaskScheduler::Scheduler
	{
	public:
		Scheduler(unsigned int InMaxScheduler)
			:queue_head(nullptr)
			, queue_tail(nullptr)
			,io_scheduler(InMaxScheduler)
			,schedulers(nullptr)
			,max_scheduler(InMaxScheduler)
		{
			std::thread fire_forget([this] {this->io_scheduler.process_events(); });

			SetThreadDescription(fire_forget.native_handle(), L"IO Thread");

			fire_forget.detach();

			schedulers = new leo::coroutine::ThreadScheduler[InMaxScheduler];
		}

	private:
		friend class TaskScheduler;

		std::atomic<leo::coroutine::ThreadScheduler::schedule_operation*> queue_tail;
		std::atomic<leo::coroutine::ThreadScheduler::schedule_operation*> queue_head;
		leo::coroutine::IOScheduler io_scheduler;
		leo::coroutine::ThreadScheduler* schedulers;
		unsigned int max_scheduler;
	};

	TaskScheduler::TaskScheduler()
		:scheduler_impl(new Scheduler(4))
	{
		task_scheduler = this;
	}

	leo::coroutine::IOScheduler& threading::TaskScheduler::GetIOScheduler() noexcept
	{
		return scheduler_impl->io_scheduler;
	}

	leo::coroutine::ThreadScheduler::schedule_operation threading::TaskScheduler::schedule() noexcept
	{
		return leo::coroutine::ThreadScheduler::schedule_operation { this };
	}

	void TaskScheduler::schedule_impl(leo::coroutine::ThreadScheduler::schedule_operation* operation) noexcept
	{
		if (thread_local_scheduler == nullptr || !leo::coroutine::ThreadScheduler::thread_local_state->try_local_enqueue(operation))
		{
			remote_enqueue(operation);
		}
		
		wake_one_thread();
	}

	void TaskScheduler::remote_enqueue(leo::coroutine::ThreadScheduler::schedule_operation* operation) noexcept
	{
		auto* tail = scheduler_impl->queue_tail.load(std::memory_order_relaxed);
		do {
			operation->next_oper = tail;
		} while (scheduler_impl->queue_tail.compare_exchange_weak(tail, operation, std::memory_order_seq_cst,
			std::memory_order_relaxed));
	}

	void TaskScheduler::wake_one_thread() noexcept
	{
		while (true)
		{
			for (std::uint32_t i = 0; i < scheduler_impl->max_scheduler; ++i)
			{
				if (scheduler_impl->schedulers[i].current_state->try_wake_up())
				{
					return;
				}
			}
			return;
		}
	}

	std::mutex queue_head_mutex;
	leo::coroutine::ThreadScheduler::schedule_operation* TaskScheduler::get_remote() noexcept
	{
		std::scoped_lock lock{ queue_head_mutex };

		auto* head = scheduler_impl->queue_head.load(std::memory_order_relaxed);

		if (head == nullptr)
		{
			if (scheduler_impl->queue_tail.load(std::memory_order_seq_cst) == nullptr)
				return nullptr;

			auto* tail = scheduler_impl->queue_tail.exchange(nullptr, std::memory_order_acquire);
			if (tail == nullptr)
				return nullptr;

			// Reverse the list 
			do
			{
				auto* next = std::exchange(tail->next_oper, head);
				head = std::exchange(tail, next);
			} while (tail != nullptr);
		}

		scheduler_impl->queue_head = head->next_oper;

		return head;
	}
}