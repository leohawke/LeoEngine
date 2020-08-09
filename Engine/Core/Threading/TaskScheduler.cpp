#include "TaskScheduler.h"

#include <atomic>
#include <memory>
#include <thread>

namespace
{
}

namespace leo::threading {
	class TaskScheduler::Scheduler
	{
	public:
		Scheduler(unsigned int InMaxScheduler)
			:schedulers(new leo::coroutine::ThreadScheduler[InMaxScheduler])
			,next_scheduler(0)
			,max_scheduler(InMaxScheduler)
		{}


	private:
		friend class TaskScheduler;

		leo::coroutine::ThreadScheduler* schedulers;
		unsigned int next_scheduler;
		unsigned int max_scheduler;
	};

	TaskScheduler::TaskScheduler()
		:scheduler_impl(new Scheduler(4))
	{
	}


	leo::coroutine::ThreadScheduler::schedule_operation threading::TaskScheduler::schedule() noexcept
	{
		auto& scheduler = scheduler_impl->schedulers[scheduler_impl->next_scheduler++ % scheduler_impl->max_scheduler];

		return scheduler.schedule();
	}

}