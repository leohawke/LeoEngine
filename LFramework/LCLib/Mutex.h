/*!	\file Mutex.h
\ingroup Framework
\brief ª•≥‚¡ø°£
*/

#ifndef FrameWork_Mutex_h
#define FrameWork_Mutex_h 1

#include <LFramework/LCLib/FCommon.h>
#include <LBase/type_op.hpp>
#if LF_Multithread == 1
#include <atomic>
#include <LBase/concurrency.h>
#define LFL_Mutex_Space std
#define LFL_Threading_Space leo
#else
#include <LBase/pseudo_mutex.h>
#define LFL_Mutex_Space leo::single_thread
#define LFL_Threading_Space leo
#endif

namespace platform {
	namespace Concurrency {
		template<typename _type>
		using atomic = leo::cond_or_t<leo::or_<std::is_integral<_type>,
			std::is_pointer<_type>>, void, std::atomic, _type>;

		using LFL_Mutex_Space::mutex;
		using LFL_Mutex_Space::recursive_mutex;

		using LFL_Mutex_Space::lock_guard;
		using LFL_Mutex_Space::unique_lock;
		
		using leo::threading::lockable_adaptor;
		using leo::threading::shared_lockable_adaptor;

#if LF_Multithread == 1 || !defined(NDEBUG)
		lconstfn bool UseLockDebug(true);
#else
		lconstfn bool UseLockDebug = {};
#endif

		template<class _tMutex>
		using shared_lock = leo::threading::shared_lock<_tMutex, UseLockDebug>;

		template<class _tMutex>
		using shared_lock_guard = leo::threading::lock_guard<_tMutex,
			UseLockDebug, shared_lockable_adaptor<_tMutex>>;

		using LFL_Mutex_Space::lock;
		using LFL_Mutex_Space::try_lock;

		using LFL_Mutex_Space::once_flag;
		using LFL_Mutex_Space::call_once;

		template<class _type, typename _tReference = leo::lref<_type>>
		using AdaptedLock = leo::threading::lock_base<_type, UseLockDebug,
			lockable_adaptor<_type, _tReference>>;

		template<class _type, typename _tReference = leo::lref<_type>>
		using SharedAdaptedLock = leo::threading::lock_base<_type, UseLockDebug,
			shared_lockable_adaptor<_type, _tReference>>;

		template<class _type, typename _tReference = leo::lref<_type>>
		using AdaptedLockGuard = leo::threading::lock_guard<_type, UseLockDebug,
			lockable_adaptor<_type, _tReference>>;

		template<class _type, typename _tReference = leo::lref<_type>>
		using SharedAdaptedLockGuard = leo::threading::lock_guard<_type,
			UseLockDebug, shared_lockable_adaptor<_type, _tReference>>;

		template<class _type>
		using IndirectLock = AdaptedLock<_type, leo::indirect_ref_adaptor<_type>>;

		template<class _type>
		using SharedIndirectLock
			= SharedAdaptedLock<_type, leo::indirect_ref_adaptor<_type>>;

		template<class _type>
		using IndirectLockGuard
			= AdaptedLockGuard<_type, leo::indirect_ref_adaptor<_type>>;

		template<class _type>
		using SharedIndirectLockGuard
			= SharedAdaptedLockGuard<_type, leo::indirect_ref_adaptor<_type>>;
	}

	namespace Threading{
		using LFL_Threading_Space::unlock_delete;
		using LFL_Threading_Space::locked_ptr;

	} // namespace Threading;
}

#undef LFL_Mutex_Space
#undef LFL_Threading_Space

#endif