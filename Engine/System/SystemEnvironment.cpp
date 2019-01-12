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

		//�ȳ�ʼ��ʱ��[������̬ʵ��]
		static platform::chrono::NinthTimer Timer;
		pEnvironment->Timer = &Timer;

		//��ʼ��ͼ������
		pLeoEngine = std::make_unique<LeoEngine::GraphicsEngine::LeoEngine>();
		pEnvironment->LeoEngine = pLeoEngine.get();

		//��ʼ��3D����
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