#include "Entities.h"
#include "Engine/Asset/LSLAssetX.h"
#include "Engine/System/SystemEnvironment.h"
#include "Engine/Core/Coroutine/WhenAllReady.h"
#include "Engine/Core/Coroutine/SyncWait.h"

std::string Access(const char* name, const scheme::TermNode& node) {
	auto it = std::find_if(node.begin(), node.end(), [&](const scheme::TermNode& child) {
		if (!child.empty())
			return leo::Access<std::string>(*child.begin()) == name;
		return false;
		});
	return leo::Access<std::string>(*(it->rbegin()));
}

Entities::Entities(const fs::path& file) {
	auto term_node = *platform::X::LoadNode(file).begin();

	auto entity_nodes = platform::X::SelectNodes("entity", term_node);
	
	std::vector<std::shared_ptr<platform::Mesh>> meshs;
	std::vector< leo::coroutine::Task<void>> tasks;

	meshs.resize(entity_nodes.size());
	int index = 0;
	for (auto& entity_node : entity_nodes)
	{
		auto mesh_name = Access("mesh", entity_node);

		tasks.emplace_back(Environment->Scheduler->Schedule([&meshs](std::string mesh_name,int index)->leo::coroutine::Task<void> {
			meshs[index] = co_await platform::X::AsyncLoadMesh(mesh_name + ".asset", mesh_name);
			co_return;
			}(mesh_name,index++)));
	}

	leo::coroutine::SyncWait(leo::coroutine::WhenAllReady(std::move(tasks)));

	for (auto& entity_node : platform::X::SelectNodes("entity", term_node))
	{
		auto mesh_name = Access("mesh", entity_node);
		auto material_name = Access("material", entity_node);
		entities.emplace_back(mesh_name, material_name);
	}

	min = leo::math::float3(FLT_MAX, FLT_MAX, FLT_MAX);
	max = leo::math::float3(FLT_MIN, FLT_MIN, FLT_MIN);
	for (auto& entity : entities)
	{
		min = leo::math::min(min, entity.GetMesh().GetBoundingMin());
		max = leo::math::max(max, entity.GetMesh().GetBoundingMax());
	}
}