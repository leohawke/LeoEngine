#include "ResourcesHolder.h"

namespace platform {
	ImplDeDtor(Resource)

	ImplDeDtor(IResourcesHolder)
	ImplDeCtor(IResourcesHolder)

	std::pmr::synchronized_pool_resource IResourcesHolder::pool_resource;
}
