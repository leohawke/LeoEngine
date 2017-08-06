#include <LBase/NativeAPI.h>
#include <LBase/Win32/Mingw32.h>
#include "../../Engine/Render/D3D12/test.h"

#include <Engine/Asset/TextureX.h>


#include "LSchemEngineUnitTest.h"

#define TEST_CODE 1

#if TEST_CODE
LRESULT Resize(WORD width, WORD height);
HWND Create(WORD width, WORD height);
#endif

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR cmdLine, int nCmdShow)
{
	leo::FetchCommonLogger().SetSender(platform_ex::SendDebugString);

	auto hwnd = Create(800, 600);

	auto swap_chain = Create(hwnd);

	//has some problem
	//unit_test::ExceuteLSchemEngineUnitTest();

	while (true)
	{
		::MSG msg{ nullptr, 0, 0, 0, 0,{ 0, 0 } };
		if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0)
		{
			if (msg.message == WM_QUIT)
				break;
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
			::WaitMessage();
	}
}


#if TEST_CODE
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_SIZE:
		return Resize(LOWORD(lParam), HIWORD(lParam));
	}
	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT Resize(WORD width, WORD height) {
	return 0;
}


HWND Create(WORD width, WORD height) {
	WNDCLASSEX  wc = { sizeof(WNDCLASSEX),CS_OWNDC,WndProc,
	0,0,
	GetModuleHandle(NULL),NULL,::LoadCursorW(NULL,IDC_ARROW),
	NULL,NULL,L"LTest",NULL };

	LFL_CallWin32F(RegisterClassExW, &wc);

	RECT r{ 0, 0, width, height };
	AdjustWindowRectEx(&r, WS_BORDER | WS_POPUP, false, 0);

	auto X = CW_USEDEFAULT, Y = CW_USEDEFAULT;

	auto sX = GetSystemMetrics(SM_CXSCREEN);
	auto sY = GetSystemMetrics(SM_CYSCREEN);

	if (sX > width)
		X = (sX - width) / 2;
	if (sY > height)
		Y = (sY - height) / 2;

	auto result_v = LFL_CallWin32F(::CreateWindowExW,0, L"LTest", L"LTest", WS_BORDER | WS_POPUP,
		X, Y,
		r.right - r.left,
		r.bottom - r.top,
		NULL, NULL,
		GetModuleHandle(NULL),
		NULL
	);

	::ShowWindow(result_v, SW_SHOW);
	::UpdateWindow(result_v);

	return result_v;
}




#endif