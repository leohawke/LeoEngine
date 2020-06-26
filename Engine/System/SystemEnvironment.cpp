#include "SystemEnvironment.h"
#include "../Render/Shader.h"

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

		pEnvironment->Gamma = 2.2f;

		//编译Shader
		platform::Render::CompileGlobalShaderMap();
	}

	~GlobalEnvironmentGurad() {
		pEnvironment.reset();
	}
};

std::shared_ptr<void> LeoEngine::System::InitGlobalEnvironment() {
	return std::make_shared<GlobalEnvironmentGurad>();
}