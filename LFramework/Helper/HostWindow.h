/*!	\file HostWindow.h
\ingroup Helper
\brief �����������ڡ�
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
		\brief ��������֧�ֵĴ��ڡ�
		*/
		class LF_API Window : public HostWindow
		{
		private:
			lref<GUIHost> host;

#if LFL_Win32
		public:
			/*!
			\brief ����Ƿ�ʹ�ò�͸���Գ�Ա��
			\note ʹ�� Windows �������ʵ�֣����� WindowReference ʵ�ֲ�ͬ��ʹ��
			::UpdateLayeredWindows ���� WM_PAINT ���´��ڡ�
			\warning ʹ�ò�͸���Գ�Աʱ�ڴ˴����ϵ��� ::SetLayeredWindowAttributes ��
			GetOpacity �� SetOpacity ���ܳ���
			*/
			bool UseOpacity{};
			/*!
			\brief ��͸���ԡ�
			\note ������������ WS_EX_LAYERED ��ʽ�� UseOpacity ����Ϊ true ʱ��Ч��
			*/
			Drawing::AlphaType Opacity{ 0xFF };

			WindowInputHost InputHost;
#endif

		public:
			/*!
			\exception LoggedEvent �쳣������������������ WindowClassName ��
			*/
			//@{
			Window(NativeWindowHandle);
			Window(NativeWindowHandle, GUIHost&);
			//@}
			~Window() override;

			DefGetter(const lnothrow, GUIHost&, GUIHostRef, host)

			/*!
			\brief ˢ�£�������Ⱦ״̬ͬ����
			*/
			virtual PDefH(void, Refresh, )
			ImplExpr(void())
		};

	} // namespace Host;
#endif
}

#endif
