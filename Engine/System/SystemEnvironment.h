/*! \file System\SystemEnvironment.h
\ingroup Engine
\brief ȫ�ֻ��������������ýӿڵ�ָ�롣
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