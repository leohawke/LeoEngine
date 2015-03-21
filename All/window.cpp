#include "platform.h"
#include "window.hpp"

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

	
}