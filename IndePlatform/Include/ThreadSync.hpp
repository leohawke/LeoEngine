#ifndef IndePlatform_ThreadSync_Hpp
#define IndePlatform_ThreadSync_Hpp


#include <cstdint>
#include <cstddef>
#include <mutex>
#include <condition_variable>
#include "ldef.h"
#include "platform_macro.h"

namespace leo
{

	class SyncHandle
	{
	public:
		SyncHandle() = default;
		template<typename T,typename F>
		explicit SyncHandle(T* hObj,const F& f) lnothrow
			:mHandle(hObj), mDeleter(f)
		{
		}

		SyncHandle(SyncHandle &&rhs) lnothrow
			 :mHandle(rhs.Release()),mDeleter(std::move(rhs.mDeleter)) 
		{
		}

		template<typename T>
		SyncHandle &operator=(T* hObj) lnothrow {
			Reset(hObj);
			return *this;
		}
		SyncHandle &operator=(SyncHandle &&rhs) lnothrow {
			Reset(rhs.Release());
			mDeleter = std::move(rhs.mDeleter);
			return *this;
		}
		~SyncHandle(){
			if (mHandle){
				mDeleter(mHandle);
			}
		}
	private:
		void * mHandle = nullptr;
		std::function<void(void *)> mDeleter;
	public:
		void * GetHandle() const
		{
			return mHandle;
		}

		void * Release(){
			auto ptr = mHandle;
			mHandle = nullptr;
			return ptr;
		}

		template<typename T>
		void Reset(T* hObj){
			if (mHandle){
				mDeleter(mHandle);
			}
			mHandle = hObj;
		}

		template<typename T>
		operator T *() const
		{
			return reinterpret_cast<T*>(GetHandle());
		}
	};

	//默认: 手动复位
	class Event : public SyncHandle
	{
	public:
		Event(bool bManualReset = true);
		~Event() = default;

		void Reset();
		void Set();

		void Wait() const;
		bool Wait(const std::uint32_t duration) const;
	};

	class Mutex : public SyncHandle
	{
	public:
		Mutex();
		~Mutex() = default;

		void Lock();
		void UnLock();
		bool TryLock();
#if defined(_DEBUG) 
		bool IsLocjed() { return true; }
#endif
	};

	class Semaphore;
	class ConditionVariable;
	class CriticalSection;

	class CriticalSection
	{
	public:
		CriticalSection();
		~CriticalSection();

		void Lock();
		void UnLock();
		bool TryLock();

		bool IsLocked() const;

		CriticalSection(const CriticalSection&) = delete;
		CriticalSection& operator=(const CriticalSection&) = delete;
	private:
		struct
		{
			void* DebugInfo;
			long LockCount;//
			long RecursionCount;//递归计数
			void* OwningThread;
			void* LockSemaphore;//使用信号量实现
			unsigned long* SpinCount;        //自旋数 force size on 64-bit systems when packed
		} mCs;
	};

#ifdef PLATFORM_WIN32
	class Semaphore : public SyncHandle
	{
	public:
		Semaphore(int nMaximumCount);
		~Semaphore() = default;
		void Acquire();
		void Release(std::uint32_t count = 1);
	};

	class ConditionVariable
	{
	public:
		ConditionVariable();
		~ConditionVariable() = default;
		void Wait(Mutex& mutex);
		bool TimedWait(Mutex& mutex,std::uint32_t millis);
		void Wait(CriticalSection& mutex);
		bool TimedWait(CriticalSection& mutex, std::uint32_t millis);
		void NotifySingle();
		void Notify();

	private:
		int mWaitersCount = 0;
		CriticalSection mWaitersCountLock;
		Semaphore mSemaphore;
		Event mWaitersDone;
		size_t mWasBroadcast = 0;
	};
#else
	class ConditionVariable
	{};

	
	class Semaphore
	{
	public:
		Semaphore(std::uint32_t nMaximumCount);
		~Semaphore() = default;
		void Acquire();
		void Release(std::uint32_t count = 1);
	private:
		std::uint32_t mCount;//Current count of the semaphore.
		std::uint64_t mWaiters_Count;//Numbers of threads that have called Wait().

		Mutex lock;//Serialize access to mCount and mWaiters_Count.

		ConditionVariable mCount_NonZero;//Condition variable that blocks the mCount 0.
	};
#endif
	
}

#endif