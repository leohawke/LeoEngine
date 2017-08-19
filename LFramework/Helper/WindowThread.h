/*!	\file WindowThread.h
\ingroup LFrameWorkHelper
\brief 宿主窗体线程抽象。
*/

#ifndef LFrameWork_Helper_WindowThread_h
#define LFrameWork_Helper_WindowThread_h 1

#include <LFramework/Helper/HostWindow.h>

namespace leo {
#if LF_Hosted
	namespace Host {
		/*!
		\brief 宿主窗口线程。

		使用合适的参数取特定类型的值表示本机窗口，并在新线程中运行这个窗口的循环。
		本机窗口类型可以是 NativeWindowHandle 、 WindowReference 或 unique_ptr<Window> 。
		*/
		class LF_API WindowThread : private OwnershipTag<Window>
		{
		public:
			using Guard = leo::any;
			using GuardGenerator = std::function<Guard(Window&)>;

		private:
			/*!
			\brief 窗口指针。
			\note 不使用 \c unique_ptr 以便于实现线程安全的状态检查。
			*/
			atomic<Window*> p_window{ {} };
			std::thread thrd;

		public:
			/*!
			\brief 在进入线程时取守护对象。
			*/
			GuardGenerator GenerateGuard{};

			template<typename... _tParams>
			WindowThread(_tParams&&... args)
				: WindowThread({}, lforward(args)...)
			{}
			//! \since build 589
			template<typename... _tParams>
			WindowThread(GuardGenerator guard_gen, _tParams&&... args)
				: thrd(&WindowThread::ThreadFunc<_tParams...>, this, lforward(args)...),
				GenerateGuard(guard_gen)
			{}
			DefDelMoveCtor(WindowThread)
				/*!
				\brief 析构：关闭窗口。
				\note 忽略实现抛出的异常。
				*/
				~WindowThread();

			/*!
			\note 线程安全。
			*/
			DefGetter(const lnothrow, observer_ptr<Window>, WindowPtr,
				make_observer(static_cast<Window*>(p_window)))

				/*!
				\brief 默认生成守护对象。

				生成的守护对象在构造和析构时分别调用 EnterWindowThread 和 LeaveWindowThread 。
				*/
				static Guard
				DefaultGenerateGuard(Window&);

		private:
			//! \since build 623
			template<typename _func, typename... _tParams>
			void
				ThreadFunc(_func&& f, _tParams&&... args) lnothrow
			{
				FilterExceptions([&, this] {
					// XXX: Blocked. 'lforward' cause G++ 5.2 crash: segmentation
					//	fault.
					ThreadLoop(leo::invoke(std::forward<_func&&>(f),
						std::forward<_tParams&&>(args)...));
				});
			}

			void
				ThreadLoop(platform_ex::NativeWindowHandle);
			PDefH(void, ThreadLoop, platform_ex::WindowReference wnd_ref)
				ImplRet(ThreadLoop(wnd_ref.GetNativeHandle()))
				void
				ThreadLoop(unique_ptr<Window>);

		public:
			static void
				WindowLoop(Window&);

		private:
			static void
				WindowLoop(Window&, GuardGenerator);
		};
	}
#endif
}

#endif
