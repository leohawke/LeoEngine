#ifndef FrameWork_ECS_Entity_h
#define FrameWork_ECS_Entity_h 1

#include <LBase/lmacro.h>
#include <LBase/linttype.hpp>
#include "ECSCommon.h"

namespace ecs {
	using EntityId = leo::uint32;

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
	private:
		EntityId id;
	};
}


#endif
