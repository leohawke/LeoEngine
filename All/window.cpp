#include "IndePlatform\platform.h"
//WARNING:Õ∑Œƒº˛À≥–Ú“¿¿µ
#include "Mgr.hpp"
#include "window.hpp"
#include "exception.hpp"

namespace leo
{
	namespace win
	{
		LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			Window * pthis = nullptr;
			if (uMsg == WM_NCCREATE)
			{
				CREATESTRUCT * pcreate = reinterpret_cast<LPCREATESTRUCT>(lParam);
				pthis = reinterpret_cast<Window*>(pcreate->lpCreateParams);
				::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pthis);
				pthis = nullptr;
				//pthis->get_hwnd() = hwnd;
			}
			else
			{
				pthis = reinterpret_cast<Window*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
			}
			if (pthis)
				return pthis->WndProc(hwnd, uMsg, wParam, lParam);
			return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
			/*
			Window * pObj = nullptr;
			if (uMsg == WM_NCCREATE)
			{
				CREATESTRUCT * pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
				pObj = reinterpret_cast<Window*>(pCreateStruct->lpCreateParams);
				::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pObj);
				pObj = nullptr;
			}
			else
			{
				LONG_PTR long_ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
				Window* pObj = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			}
			if (pObj)
				return pObj->WndProc(hwnd, uMsg, wParam, lParam);
			return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
			*/
		}
	}

	bool OutputWindow::Create(HINSTANCE hInstance,
		std::pair<leo::uint16, leo::uint16>& clientSize,
		LPCWSTR classname,
		DWORD style,
		DWORD exStyle,
		LPCWSTR iconRes,
		LPCWSTR smalliconRes)
	{
		m_regName = classname;

		HICON hIcon = NULL;
		if (iconRes)
		{
			hIcon = reinterpret_cast<HICON>(::LoadImage(hInstance,
				iconRes,
				IMAGE_ICON,
				0,
				0,
				LR_DEFAULTCOLOR));
		}

		HICON hSmallIcon = NULL;
		if (smalliconRes)
		{
			hSmallIcon = reinterpret_cast<HICON>(::LoadImage(hInstance,
				smalliconRes,
				IMAGE_ICON,
				0,
				0,
				LR_DEFAULTCOLOR));
		}

		HCURSOR hCursor = ::LoadCursorW(NULL, IDC_ARROW);

		// Register the window class
		WNDCLASSEX wc = { sizeof(WNDCLASSEX),
			CS_OWNDC,
			win::WndProc,
			0,
			0,
			hInstance,
			hIcon,
			hCursor,
			nullptr,
			nullptr,
			classname,
			hSmallIcon
		};

		if (!::RegisterClassEx(&wc))
			Raise_Win32_Exception("MakeOutputWindow Failed:RegisterClassEx return false");

		RECT r{ 0, 0, clientSize.first, clientSize.second };
		AdjustWindowRectEx(&r, style, false, exStyle);

		// Create the application's window
		global::globalHwnd = ::CreateWindowEx(exStyle,
			classname,
			classname,
			style,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			r.right - r.left,
			r.bottom - r.top,
			NULL,
			NULL,
			hInstance,
			reinterpret_cast<Window*>(this)
			);

		if (!global::globalHwnd)
			Raise_Win32_Exception("MakeOutputWindow Failed:CreateWindowEx return nullptr");

		ShowWindow(global::globalHwnd, SW_SHOW);
		UpdateWindow(global::globalHwnd);

		clientSize = ClientSize();
		return true;
	}

	BOOL OutputWindow::IsAlive() const
	{
		return ::IsWindow(GetHwnd());
	}

	BOOL OutputWindow::IsMin() const
	{
		return ::IsIconic(GetHwnd());
	}

	void OutputWindow::Destroy()
	{
		::DestroyWindow(this->GetHwnd());
		::UnregisterClass(m_regName.c_str(), GetModuleHandle(nullptr));
	}

	std::pair<leo::uint16, leo::uint16> OutputWindow::ClientSize() const lnothrow
	{
		RECT r;
		GetClientRect(global::globalHwnd, &r);
		std::pair<leo::uint16, leo::uint16> mClientSize;
		mClientSize.first = static_cast<leo::uint16>(r.right - r.left);
		mClientSize.second = static_cast<leo::uint16>(r.bottom - r.top);
		return mClientSize;
	}

	LRESULT OutputWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (m_userMessages.find(uMsg) != m_userMessages.end())
		{
			return m_userMessages[uMsg](hWnd, uMsg, wParam, lParam);
		}
		else
		{
			switch (uMsg)
			{
			case WM_CREATE:
			{
				LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
				OutputWindow* pOutputWindow = reinterpret_cast<OutputWindow*>(pcs->lpCreateParams);
				::SetWindowLong(GetHwnd(), GWLP_USERDATA, PtrToUlong(pOutputWindow));
				return 1;
			}
				// OutputWindow is being destroyed
			case WM_DESTROY:
				::PostQuitMessage(0);
				return 0;

				// OutputWindow is being closed
			case WM_CLOSE:
				::DestroyWindow(GetHwnd());
				return 0;
			}
		}
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	void OutputWindow::BindMsgFunc(UINT uMessage, MsgFunction msgFunction)
	{
		m_userMessages[uMessage] = msgFunction;
	}
}