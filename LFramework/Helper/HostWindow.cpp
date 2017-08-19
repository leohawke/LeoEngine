#include "HostWindow.h"
#include "GUIApplication.h"

namespace leo
{
	using namespace Drawing;

#if LF_Hosted
	namespace Host
	{

		Window::Window(NativeWindowHandle h_wnd)
			: Window(h_wnd, FetchGUIHost())
		{}
		Window::Window(NativeWindowHandle h_wnd, GUIHost& h)
			: HostWindow(h_wnd), host(h)
#if LFL_Win32
			, InputHost(*this)
#endif
		{
			h.AddMappedItem(h_wnd, make_observer(this));
		}
		Window::~Window()
		{
			host.get().RemoveMappedItem(GetNativeHandle());
		}

#	endif

	} // namespace Host;

} // namespace leo;
