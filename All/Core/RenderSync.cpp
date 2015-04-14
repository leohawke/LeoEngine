#include "Singleton.hpp"
#include "ThreadSync.hpp"
#include "leoint.hpp"
#include "RenderSync.hpp"
#include <atomic>
#include <thread>
namespace leo
{
	class RenderSyncDelegate : CONCRETE(RenderSync), public Singleton < RenderSyncDelegate,false >
	{
	public:
		RenderSyncDelegate()
		{
			Present();
		}
		~RenderSyncDelegate()
		{}
	public:
		void Sync()
		{
			while (mWaitCounter)
			{ 
				std::this_thread::sleep_for(std::chrono::milliseconds(0));
			}
			mEvent.Reset();
		}
		void Present()
		{
			mEvent.Set();
		}

		void Wait()
		{
			mEvent.Wait();
			++mWaitCounter;
		}
		void Release()
		{
			mEvent.Wait();
			--mWaitCounter;
		}
	private:
		Event mEvent;
		std::atomic<uint32> mWaitCounter = 0;
	};

	void RenderSync::Sync()
	{
		lassume(dynamic_cast<RenderSyncDelegate*>(this));

		return ((RenderSyncDelegate*)this)->Sync(
			);
	}
	void  RenderSync::Present()
	{
		lassume(dynamic_cast<RenderSyncDelegate*>(this));

		return ((RenderSyncDelegate*)this)->Present(
			);
	}

	void  RenderSync::Wait()
	{
		lassume(dynamic_cast<RenderSyncDelegate*>(this));

		return ((RenderSyncDelegate*)this)->Wait(
			);
	}
	void  RenderSync::Release()
	{
		lassume(dynamic_cast<RenderSyncDelegate*>(this));

		return ((RenderSyncDelegate*)this)->Release(
			);
	}


	const std::unique_ptr<RenderSync>& RenderSync::GetInstance()
	{
		static auto mInstance = unique_raw<RenderSync>(new RenderSyncDelegate());
		return mInstance;
	}
}