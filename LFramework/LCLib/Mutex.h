/*!	\file Mutex.h
\ingroup Framework
\brief ª•≥‚¡ø°£
*/

#ifndef FrameWork_Mutex_h
#define FrameWork_Mutex_h 1

#include <LFramework/LCLib/FCommon.h>
#if LB_Multithread == 1
#include <LBase/concurrency.h>
#define LFL_Mutex_Space std
#define LFL_Threading_Space leo
#else
#include <LBase/pseudo_mutex.h>
#define LFL_Mutex_Space leo::single_thread
#define LFL_Threading_Space leo::threading
#endif

namespace platform {
	namespace Concurrency {
		using LFL_Mutex_Space::mutex;
		using LFL_Mutex_Space::recursive_mutex;

		using LFL_Mutex_Space::lock_guard;
		using LFL_Mutex_Space::unique_lock;

		using LFL_Mutex_Space::lock;
		using LFL_Mutex_Space::try_lock;

		using LFL_Mutex_Space::once_flag;
		using LFL_Mutex_Space::call_once;
	}

	namespace Threading{
		using LFL_Threading_Space::unlock_delete;
		using LFL_Threading_Space::locked_ptr;

	} // namespace Threading;
}

#undef LFL_Mutex_Space
#undef LFL_Threading_Space

#endif