#pragma once

#include "LBase/sutility.h"
#include "LBase/linttype.hpp"
#ifdef LB_IMPL_MSCPP
#include <string_view>
#else
#include <experimental/string_view> //TODO:replace it
#endif
#include <LFramework/LCLib/NativeAPI.h>

namespace Test {
	//简化测试代码框架的基类,理论上:
	//游戏应该由启动器载入逻辑模块来接管程序，这也可作为启动器和逻辑模块的初步抽象接口。

	//Create 
	//Run

	//OnCreate
	//OnDestroy()
	//DoUpdate()

	class TestFrameWork :leo::noncopyable {
	public:
		//更新函数所做的状态或者需要完成的事情 但是并不能默认不写Return啊
		enum UpdateRet {
			Nothing = 0,
		};

	public:
		explicit TestFrameWork(const std::wstring_view &name);

		virtual ~TestFrameWork();

		void Create();
		void Run();
	protected:
		leo::uint32 Update(leo::uint32 pass);

	private:
		virtual void OnCreate();
		virtual void OnDestroy();
		virtual leo::uint32 DoUpdate(leo::uint32 pass) = 0;

#ifdef LFL_Win32
	public:
		bool active;
		HWND host_hwnd;

		static LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

	private:
#ifdef LFL_Win32
		HWND
#endif
			MakeWindow(const std::wstring_view &name, leo::uint16 width, leo::uint16 height);
	};
}