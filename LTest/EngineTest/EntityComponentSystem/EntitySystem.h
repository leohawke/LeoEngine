#ifndef FrameWork_ECS_Entity_SYSTEM_h
#define FrameWork_ECS_Entity_SYSTEM_h 1

#include <LBase/typeinfo.h>
#include <LBase/lmacro.h>
#include <LBase/memory.hpp>
#include "System.h"
#include "Entity.h"
#include "ECSException.h"

namespace ecs {

	class EntitySystem:System {
	public:
		//TODO Coroutine!
		leo::uint32 Update(const UpdateParams&) override;

		template<typename _type,typename... _tParams>
		leo::observer_ptr<_type> AddSystem(_tParams&&... args) {
			TryRet(leo::make_observer(static_cast<_type*>(Add(leo::type_id<_type>(),std::make_unique<_type>(lforward(args)...)).get())))
				CatchThrow(ECSException& e, leo::LoggedEvent(leo::sfmt("AddSystem failed. (Inner %s)",e.message()),Warning))
		}

		template<typename _type, typename... _tParams>
		EntityId AddEntity(_tParams&&... args) {
			auto ret = GenerateEntityId();
			if (ret == InvalidEntityId) {
				Trace(Critical, "AddEntity Failed.ID range is full!");
				return ret;
			}
			TryRet((Add(leo::type_id<_type>(), ret,std::make_unique<_type>(ret,lforward(args)...))->GetId()))
				CatchThrow(ECSException& e, leo::LoggedEvent(leo::sfmt("AddEntity failed. (Inner %s)", e.what()), Warning))
		}

		void RemoveEntity(EntityId id) lnothrow;

		void PostMessage(const leo::Message& message);

		void OnGotMessage(const leo::Message& message) override;

		static EntitySystem& Instance();
	private:
		EntityId GenerateEntityId() const lnothrow;

		leo::observer_ptr<System> Add(const leo::type_info& type_info, std::unique_ptr<System> pSystem);
		leo::observer_ptr<Entity> Add(const leo::type_info& type_info,EntityId id,std::unique_ptr<Entity> pEntity);
	};
}


#endif
