#include <LFramework/Helper/HostWindow.h>
#include <LFramework/Core/LShell.h>
#include <LBase/scope_gurad.hpp>

namespace leo {
	

	using Messaging::MessageQueue;

	/*!
	\brief 程序实例。
	*/
	class LF_API Application : public Shell
	{
	private:
		/*!
		\brief 初始化守护。
		*/
		stack<leo::any> on_exit{};
		/*
		\brief 初始化守护互斥锁。
		*/
		recursive_mutex on_exit_mutex{};
		/*
		\brief 主消息队列互斥锁。
		*/
		recursive_mutex queue_mutex{};

	protected:
		/*
		\brief 主消息队列。
		*/
		MessageQueue qMain{};
		/*!
		\brief 当前 Shell 句柄：指示当前线程空间中运行的 Shell 。
		\note 全局单线程，生存期与进程相同。
		*/
		shared_ptr<Shell> hShell{};

	public:
		//! \brief 无参数构造：默认构造。
		Application();

		//! \brief 析构：释放 Shell 所有权和其它资源。
		~Application() override;

		//! \brief 取得线程空间中当前运行的 Shell 的句柄。
		DefGetter(const lnothrow, shared_ptr<Shell>, ShellHandle, hShell)

			/*!
			\brief 执行消息队列操作。
			\note 线程安全：全局消息队列互斥访问。
			*/
			template<typename _func>
		auto
			AccessQueue(_func f) -> decltype(f(qMain))
		{
			lock_guard<recursive_mutex> lck(queue_mutex);

			return f(qMain);
		}

		/*!
		\pre 参数调用无异常抛出。
		\note 线程安全：全局互斥访问。
		*/
		//@{
		template<typename _tParam>
		void
			AddExit(_tParam&& arg)
		{
			lock_guard<recursive_mutex> lck(on_exit_mutex);

			PushExit(lforward(arg));
		}

		template<typename _func>
		void
			AddExitGuard(_func f)
		{
			static_assert(std::is_nothrow_copy_constructible<_func>(),
				"Invalid guard function found.");
			lock_guard<recursive_mutex> lck(on_exit_mutex);

			TryExpr(PushExit(leo::unique_guard(f)))
				catch (...)
			{
				f();
				throw;
			}
		}

		/*!
		\brief 锁定添加的初始化守护。
		\note 线程安全：全局初始化守护互斥访问。
		*/
		template<typename _tParam>
		locked_ptr<leo::decay_t<_tParam>, recursive_mutex>
			LockAddExit(_tParam&& arg)
		{
			unique_lock<recursive_mutex> lck(on_exit_mutex);

			PushExit(lforward(arg));
			return { leo::unchecked_any_cast<leo::decay_t<_tParam>>(
				&on_exit.top()), std::move(lck) };
		}
		//@}

		/*!
		\brief 处理消息：分发消息。
		\pre 断言：当前 Shell 句柄有效。
		\exception 捕获并忽略 Messaging::MessageSignal ，其它异常中立。
		*/
		void
			OnGotMessage(const Message&) override;

	private:
		template<typename _tParam>
		inline void
			PushExit(_tParam&& arg)
		{
			on_exit.push(lforward(arg));
		}

	public:
		/*!
		\brief 切换：若参数非空，和线程空间中当前运行的 Shell 的句柄交换。
		\return 参数是否有效。
		*/
		bool
			Switch(shared_ptr<Shell>&) lnothrow;
		/*!
		\brief 切换：若参数非空，和线程空间中当前运行的 Shell 的句柄交换。
		\return 参数是否有效。
		\warning 空句柄在此处是可接受的，但继续运行可能会导致断言失败。
		\since build 295
		*/
		PDefH(bool, Switch, shared_ptr<Shell>&& h) lnothrow
			ImplRet(Switch(h))
	};


	/*!
	\brief 取应用程序实例。
	\throw LoggedEvent 找不到全局应用程序实例或消息发送失败。
	\note 保证在平台相关的全局资源初始化之后初始化此实例。
	\note 线程安全。
	*/
	extern LF_API Application&
		FetchAppInstance();


	lconstexpr const Messaging::Priority DefaultQuitPriority(0xF0);


	/*!
	\brief 全局默认队列消息发送函数。
	\exception LoggedEvent 找不到全局应用程序实例或消息发送失败。
	\note 线程安全。
	*/
	//@{
	LF_API void
		PostMessage(const Message&, Messaging::Priority);
	inline PDefH(void, PostMessage, Messaging::ID id, Messaging::Priority prior,
		const ValueObject& vo = {})
		ImplRet(PostMessage(Message(id, vo), prior))
		inline PDefH(void, PostMessage, Messaging::ID id, Messaging::Priority prior,
			ValueObject&& c)
		ImplRet(PostMessage(Message(id, std::move(c)), prior))
		template<Messaging::MessageID _vID>
	inline void
		PostMessage(Messaging::Priority prior,
			const typename Messaging::SMessageMap<_vID>::TargetType& target)
	{
		PostMessage(_vID, prior, ValueObject(target));
	}
	//@}

	/*!
	\brief 以指定错误码和优先级发起 Shell 终止请求。
	*/
	LF_API void
		PostQuitMessage(int = 0, Messaging::Priority p = DefaultQuitPriority);

