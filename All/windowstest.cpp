#if 0
#define _CRTDBG_MAP_ALLOC
#include	<stdlib.h>
#include	<crtdbg.h>

#include "Win.hpp"
#include <Ole2.h>
#include <CommCtrl.h>
#include <Shlwapi.h>
#include "window.hpp"
#include <comdef.h>
#include "COM.hpp"

#include "IndePlatform\Singleton.hpp"


class A : public leo::Singleton<A>
{
protected:
	A() = default;
public:
	~A()
	{
		DebugPrintf("析构A\n");
	}
public:
	static std::unique_ptr<A>& GetInstance()
	{
		static auto mInstance = std::unique_ptr<A>(new A());
		return mInstance;
	}
};

class B : public leo::Singleton<B>
{
protected:
	B() = default;
public:
	~B()
	{
		DebugPrintf("析构B\n");
	}
public:
	static std::unique_ptr<B>& GetInstance()
	{
		static auto mInstance = std::unique_ptr<B>(new B());
		return mInstance;
	}
};



class Window : public leo::win::Window_Template<Window>
{
public:
	virtual LRESULT	WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uiMsg)
		{
			HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
			HANDLE_MSG(hwnd, WM_SIZE, OnSize);
			HANDLE_MSG(hwnd, WM_DESTROY, OnDestory);
			HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
		case WM_PRINTCLIENT:
			OnPrintClient(hwnd, (HDC)wParam);
			return 0;
		case WM_CONTEXTMENU:
			if (lParam != -1 &&
				SendMessage(hwnd, WM_NCHITTEST, 0, lParam) == HTSYSMENU){
				HMENU hmenu = CreatePopupMenu();
				if (hmenu){
					AppendMenu(hmenu, MF_STRING, 1, TEXT("Hello World"));
					TrackPopupMenu(hmenu, TPM_LEFTALIGN |
						TPM_TOPALIGN |
						TPM_RIGHTBUTTON,
						GET_X_LPARAM(lParam),
						GET_Y_LPARAM(lParam),
						0, hwnd, NULL);
					DestroyMenu(hmenu);
				}
			}
			return 0;
		}
		return DefWindowProc(hwnd, uiMsg, wParam, lParam);
	}

	BOOL InitApp(HINSTANCE hInstance, int nCmdShow)
	{
		mHinst = hInstance;

		WNDCLASS wc;
		wc.style = 0;
		wc.lpfnWndProc = leo::win::WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = mHinst;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = TEXT("Scratch");

		if (!RegisterClass(&wc))
			return FALSE;

		InitCommonControls();

		if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))){
			GetHwnd() = CreateWindow(
				TEXT("Scratch"),//类的名字
				TEXT("Scratch"),//标题
				WS_OVERLAPPEDWINDOW,//风格
				CW_USEDEFAULT, CW_USEDEFAULT,//位置
				CW_USEDEFAULT, CW_USEDEFAULT,//大小
				NULL,//没有父窗口
				NULL,//没有菜单
				mHinst,//实例句柄
				this//特许参数
				);
		}

		ShowWindow(GetHwnd(), nCmdShow);
		return TRUE;
	}

	static void Msg()
	{
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		CoUninitialize();
	}
protected:
	void PaintContent(HWND hwnd,PAINTSTRUCT * pps)
	{}

	void  OnPaint(HWND hwnd)
	{
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);
		PaintContent(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}

	void OnPrintClient(HWND hwnd, HDC hdc)
	{
		PAINTSTRUCT ps;
		ps.hdc = hdc;
		GetClientRect(hwnd, &ps.rcPaint);
		ps.fErase = FALSE;
		PaintContent(hwnd, &ps);
	}

	void OnSize(HWND hwnd, UINT state, int cx, int cy)
	{
		//DebugPrintf("窗体改变大小(宽度: %d 高度: %d)\n",cx,cy);
		if (mHwndChild){
			MoveWindow(mHwndChild, 0, 0, cx, cy, TRUE);
		}
	}
	BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
	{
		return TRUE;
	}
	void OnDestory(HWND hwnd)
	{
		PostQuitMessage(0);
	}
private:
	//应用程序的句柄HINSTANCE
	HINSTANCE mHinst;
	//可选子窗口
	HWND mHwndChild;
};


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	AllocConsole();

	A::GetInstance();
	B::GetInstance();

	leo::SingletonManger::GetInstance();

#ifdef DEBUG
	leo::SingletonManger::GetInstance()->PrintAllSingletonInfo();
#endif

	leo::SingletonManger::GetInstance()->UnInstallAllSingleton();
	FreeConsole();
	return 0;
}

#endif

#include "Win.hpp"
#include "debug.hpp"
#include "IndePlatform\utility.hpp"
#include <iostream>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
	AllocConsole();
	leo::ReSetStdIOtoFile(leo::which_stdio::ostream, L"CONOUT$");
	{
		auto&& close_file = leo::finally([]() noexcept {
			std::cout << 1;
		});
	}
	FreeConsole();
	return 0;
}