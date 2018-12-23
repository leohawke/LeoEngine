/*! \file System\SystemEnvironment.h
\ingroup Engine
\brief 全局环境，包含经常用接口的指针。
*/

#ifndef LE_System_Environment_H
#define LE_System_Environment_H 1

#include "../Core/GraphicsEngine.h"
#include "NinthTimer.h"

namespace LeoEngine::System {
	struct GlobalEnvironment {
		LeoEngine::GraphicsEngine::ILeoEngine* LeoEngine;
		platform::chrono::NinthTimer* Timer;
	};

	GlobalEnvironment& FetchGlobalEnvironment();
}

extern LeoEngine::System::GlobalEnvironment* Environment;

#endif