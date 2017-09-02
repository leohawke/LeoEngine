#ifndef FrameWork_ECS_Entity_h
#define FrameWork_ECS_Entity_h 1

#include <LBase/linttype.hpp>
#include "ECSCommon.h"

namespace ecs {
	using EntityId = leo::uint32;

	lconstexpr EntityId InvalidEntityId = {};

	struct EntitySpawnParams {

	};

	class Entity {
	public:
		void ReSpawn(const EntitySpawnParams& params);
	};
}


#endif
