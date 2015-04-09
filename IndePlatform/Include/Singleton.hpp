#ifndef IndePlatform_Singleton_hpp
#define IndePlatform_Singleton_hpp

#include "memory.hpp"
#include <functional>
namespace leo
{
	namespace details
	{
		void SingletonRegister(const std::function<void()>& f);
	}
	template<typename Single,bool Manged = true>
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
			details::SingletonRegister(f);
		}
	public:
		virtual ~Singleton()
		{
			//assert(0);
		}
	};

	template<typename Single>
	class Singleton < Single, false >
	{
	protected:
		Singleton()
		{
			details::SingletonRegister([]{});
		}
	public:
		virtual ~Singleton()
		{
			//assert(0);
		}
	};

	class SingletonManger : public Singleton<SingletonManger,false>
	{
	protected:
		SingletonManger() = default;
	public:
		//do nothing
		~SingletonManger() = default;
	public:
		void UnInstallAllSingleton();

#ifdef DEBUG
		void PrintAllSingletonInfo();
#endif

		static const std::unique_ptr<SingletonManger>& GetInstance()
		{
			static auto mInstance = unique_raw(new SingletonManger());
			return mInstance;
		}
	};
}



#endif