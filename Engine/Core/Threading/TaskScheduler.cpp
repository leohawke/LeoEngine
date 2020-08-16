#include "TaskScheduler.h"
#include <atomic>
#include <memory>
#include <thread>

namespace leo::threading {
	class TaskScheduler::Scheduler
	{
	public:
		Scheduler(unsigned int InMaxScheduler)
			:io_scheduler(InMaxScheduler)
			,schedulers(new leo::coroutine::ThreadScheduler[InMaxScheduler])
			,next_scheduler(0)
			,max_scheduler(InMaxScheduler)
		{}

	private:
		friend class TaskScheduler;

		leo::coroutine::IOScheduler io_scheduler;
		leo::coroutine::ThreadScheduler* schedulers;
		unsigned int next_scheduler;
		unsigned int max_scheduler;
	};

	TaskScheduler::TaskScheduler()
		:scheduler_impl(new Scheduler(4))
	{
	}

	leo::coroutine::IOScheduler& threading::TaskScheduler::GetIOScheduler() noexcept
	{
		return scheduler_impl->io_scheduler;
	}

	leo::coroutine::ThreadScheduler::schedule_operation threading::TaskScheduler::schedule() noexcept
	{
		auto& scheduler = scheduler_impl->schedulers[scheduler_impl->next_scheduler++ % scheduler_impl->max_scheduler];

		return scheduler.schedule();
	}

}