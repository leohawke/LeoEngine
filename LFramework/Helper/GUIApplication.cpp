#include "GUIApplication.h"
#include "../LCLib/NativeAPI.h"
#include "../Win32/LCLib/Mingw32.h"
#include "../Core/lmsgdef.h"
namespace leo {
	using namespace Drawing;
#if LF_Hosted
	using namespace Host;
#endif

	namespace
	{
		recursive_mutex ApplicationMutex;

		GUIApplication* ApplicationPtr;

		/*!
		\brief 取空闲消息。
		*/
		inline PDefH(Message, FetchIdleMessage, )
			ImplRet(Message(SM_Input))

			/*!
			\brief 后台消息处理程序。
			*/
			inline void
			Idle(Messaging::Priority prior)
		{
			// NOTE: Wait for GUI input of any shells. Post message for specific shell
			//	would cause low performance when there are many candidate messages
			//	of distinct shells.
			PostMessage(FetchIdleMessage(), prior);
		}

	} // unnamed namespace;
	
	Application::Application()
		: Shell()
	{}
	Application::~Application()
	{
		// NOTE: It is necessary to cleanup to make sure all shells are destroyed.
		qMain.clear();
		//	hShell = {};
		// NOTE: All shells must have been released.
		LAssert(!hShell, "Active shell found.");
		TraceDe(Notice, "Uninitialization entered with %zu handler(s) to be"
			" called.", on_exit.size());
		// NOTE: This is needed because the standard containers (and adaptors)
		//	guarantee nothing about destruction order of contained elements.
		while (!on_exit.empty())
			on_exit.pop();
	}

	void
		Application::OnGotMessage(const Message& msg)
	{
		TryExpr(Deref(GetShellHandle()).OnGotMessage(msg))
			CatchIgnore(Messaging::MessageSignal&)
#ifndef NDEBUG
			CatchExpr(std::exception& e, TraceDe(Emergent, "Unexpected exception."),
				ExtractAndTrace(e, Emergent), throw)
			CatchExpr(..., TraceDe(Emergent, "Unknown unexpected exception."), throw)
#endif
	}

	bool
		Application::Switch(shared_ptr<Shell>& h) lnothrow
	{
		if (LB_LIKELY(hShell != h))
		{
			std::swap(hShell, h);
			return true;
		}
		return {};
	}
	

	void
		PostMessage(const Message& msg, Messaging::Priority prior)
	{
		FetchAppInstance().AccessQueue([=, &msg](MessageQueue& mq) {
			mq.Push(msg, prior);
		});
	}

	void
		PostQuitMessage(int n, Messaging::Priority prior)
	{
		TraceDe(Informative, "Ready to post quit message with exit code = %d,"
			" priority = %u.", n, unsigned(prior));
		PostMessage<SM_Set>(prior, shared_ptr<Shell>());
		PostMessage<SM_Quit>(prior, n);
	}


	GUIHost::GUIHost()
#if LF_Hosted
		: wnd_map(), wmap_mtx()
#	if LF_Multithread == 1
#		if LFL_Win32
		, window_class(WindowClassName)
#		endif
#	endif
#	if LFL_Android
		, Desktop(Android::FetchNativeHostInstance().GetHostScreenRef())
#	endif
#endif
	{
#if LFL_Win32
		//Desktop PseudoRenderer ::GetDesktopWindow();
#endif
		LF_Trace(Debug, "GUI host lifetime began.");
	}
	GUIHost::~GUIHost()
	{
		LF_Trace(Debug, "GUI host lifetime ended.");

#	if LF_Hosted && !LFL_Android
		using leo::get_value;

		std::for_each(wnd_map.cbegin() | get_value, wnd_map.cend() | get_value,
			[](observer_ptr<Window> p) lnothrow{
			FilterExceptions([&] {
			p->Close();
		});
		});
#	endif
	}

#if LF_Hosted
	observer_ptr<Window>
		GUIHost::GetForegroundWindow() const lnothrow
	{
#	ifdef LFL_Win32
		return FindWindow(::GetForegroundWindow());
#	else
		return {};
#	endif
	}

	void
		GUIHost::AddMappedItem(NativeWindowHandle h, observer_ptr<Window> p)
	{
		lock_guard<mutex> lck(wmap_mtx);

		wnd_map.emplace(h, p);
	}

	void
		GUIHost::EnterWindowThread()
	{
		++wnd_thrd_count;
	}

