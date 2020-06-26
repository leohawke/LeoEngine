#pragma once

#include "Engine/Asset/MaterialX.h"
#include "Engine/Core/Mesh.h"
#include "LBase/lmathtype.hpp"
#include "Engine/Asset/MeshX.h"
#include "Engine/Render/IRayTracingScene.h"
#include "Engine/Render/IRayDevice.h"
#include "Engine/Render/IContext.h"
#include "Engine/Render/IRayContext.h"
#include "LBase/span.hpp"

#include <string>
#include <filesystem>
#include <vector>
#include <memory>

namespace fs = std::filesystem;

class Entity {
public:
	Entity(const std::string& mesh_name,const std::string& material_name) {
		pMesh = platform::X::LoadMesh(mesh_name + ".asset", mesh_name);
		pMaterial = platform::X::LoadMaterial(material_name + ".mat.lsl", material_name);
	}

	const platform::Material& GetMaterial() const {
		return *pMaterial;
	}

	platform::Mesh& GetMesh() {
		return *pMesh;
	}

	const platform::Mesh& GetMesh() const {
		return *pMesh;
	}
private:
	std::shared_ptr<platform::Material> pMaterial;
	std::shared_ptr<platform::Mesh> pMesh;
};

class Entities {
public:
	Entities(const fs::path& file);

	const std::vector<Entity>& GetRenderables() const {
		return entities;
	}

	leo::unique_ptr<platform::Render::RayTracingScene> BuildRayTracingScene()
	{
		platform::Render::RayTracingSceneInitializer initializer;

		std::vector<platform::Render::RayTracingGeometryInstance> Instances;

		for (auto& entity : entities)
		{
			platform::Render::RayTracingGeometryInstance Instance;
			Instance.Geometry = entity.GetMesh().GetRayTracingGeometry();

			Instance.Transform = leo::math::float4x4::identity;

			Instances.push_back(Instance);
		}

		initializer.Instances = leo::make_span(Instances);

		return leo::unique_raw(platform::Render::Context::Instance().GetRayContext().GetDevice().CreateRayTracingScene(initializer));
	}

	leo::math::float3 min;
	leo::math::float3 max;
private:
	std::vector<Entity> entities;
};