/*! \file Core\Threading\ThreadSafeWorkerContainer.h
\ingroup LeoEngine
\brief 特别设计 用于 渲染数据，具备以下特征:
	-在更新时多个Worker送入数据，在渲染时被使用

	注意，依赖JobDispatcher被构建
*/
#ifndef LECT_THREADSAFEWORKERCONTAINER_HPP_
#define LECT_THREADSAFEWORKERCONTAINER_HPP_ 1

#include <LBase/container.hpp>
#include "JobDispatcher.h"

namespace LeoEngine::Worker {

	template<typename T>
	class ThreadSafeWorkerContainer {
	public:
		ThreadSafeWorkerContainer()
			:
			numWorkers(0),
			workers(nullptr),
			coalescedArrCapacity(0),
			coalescedArr(nullptr),
			isCoalesced(false)
		{}

		~ThreadSafeWorkerContainer() {
			clear();
			delete[] workers;
			workers = nullptr;
		}

		ThreadSafeWorkerContainer(const ThreadSafeWorkerContainer&) = delete;
		ThreadSafeWorkerContainer(ThreadSafeWorkerContainer&&) = delete;

		ThreadSafeWorkerContainer& operator=(const ThreadSafeWorkerContainer&) = delete;

		//! Safe access of elements for calling thread via operator[].
		uint32 ConvertToEncodedWorkerId_threadlocal(uint32 nIndex) const;

		uint32 GetNumWorkers() const;
		uint32 GetWorkerId_threadlocal() const;

		//! \note Be aware that these values can potentially change if some objects are added in parallel.
		size_t size() const;
		bool   empty() const;
		size_t capacity() const;

		size_t size_threadlocal() const;
		bool   empty_threadlocal() const;
		size_t capacity_threadlocal() const;

		//! \note Be aware that this operator is more expensive if the memory was not coalesced before.
		T& operator[](size_t n);
		const T& operator[](size_t n) const;

		T* push_back_new();
		T* push_back_new(size_t& nIndex);

		void     push_back(const T& rObj);
		void     push_back(const T& rObj, size_t& nIndex);

		//! \note These functions are changing the size of the continuous memory block and thus are *not* thread-safe.
		void                                        clear();
		template<class OnElementDeleteFunctor> void clear(const OnElementDeleteFunctor& rFunctor = CThreadSafeWorkerContainer<T>::SDefaultNoOpFunctor());
		void                                        erase(const T& rObj);
		void                                        resize(size_t n);
		void                                        reserve(size_t n);

		//  *not* thread-safe functions
		void PrefillContainer(T* pElement, size_t numElements);
		void CoalesceMemory();

	private:
		class lalignas(128) SWorker
		{
		public:
			SWorker() : datasize(0) {}

			uint32 datasize;
			std::vector<T> data;
		};

		T* push_back_impl(size_t& nIndex);
		void ReserverCoalescedMemory(size_t n);

		workerid  foreignWorkerId; //!< Id of the non-job-manager-worker thread that's also allowed to use this container.

		SWorker* workers;
		uint32   numWorkers;

		uint32   coalescedArrCapacity;
		T* coalescedArr;
		bool  isCoalesced;
	};
}

#endif