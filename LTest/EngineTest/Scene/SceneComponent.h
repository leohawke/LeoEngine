#ifndef FrameWork_ECS_SceneComponent_h
#define FrameWork_ECS_SceneComponent_h 1

#include "../EntityComponentSystem/Component.h"
#include "../EntityComponentSystem/Entity.h"
#include <LFramework/Adaptor/LAdaptor.h>
namespace ecs {
	struct SceneComponent final :public Component {
		leo::observer_ptr<Entity> pOwner;
		leo::observer_ptr<Entity> pParent;
		leo::vector<leo::observer_ptr<Entity>> pChilds;

		SceneComponent(const leo::type_info& type_info, EntityId parent);

		SceneComponent(const leo::type_info& type_info, leo::observer_ptr<Entity> pParent)
			:SceneComponent(type_info, pParent->GetId()) {

		}
	};
}

#endif