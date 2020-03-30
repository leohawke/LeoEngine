#include "SystemEnvironment.h"

using namespace LeoEngine::System;

GlobalEnvironment* Environment = nullptr;

struct GlobalEnvironmentGurad {
	std::unique_ptr<GlobalEnvironment> pEnvironment;
	GlobalEnvironmentGurad()
		:pEnvironment(std::make_unique<GlobalEnvironment>()){

		Environment = pEnvironment.get();

		//先初始化时间[函数静态实例]
		static platform::chrono::NinthTimer Timer;
		pEnvironment->Timer = &Timer;

		//初始化图形引擎

		//初始化3D引擎
	}

	~GlobalEnvironmentGurad() {
		pEnvironment.reset();
	}
};

std::shared_ptr<void> LeoEngine::System::InitGlobalEnvironment() {
	return std::make_shared<GlobalEnvironmentGurad>();
}