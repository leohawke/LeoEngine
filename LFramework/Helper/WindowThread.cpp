#include "WindowThread.h"
#include "../Win32/LCLib/Mingw32.h"
#include "GUIApplication.h"
#include <LBase/scope_gurad.hpp>

namespace leo {
#if LF_Hosted
	namespace Host {

		WindowThread::~WindowThread()
		{
			const unique_ptr<Window> p_hold(Nonnull(GetWindowPtr().get()));

			TraceDe(Debug, "Ready to close window '%p' on leaving window"
				" thread.", leo::pvoid(p_hold.get()));

			FilterExceptions([&, this] {
#	if !LFL_Android
				TryExpr(p_hold->Close())
					// TODO: Log.
#		if LFL_HostedUI_XCB
					CatchIgnore(XCB::XCBException&)
#		endif
#		if LFL_Win32
					CatchIgnore(platform_ex::Windows::Win32Exception&)
#		else
					CatchIgnore(Exception&) // TODO: Use proper platform-dependent type.
#		endif
#	endif
					TraceDe(Informative, "Ready to join the window thread '%p' of closed"
						" window '%p'.", leo::pvoid(&thrd), leo::pvoid(p_hold.get()));
				thrd.join();
				TraceDe(Debug, "Window thread '%p' joined.",
					leo::pvoid(&thrd));
			}, lfsig);
		}

		WindowThread::Guard
			WindowThread::DefaultGenerateGuard(Window& wnd)
		{
#if LF_Multithread == 1
			wnd.GetGUIHostRef().EnterWindowThread();
			return leo::unique_guard([&]() lnothrow{
				FilterExceptions([&] {
				wnd.GetGUIHostRef().LeaveWindowThread();
			}, "default event guard destructor");
			});
#else
			return {};
#endif
		}

		void
			WindowThread::ThreadLoop(NativeWindowHandle h_wnd)
		{
			ThreadLoop(make_unique<Window>(h_wnd));
		}
		void
			WindowThread::ThreadLoop(unique_ptr<Window> p_wnd)
		{
			LAssert(!p_window, "Duplicate window initialization detected.");

			auto& wnd(Deref(p_wnd));

			p_window = p_wnd.release();
			WindowLoop(wnd, GenerateGuard);
		}

		void
			WindowThread::WindowLoop(Window& wnd)
		{
			TraceDe(Informative, "Host loop began.");
#	if LFL_HostedUI_XCB
			// XXX: Exit on I/O error occurred?
			// TODO: Log I/O error.
			while (const auto p_evt = unique_raw(::xcb_wait_for_event(
				&Deref(wnd.GetNativeHandle().get()).DerefConn()), std::free))
			{
				//	YSL_DEBUG_DECL_TIMER(tmr, to_string(msg));
				auto& m(wnd.MessageMap);
				const auto msg(p_evt->response_type & ~0x80);

				if (msg == XCB_CLIENT_MESSAGE)
				{
					auto& cevt(reinterpret_cast<::xcb_client_message_event_t&>(*p_evt));

					if (cevt.type == wnd.WM_PROTOCOLS
						&& cevt.data.data32[0] == wnd.WM_DELETE_WINDOW)
						break;
				}

				const auto i(m.find(msg & ~0x80));

				if (i != m.cend())
					i->second(p_evt.get());
			}
#	else
			lunused(wnd);
#		if LFL_Win32

			::MSG msg{ nullptr, 0, 0, 0, 0,{ 0, 0 } };
			int res;

			while ((res = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
			{

				// XXX: Error ignored.
				if (res != -1)
				{
					::TranslateMessage(&msg);
					::DispatchMessageW(&msg);
				}
				else
					LCL_Trace_Win32E(Warning, GetMessageW, lfsig);
			}
#		endif
#	endif
			TraceDe(Informative, "Host loop ended.");
		}
		void
			WindowThread::WindowLoop(Window& wnd, GuardGenerator gd_gen)
		{
			const auto gd(gd_gen ? gd_gen(wnd) : Guard());

			WindowLoop(wnd);
		}
	}
#endif
}