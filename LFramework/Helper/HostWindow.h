/*!	\file HostWindow.h
\ingroup Helper
\brief 宿主环境窗口。
*/

#ifndef LFrameWork_Helper_HostWindow_h
#define LFrameWork_Helper_HostWindow_h 1

#include <LFramework/LCLib/FCommon.h>
#include <LFramework/Core/lmsgdef.h>


namespace leo {
	class GUIHost;

#if LF_Hosted
	namespace Host {
		class Window;
		class WindowThread;
	}
#endif
}

#include <LFramework/LCLib/HostGUI.h>
#if LFL_Win32
#include <LFramework/Core/LString.h>
#endif

namespace leo {

#if LF_Hosted
	namespace Host
	{
		using namespace platform_ex;

		/*!
		\brief 宿主环境支持的窗口。
		*/
		class LF_API Window : public HostWindow
		{
		private:
			lref<GUIHost> host;

#if LFL_Win32
		public:
			/*!
			\brief 标记是否使用不透明性成员。
			\note 使用 Windows 层叠窗口实现，但和 WindowReference 实现不同：使用
			::UpdateLayeredWindows 而非 WM_PAINT 更新窗口。
			\warning 使用不透明性成员时在此窗口上调用 ::SetLayeredWindowAttributes 、
			GetOpacity 或 SetOpacity 可能出错。
			*/
			bool UseOpacity{};
			/*!
			\brief 不透明性。
			\note 仅当窗口启用 WS_EX_LAYERED 样式且 UseOpacity 设置为 true 时有效。
			*/
			Drawing::AlphaType Opacity{ 0xFF };

			WindowInputHost InputHost;
#endif

		public:
			/*!
			\exception LoggedEvent 异常中立：窗口类名不是 WindowClassName 。
			*/
			//@{
			Window(NativeWindowHandle);
			Window(NativeWindowHandle, GUIHost&);
			//@}
			~Window() override;

			DefGetter(const lnothrow, GUIHost&, GUIHostRef, host)

			/*!
			\brief 刷新：保持渲染状态同步。
			*/
			virtual PDefH(void, Refresh, )
			ImplExpr(void())
		};

	} // namespace Host;
#endif
}

#endif
