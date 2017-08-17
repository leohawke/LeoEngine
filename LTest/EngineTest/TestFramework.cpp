#include "TestFramework.h"
#include "LFramework/Helper/Initialization.H"
#include <LFramework/Win32/LCLib/Mingw32.h>


using namespace Test;

TestFrameWork::TestFrameWork(const std::wstring_view & name)
#ifdef LFL_Win32
	:active(false)
#endif
{
	auto init_node = Test::FetchRoot()["LTest"]["Init"];
#ifdef LFL_Win32
	host_hwnd =
#endif
		MakeWindow(name, 800, 600);
}

TestFrameWork::~TestFrameWork()
{
	OnDestroy();
#ifdef LFL_Win32
	::SetWindowLongPtrW(host_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
	::DestroyWindow(host_hwnd);
	host_hwnd = nullptr;
#endif
}

void TestFrameWork::Create()
{
	OnCreate();
}

void TestFrameWork::Run()
{
	while (true)
	{
		::MSG msg{ nullptr, 0, 0, 0, 0,{ 0, 0 } };

		if (
			(active && ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) != 0) ||
			(!active && ::GetMessageW(&msg, nullptr, 0, 0) != 0)
			)
		{
			if (msg.message == WM_QUIT)
				break;
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		else
			Update(0);
	}
}

leo::uint32 TestFrameWork::Update(leo::uint32 pass)
{
	return DoUpdate(pass);
}

void TestFrameWork::OnCreate()
{
}

void TestFrameWork::OnDestroy()
{
}

#ifdef LFL_Win32
LRESULT TestFrameWork::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p_this = reinterpret_cast<TestFrameWork*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
	if (p_this)
		return p_this->MsgProc(hWnd, uMsg, wParam, lParam);
	else
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT TestFrameWork::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_ACTIVATE:
		active = (WA_INACTIVE != LOWORD(wParam));
		break;
	case WM_SIZE:
		if ((SIZE_MAXHIDE == wParam) || (SIZE_MINIMIZED == wParam))
			active = false;
		else
			active = true;
		break;
	case WM_CLOSE:
		active = false;
		break;
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#endif


#ifdef LFL_Win32
HWND
#endif
TestFrameWork::MakeWindow(const std::wstring_view & name, leo::uint16 width, leo::uint16 height)
{
#ifdef LFL_Win32
	WNDCLASSEX  wc = { sizeof(WNDCLASSEX),CS_OWNDC,TestFrameWork::WndProc,
		0,0,
		GetModuleHandle(NULL),NULL,::LoadCursorW(NULL,IDC_ARROW),
		NULL,NULL,name.data(),NULL };

	LCL_CallF_Win32(RegisterClassExW, &wc);

	RECT r{ 0, 0, width, height };
	AdjustWindowRectEx(&r, WS_BORDER | WS_POPUP, false, 0);

	auto X = CW_USEDEFAULT, Y = CW_USEDEFAULT;

	auto sX = GetSystemMetrics(SM_CXSCREEN);
	auto sY = GetSystemMetrics(SM_CYSCREEN);

	if (sX > width)
		X = (sX - width) / 2;
	if (sY > height)
		Y = (sY - height) / 2;

	auto result_v = LCL_CallF_Win32(::CreateWindowExW, 0, name.data(), name.data(), WS_BORDER | WS_POPUP,
		X, Y,
		r.right - r.left,
		r.bottom - r.top,
		NULL, NULL,
		GetModuleHandle(NULL),
		NULL
	);

	::SetWindowLongPtrW(result_v, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	::ShowWindow(result_v, SW_SHOW);
	::UpdateWindow(result_v);

	return result_v;
#endif
}
