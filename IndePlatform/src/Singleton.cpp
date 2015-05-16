#include "Singleton.hpp"
#include "DebugOutput.hpp"
#include <string>
#include <vector>
#include <list>
#include <typeindex>

namespace leo
{
	namespace details
	{

		struct register_value_type
		{
			std::function<void()> f;
#ifdef DEBUG
			std::type_index t;
			register_value_type(const std::function<void()>& f,const std::type_info& t)
				:f(f), t(t)
			{}
#else
			register_value_type(const std::function<void()>& f, const std::type_info& t)
				: f(f)
			{}
#endif
		};


		std::vector<register_value_type>& getContainer() {
			static std::vector<register_value_type> SingletonUnInstallFunctionContainer = {};
			return SingletonUnInstallFunctionContainer;
		}
		



		void SingletonRegister(const std::function<void()>& f,const std::type_info& t)
		{
			getContainer().emplace_back(f, t);
		}
	}

	void SingletonManger::UnInstallAllSingleton()
	{
		for (auto rb = details::getContainer().rbegin(); rb != details::getContainer().rend(); ++rb)
			rb->f();
	}

#ifdef DEBUG
	void SingletonManger::PrintAllSingletonInfo()
	{
		DebugPrintf("已构造 %u 个单列,类型如下:\n",details::getContainer().size());
		for (auto rb = details::getContainer().begin(); rb != details::getContainer().end(); ++rb)
			DebugPrintf("\t%s\n", rb->t.name());
	}
#endif
}