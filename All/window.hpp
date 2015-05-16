#pragma once
#include <functional>
#include <unordered_map>
#include <vector>

#include "BaseMacro.h"
#include "Core\CoreDebug.hpp"


namespace leo
{
	namespace win
	{
		using HWND = ::HWND;
		using UINT = ::UINT;
		using WPARAM = ::WPARAM;
		using LPARAM = ::LPARAM;

		class Window
		{
		public:
			virtual LRESULT	WndProc(HWND, UINT,WPARAM,LPARAM) = 0;
			friend extern LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		};

		template<typename _Ty>
		class Window_Template : public Window
		{
		public:
			DefGetter(const _NOEXCEPT, HWND, Hwnd, m_hWnd);
			DefGetter(_NOEXCEPT, HWND&, Hwnd, m_hWnd);

			virtual LRESULT	WndProc(HWND, UINT, WPARAM, LPARAM) = 0;
		protected:
			Window_Template() = default;
			~Window_Template() = default;
			HWND m_hWnd = nullptr;
		};
	}
}