	observer_ptr<Window>
		GUIHost::FindWindow(NativeWindowHandle h) const lnothrow
	{
		lock_guard<mutex> lck(wmap_mtx);
		const auto i(wnd_map.find(h));

		return i == wnd_map.end() ? nullptr : i->second;
	}

#	if LF_Multithread == 1
	void
		GUIHost::LeaveWindowThread()
	{
		if (--wnd_thrd_count == 0 && ExitOnAllWindowThreadCompleted)
			leo::PostQuitMessage(0);
		TraceDe(Debug, "New window thread count = %zu.", wnd_thrd_count.load());
	}
#	endif

	pair<observer_ptr<Window>, Point>
		GUIHost::MapCursor() const
	{
#	if LFL_Win32
		::POINT cursor;

		if (LB_UNLIKELY(!::GetCursorPos(&cursor)))
			return { {}, Point::Invalid };

		const Point pt{ cursor.x, cursor.y };

		return MapPoint ? MapPoint(pt) : pair<observer_ptr<Window>, Point>({}, pt);
#	elif LFL_Android
		// TODO: Support floating point coordinates.
		const auto& cursor(platform_ex::FetchCursor());
		const Point pt(cursor.first, cursor.second);

		return { {}, MapPoint ? MapPoint(pt) : pt };
#	else
		return { {}, Point::Invalid };
#	endif
	}

#	if LFL_Win32
	pair<observer_ptr<Window>, Point>
		GUIHost::MapTopLevelWindowPoint(const Point& pt) const
	{
		::POINT cursor{ pt.X, pt.Y };

		if (const auto h = ::ChildWindowFromPointEx(::GetDesktopWindow(),
			cursor, CWP_SKIPINVISIBLE))
		{
			auto p_wnd = FindWindow(h);

			if (!p_wnd)
				p_wnd = GetForegroundWindow();
			if (p_wnd)
			{
				LCL_CallF_Win32(ScreenToClient, p_wnd->GetNativeHandle(), &cursor);
				return { p_wnd, p_wnd->MapPoint({ cursor.x, cursor.y }) };
			}
		}
		return { {}, Point::Invalid };
	}
#	endif

	void
		GUIHost::RemoveMappedItem(NativeWindowHandle h) lnothrow
	{
		lock_guard<mutex> lck(wmap_mtx);
		const auto i(wnd_map.find(h));

		if (i != wnd_map.end())
			wnd_map.erase(i);
	}

	void
		GUIHost::UpdateRenderWindows()
	{
		lock_guard<mutex> lck(wmap_mtx);

		for (const auto& pr : wnd_map)
			if (pr.second)
				pr.second->Refresh();
	}
#endif

	GUIApplication::InitBlock::InitBlock(Application& app)
	{}

	GUIApplication::GUIApplication()
		: Application(), init(*this)
	{
		lock_guard<recursive_mutex> lck(ApplicationMutex);

		LAssert(!ApplicationPtr, "Duplicate instance found.");
		ApplicationPtr = this;
	}

	GUIApplication::~GUIApplication()
	{
		lock_guard<recursive_mutex> lck(ApplicationMutex);

		ApplicationPtr = {};
	}

	
	GUIHost&
		GUIApplication::GetGUIHostRef() const lnothrow
	{
		return init.get().host;
	}

	bool
		GUIApplication::DealMessage()
	{
		if (AccessQueue([](MessageQueue& mq) lnothrow{
			return mq.empty();
		}))
			//	Idle(UIResponseLimit);
			OnGotMessage(FetchIdleMessage());
		else
		{
			const auto i(AccessQueue([](MessageQueue& mq) lnothrow{
				return mq.cbegin();
			}));

			if (LB_UNLIKELY(i->second.GetMessageID() == SM_Quit))
				return {};
			if (i->first < UIResponseLimit)
				Idle(UIResponseLimit);
			OnGotMessage(i->second);
			AccessQueue([i](MessageQueue& mq) lnothrowv{
				mq.erase(i);
			});
		}
		return true;
	}

	GUIApplication&
		FetchGlobalInstance()
	{
		lock_guard<recursive_mutex> lck(ApplicationMutex);

		if (ApplicationPtr)
			return *ApplicationPtr;
		throw GeneralEvent("Application instance is not ready.");
	}

	/* extern */Application&
		FetchAppInstance()
	{
		return FetchGlobalInstance();
	}

}