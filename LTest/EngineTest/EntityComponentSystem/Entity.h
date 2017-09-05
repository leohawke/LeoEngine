#ifndef FrameWork_ECS_Entity_h
#define FrameWork_ECS_Entity_h 1

#include <LBase/lmacro.h>
#include <LBase/typeindex.h>
#include <LFramework/Adaptor/LAdaptor.h>
#include "Component.h"

namespace ecs {

	lconstexpr EntityId InvalidEntityId = {};

	class Entity {
	public:
		template<typename... _tParams>
		Entity(EntityId id_, _tParams&&...)
			:id(id_)
		{}

		virtual ~Entity();

		DefGetter(const lnothrow,EntityId,Id,id)
		DefSetter(lnothrow,EntityId,Id,id)

		template<typename _type,typename ..._tParams>
		leo::observer_ptr<_type> AddComponent(_tParams&&... args) {
			TryRet(leo::make_observer(static_cast<_type*>(Add(leo::type_id<_type>(), std::make_unique<_type>(leo::type_id<_type>(),lforward(args)...)).get())))
				CatchThrow(ECSException& e, leo::LoggedEvent(leo::sfmt("AddComponent failed. (Inner %s)", e.message()), Warning))
		}
	private:
		leo::observer_ptr<Component> Add(const leo::type_info& type_info,std::unique_ptr<Component> pComponent);
	private:
		EntityId id;

		leo::unordered_multimap<leo::type_index, leo::unique_ptr<Component>> components;
	};
}


#endif
