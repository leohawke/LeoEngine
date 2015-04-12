#define _DEBUG
#include "Core\MeshLoad.hpp"
#include "file.hpp"
#include "leomath.hpp"
#include "Core\Camera.hpp"
#include <Windows.h>
#include "Input.h"
#include "Singleton.hpp"

namespace leo {
	
}

namespace xp
{

	struct register_value_type
	{
		register_value_type(const std::function<void()>& _f)
			:f(_f) {
		}

		std::function<void()> f;
	};

	std::list<register_value_type> SingletonUnInstallFunctionContainer;


	void SingletonRegister(const std::function<void()>& f)
	{
		SingletonUnInstallFunctionContainer.emplace_back(f);
	}

	template<typename Single, bool Manged = true>
	//单列模式基类
	//0.继承Singleton ->class Sample : public Singleton<Sample>
	//1.析构函数修饰为public,并完成资源释放
	class Singleton
	{
	protected:
		Singleton()
		{
			auto f = [this]()
			{
				this->~Singleton();
			};
			leo::details::SingletonRegister(f);
		}
	public:
		virtual ~Singleton()
		{
			//assert(0);
		}
	};

	class X :Singleton<X> {
	};

	X a;
	X b;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	return 0;
}