	/*!
	\brief GUI 宿主。
	*/
	class LF_API GUIHost
	{
	private:
#if LF_Hosted
		/*!
		\brief 本机窗口对象映射。
		\note 不使用 ::SetWindowLongPtr 等 Windows API 保持跨平台，
		并避免和其它代码冲突。
		\warning 销毁窗口前移除映射，否则可能引起未定义行为。
		\warning 非线程安全，应仅在宿主线程上操作。
		*/
		map<Host::NativeWindowHandle, observer_ptr<Host::Window>> wnd_map;
		//! \brief 窗口对象映射锁。
		mutable mutex wmap_mtx;
		/*!
		\brief 窗口线程计数。
		\sa EnterWindowThrad, LeaveWindowThread
		*/
		atomic<size_t> wnd_thrd_count{ 0 };

	public:
		/*!
		\brief 退出标记。
		\sa LeaveWindowThread
		*/
		atomic<bool> ExitOnAllWindowThreadCompleted{ true };

#	if LFL_Win32
		/*!
		\brief 点映射例程。
		\sa MapCursor
		*/
		std::function<pair<observer_ptr<Host::Window>, Drawing::Point>(
			const Drawing::Point&)> MapPoint{};

	private:
		Host::WindowClass window_class;
#	elif LFL_Android
		/*!
		\brief 点映射例程。
		\note 若非空则 MapCursor 调用此实现，否则使用恒等变换。
		*/
		leo::id_func_clr_t<Drawing::Point>* MapPoint = {};
		/*!
		\brief 宿主环境桌面。
		*/
		UI::Desktop Desktop;
#	endif
#endif

	public:
		GUIHost();
		~GUIHost();

#if LF_Hosted
		/*!
		\brief 取 GUI 前景窗口。
		\todo 线程安全。
		\todo 非 Win32 宿主平台实现。
		*/
		observer_ptr<Host::Window>
			GetForegroundWindow() const lnothrow;
#endif

#if LF_Hosted
		/*!
		\brief 插入窗口映射项。
		\note 线程安全。
		*/
		void
			AddMappedItem(Host::NativeWindowHandle, observer_ptr<Host::Window>);

#	if LF_Multithread == 1
		/*!
		\brief 标记开始窗口线程，增加窗口线程计数。
		\note 线程安全。
		\since build 399
		*/
		void
			EnterWindowThread();
#	endif

		/*!
		\brief 取本机句柄对应的窗口指针。
		\note 线程安全。
		*/
		observer_ptr<Host::Window>
			FindWindow(Host::NativeWindowHandle) const lnothrow;

#	if LF_Multithread == 1
		/*!
		\brief 标记结束窗口线程，减少窗口线程计数并在计数为零时执行附加操作。

		减少窗口计数后检查，若为零且退出标记被设置时向 YSLib 消息队列发送退出消息。
		*/
		void
			LeaveWindowThread();
#	endif

		/*!
		\brief 映射宿主光标位置到相对顶级窗口输入的光标位置。
		\return 使用的顶级窗口指针（若使用屏幕则为空）和相对顶级窗口或屏幕的位置。
		\todo 支持 Win32 和 Android 以外的平台。

		首先确定屏幕光标位置，若 MapPoint 非空则调用 MapPoint 确定顶级窗口及变换坐标，
		最后返回结果。
		*/
		pair<observer_ptr<Host::Window>, Drawing::Point>
			MapCursor() const;

#	if LFL_Win32
		/*!
		\brief 映射顶级窗口的点。

		首先调用使用指定的参数作为屏幕光标位置确定顶级窗口。
		若存在指定的顶级窗口，则调用窗口的 MapCursor 方法确定结果，否则返回无效值。
		*/
		pair<observer_ptr<Host::Window>, Drawing::Point>
			MapTopLevelWindowPoint(const Drawing::Point&) const;
#	endif
		/*!
		\brief 移除窗口映射项。
		\note 线程安全。
		*/
		void
			RemoveMappedItem(Host::NativeWindowHandle) lnothrow;

		void
			UpdateRenderWindows();
#endif



	};


	/*!
	\brief 平台相关的应用程序类。
	\note 含默认接口。
	*/
	class LF_API GUIApplication : public Application
	{
	private:
		struct InitBlock final
		{
			//! \brief 环境状态。
			//! \brief GUI 宿主状态。
			mutable GUIHost host{};

			InitBlock(Application&);
		};

		leo::call_once_init<InitBlock, once_flag> init;

	public:
		/*!
		\brief 用户界面输入响应阈值。
		\sa DSApplication::Run

		用于主消息队列的消息循环中控制后台消息生成策略的全局消息优先级。
		*/
		Messaging::Priority UIResponseLimit = 0x40;

		/*!
		\brief 无参数构造。
		\pre 断言：唯一性。
		*/
		GUIApplication();
		/*!
		\brief 析构：释放资源。
		*/
		~GUIApplication() override;

		GUIHost&
			GetGUIHostRef() const lnothrow;

		/*!
		\brief 处理当前消息。
		\return 循环条件。
		\note 线程安全：全局消息队列互斥访问。
		\note 优先级小于 UIResponseLimit 的消息时视为后台消息，否则为前台消息。
		\note 消息在响应消息后移除。
		\warning 应保证不移除正在被响应的消息。

		若主消息队列为空，处理空闲消息，否则从主消息队列取出并分发消息。
		当取出的消息的标识为 SM_Quit 时视为终止循环。
		对后台消息，分发前调用后台消息处理程序：分发空闲消息并可进行时序控制。
		*/
		bool
			DealMessage();
	};


	/*!
	\brief 取全局应用程序实例。
	\throw GeneralEvent 不存在初始化完成的应用程序实例。
	\warning 调用线程安全，但不保证调用结束后实例仍在生存期内。
	*/
	//@{
	LF_API limpl(GUIApplication&)
		FetchGlobalInstance();

	inline PDefH(GUIHost&, FetchGUIHost, )
		ImplRet(FetchGlobalInstance().GetGUIHostRef())
}
