#ifdef WIN32

#include "platform.h"
#include "ThreadSync.hpp"




namespace leo
{
	Event::Event(bool bManualReset)
		:SyncHandle(::CreateEventW(nullptr, bManualReset, false, nullptr))
	{}

	void Event::Reset()
	{
		ResetEvent(GetHandle());
	}

	void Event::Set()
	{
		SetEvent(GetHandle());
	}

	void Event::Wait() const
	{
		WaitForSingleObject(GetHandle(), INFINITE);
	}

	bool Event::Wait(const std::uint32_t duration) const
	{
		return WaitForSingleObject(GetHandle(), duration) != WAIT_TIMEOUT;
	}

	Mutex::Mutex()
		:SyncHandle(CreateMutexW(nullptr,false,nullptr))
	{}

	void Mutex::Lock()
	{
		WaitForSingleObject(GetHandle(), INFINITE);
	}

	void Mutex::UnLock()
	{
		ReleaseMutex(GetHandle());
	}

	bool Mutex::TryLock()
	{
		return WaitForSingleObject(GetHandle(), 0) != WAIT_TIMEOUT;
	}

	CriticalSection::CriticalSection()
	{
		InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&mCs));
	}

	CriticalSection::~CriticalSection()
	{
		DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&mCs));
	}

	void CriticalSection::Lock()
	{
		EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&mCs));
	}

	void CriticalSection::UnLock()
	{
		LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&mCs));
	}

	bool CriticalSection::TryLock()
	{
		return TryEnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&mCs)) != FALSE;
	}

	bool CriticalSection::IsLocked() const
	{
		return mCs.RecursionCount > 0 && (DWORD)(UINT_PTR)mCs.OwningThread == GetCurrentThreadId();
	}

	Semaphore::Semaphore(int nMaximumCount)
		:SyncHandle(::CreateSemaphoreW(nullptr,0,nMaximumCount,nullptr))
	{}

	void Semaphore::Acquire()
	{
		WaitForSingleObject(GetHandle(), INFINITE);
	}

	void Semaphore::Release(std::uint32_t count)
	{
		ReleaseSemaphore(GetHandle(), count, nullptr);
	}


	//reference: CryEngine
	//			 http://www.cs.wustl.edu/~schmidt/win32-cv-1.html

	ConditionVariable::ConditionVariable()
		:mSemaphore(0x7fffffff), mWaitersCountLock(), mWaitersDone(false)
	{}

	void ConditionVariable::Wait(Mutex& mutex)
	{
		mWaitersCountLock.Lock();
		++mWaitersCount;
		mWaitersCountLock.UnLock();

		SignalObjectAndWait(mutex, mSemaphore, INFINITE, FALSE);

		mWaitersCountLock.Lock();
		--mWaitersCount;
		bool lastWaiter = mWasBroadcast && mWaitersCount == 0;
		mWaitersCountLock.UnLock();

		if (lastWaiter)
			SignalObjectAndWait(mWaitersDone,mutex, INFINITE, FALSE);
		else
			mutex.Lock();
	}

	bool ConditionVariable::TimedWait(Mutex& mutex, std::uint32_t millis)
	{
		mWaitersCountLock.Lock();
		++mWaitersCount;
		mWaitersCountLock.UnLock();

		bool ok = true;
		if (WAIT_TIMEOUT == SignalObjectAndWait(mutex, mSemaphore, millis, FALSE))
			ok = false;

		mWaitersCountLock.Lock();
		--mWaitersCount;
		bool lastWaiter = mWasBroadcast && mWaitersCount == 0;
		mWaitersCountLock.UnLock();

		if (lastWaiter)
			SignalObjectAndWait(mWaitersDone, mutex, INFINITE, FALSE);
		else
			mutex.Lock();

		return ok;
	}

	void ConditionVariable::NotifySingle()
	{
		mWaitersCountLock.Lock();
		bool haveWaiters = mWaitersCount > 0;
		mWaitersCountLock.UnLock();
		if (haveWaiters)
			mSemaphore.Release();
	}

	void ConditionVariable::Notify()
	{
		mWaitersCountLock.Lock();
		bool haveWaiters = false;
		if (mWaitersCount > 0)
		{
			mWasBroadcast = 1;
			haveWaiters = true;
		}
		if (haveWaiters)
		{
			mSemaphore.Release(mWaitersCount);
			mWaitersCountLock.UnLock();
			mWaitersDone.Wait();
			mWasBroadcast = 0;
		}
		else
		{
			mWaitersCountLock.UnLock();
		}
	}

}
	
#else
#include <pthreads.h>

namespace leo
{
	Semaphore::Semaphore(std::uint32_t nMaximumCount)
		:mCount(nMaximumCount), mWaiters_Count(0)
	{}

	void Semaphore::Acquire()
	{
		mLock.Lock();

		++mWaiters_Count;

		while (mCount == 0)
			mCount_NonZero.Wait(mLock);

		--mWaiters_Count;

		mLock.UnLock();
	}

	void Semaphore::Release(std::uint32_t count)
	{
		mLock.Lock();

		if (mWaiters_Count > 0)
			mCount_NonZero.Notify();

		++mCount;

		mLock.UnLock();
	}
}
#endif