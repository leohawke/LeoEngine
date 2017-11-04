#include "Entity.h"
#include "EntitySystem.h"
#include "ecsmsgdef.h"

using namespace ecs;

ImplDeDtor(Entity)

leo::observer_ptr<Component> ecs::Entity::Add(const leo::type_info & type_info, std::unique_ptr<Component> pComponent)
{
	EntitySystem::Instance().PostMessage({ Messaging::AddComponent,leo::make_pair(this, pComponent.get()) });
	return leo::make_observer(components.emplace(leo::type_index(type_info), pComponent.release())->second.get());
}
