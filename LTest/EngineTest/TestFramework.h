#pragma once

#include "LBase/sutility.h"
#include "LBase/linttype.hpp"
#include <string_view>
#include <LFramework/LCLib/NativeAPI.h>
#include <LFramework/Helper/WindowThread.h>

namespace Test {
	using namespace leo;


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

		platform_ex::NativeWindowHandle GetNativeHandle();

		std::thread::native_handle_type GetThreadNativeHandle()
		{
			return p_wnd_thrd->GetNativeHandle();
		}

		platform_ex::MessageMap& GetMessageMap();
	protected:
		leo::uint32 Update(leo::uint32 pass);

	private:
		virtual void OnCreate();
		virtual void OnDestroy();
		virtual leo::uint32 DoUpdate(leo::uint32 pass) = 0;

#if LFL_Win32
		unique_ptr<Host::WindowThread> p_wnd_thrd;
#endif

	};
}