#pragma once

#include "LBase/sutility.h"
#include "LBase/linttype.hpp"
#ifdef LB_IMPL_MSCPP
#include <string_view>
#else
#include <experimental/string_view> //TODO:replace it
#endif

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
		explicit TestFrameWork(const std::string_view &name);

		virtual ~TestFrameWork();

		void Create();
		void Run();
	protected:
		leo::uint32 Update(leo::uint32 pass);

	private:
		virtual void OnCreate();
		virtual void OnDestroy();
		virtual leo::uint32 DoUpdate(leo::uint32 pass) = 0;
	};
}