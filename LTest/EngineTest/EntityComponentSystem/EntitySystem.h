#ifndef FrameWork_ECS_Entity_SYSTEM_h
#define FrameWork_ECS_Entity_SYSTEM_h 1

#include <LBase/lmacro.h>
#include <LBase/memory.hpp>
#include "System.h"
#include "Entity.h"
#include "ECSException.h"

namespace ecs {
	struct EntitySystemSpawnEntityParams {
		//! The Entity unique identifier (EntityId).
		//! If InvalidEntityId then an ID will be generated automatically
		EntityId id = InvalidEntityId;

		EntitySpawnParams params = {};
	};

	class EntitySystem:System {
	public:
		//TODO Coroutine!
		leo::uint32 Update(const UpdateParams&) override;

		template<typename _type,typename... _tParams>
		leo::observer_ptr<_type> AddSystem(_tParams&&... args) {
			TryRet(Add(std::make_unique<_type>(lforward(args)...)))
				CatchThrow(ECSException& e, leo::LoggedEvent(leo::sfmt("AddSystem failed. (Inner %s)",e.message()),Warning))
		}

		leo::observer_ptr<Entity> SpawnEntity(const EntitySystemSpawnEntityParams& params);
	private:
		leo::observer_ptr<System> Add(std::unique_ptr<System> pSystem);
	};
}


#endif
