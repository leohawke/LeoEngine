#ifndef FrameWork_ECS_COMMON_h
#define FrameWork_ECS_COMMON_h 1

#include <LBase/linttype.hpp>
#include <LFramework/LCLib/Debug.h>

namespace ecs {
	class Entity;
	class System;
	struct Component;

	using EntityId = leo::uint32;

	using namespace platform::Descriptions;
}


#endif
