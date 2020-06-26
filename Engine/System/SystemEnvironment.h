/*! \file System\SystemEnvironment.h
\ingroup Engine
\brief ȫ�ֻ��������������ýӿڵ�ָ�롣
*/

#ifndef LE_System_Environment_H
#define LE_System_Environment_H 1

#include "NinthTimer.h"
#include <LBase/lmemory.hpp>

namespace LeoEngine::System {
	struct GlobalEnvironment {
		platform::chrono::NinthTimer* Timer;

		float Gamma;
	};

	[[nodiscard]]
	std::shared_ptr<void> InitGlobalEnvironment();
}

extern LeoEngine::System::GlobalEnvironment* Environment;

#endif