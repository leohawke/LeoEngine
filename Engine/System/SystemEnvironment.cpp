#include "SystemEnvironment.h"
#include "../Core/LeoEngine.h"
#include "../Core/WhiteEngine.h"

using namespace LeoEngine::System;

GlobalEnvironment* Environment = nullptr;

struct GlobalEnvironmentGurad {
	std::unique_ptr<GlobalEnvironment> pEnvironment;
	std::unique_ptr<LeoEngine::GraphicsEngine::LeoEngine> pLeoEngine;
	std::unique_ptr<LeoEngine::Core::WhiteEngine> pWhiteEngine;
	GlobalEnvironmentGurad()
		:pEnvironment(std::make_unique<GlobalEnvironment>()){

		Environment = pEnvironment.get();

		//先初始化时间[函数静态实例]
		static platform::chrono::NinthTimer Timer;
		pEnvironment->Timer = &Timer;

		//初始化图形引擎
		pLeoEngine = std::make_unique<LeoEngine::GraphicsEngine::LeoEngine>();
		pEnvironment->LeoEngine = pLeoEngine.get();

		//初始化3D引擎
		pWhiteEngine = std::make_unique<LeoEngine::Core::WhiteEngine>();
		pEnvironment->WhiteEngine = pWhiteEngine.get();
	}

	~GlobalEnvironmentGurad() {
		pLeoEngine.reset();
		pEnvironment.reset();
	}
};

std::shared_ptr<void> LeoEngine::System::InitGlobalEnvironment() {
	return std::make_shared<GlobalEnvironmentGurad>();
}