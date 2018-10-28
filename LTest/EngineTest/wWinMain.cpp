#include "../../Engine/test.h"
#include "../../Engine/Asset/MeshX.h"
#include "../../Engine/Render/IContext.h"
#include "../../Engine/Core/Mesh.h"
#include "../../Engine/Render/IFrameBuffer.h"
#include "../../Engine/Asset/EffectX.h"
#include "../../Engine/Asset/MaterialX.h"
#include "../../Engine/Asset/LSLAssetX.h"
#include "../../Engine/System/NinthTimer.h"
#include "TestFramework.h"
#include "EntityComponentSystem/EntitySystem.h"
#include "LSchemEngineUnitTest.h"
#include "../../Engine/Core/Camera.h"

#include <LScheme/LScheme.h>

#define TEST_CODE 1

#include <filesystem>

using namespace platform::Render;
namespace fs = std::filesystem;

namespace lm = leo::math;

class Entity {
public:
	Entity(const scheme::TermNode& node) {
		auto mesh_name = Access("mesh", node);
		auto material_name = Access("material", node);

		pMesh = platform::X::LoadMesh(mesh_name + ".asset", mesh_name);
		pMaterial = platform::X::LoadMaterial(material_name+".mat.lsl", material_name);
	}

	const platform::Material& GetMaterial() const {
		return *pMaterial;
	}

	const platform::Mesh& GetMesh() const {
		return *pMesh;
	}
private:
	std::string Access(const char* name, const scheme::TermNode& node) {
		auto it = std::find_if(node.begin(), node.end(), [&](const scheme::TermNode& child) {
			if (!child.empty())
				return leo::Access<std::string>(*child.begin()) == name;
			return false;
		});
		return leo::Access<std::string>(*(it->rbegin()));
	}

	std::shared_ptr<platform::Material> pMaterial;
	std::shared_ptr<platform::Mesh> pMesh;
};

class Entities {
public:
	Entities(const fs::path& file) {
		auto term_node = *LoadNode(file).begin();

		for (auto& entity_node : platform::X::SelectNodes("entity", term_node))
			entities.emplace_back(entity_node);
	}

	const std::vector<Entity>& GetRenderables() const {
		return entities;
	}
private:
	template<typename path_type>
	scheme::TermNode LoadNode(const path_type& path) {
		std::ifstream fin(path);
		fin >> std::noskipws;
		using sb_it_t = std::istream_iterator<char>;

		scheme::Session session(sb_it_t(fin), sb_it_t{});

		try {
			return scheme::SContext::Analyze(std::move(session));
		}

		CatchExpr(..., leo::rethrow_badstate(fin, std::ios_base::failbit))
	}

	std::vector<Entity> entities;
};

class EngineTest : public Test::TestFrameWork {
public:
	using base = Test::TestFrameWork;
	using base::base;

	std::unique_ptr<Entities> pEntities;
private:
	leo::uint32 DoUpdate(leo::uint32 pass) override {
		auto& timer = platform::chrono::FetchGlobalTimer();
		timer.UpdateOnFrameStart();
		platform::Material::GetInstanceEvaluator().Define("time", timer.GetFrameTime(), true);

		auto entityId = ecs::EntitySystem::Instance().AddEntity<ecs::Entity>();

		Context::Instance().BeginFrame();
		Context::Instance().GetScreenFrame()->Clear(FrameBuffer::Color | FrameBuffer::Depth | FrameBuffer::Stencil, { 0,0,0,1 }, 1,0);
		auto& Device = Context::Instance().GetDevice();
		//auto pTex = leo::unique_raw(Device.CreateTexture(512, 0, 1, EFormat::EF_ABGR8, EAccessHint::EA_GenMips | EAccessHint::EA_GPUWrite | EAccessHint::EA_GPURead, {}));

		ecs::EntitySystem::Instance().RemoveEntity(entityId);

		auto pMesh = platform::X::LoadMesh("Broadleaf_Desktop_LOD0.asset","Broadleaf_Desktop_LOD0");
		auto pEffect = platform::X::LoadEffect("ForwardPointLightDiffuseShading");
		auto pMaterialAsset = platform::X::LoadMaterialAsset("FPDS.mat.lsl");
		auto pMaterial = platform::X::LoadMaterial("FPDS.mat.lsl", "lyp");

		lm::float4x4 worldmatrix = {
			{1,0,0,0},
			{0,1,0,0},
			{0,0,1,0},
			{0,0,0,1}
		};
		auto projmatrix = platform::X::perspective_fov_lh(3.14f / 6, 384 / 256.f, 1, 1000);
		auto viewmatrix = platform::X::look_at_lh({ 0,0,-10 }, { 0,0,0 }, { 0,1,0 });

		auto worldview = worldmatrix * viewmatrix;
		auto worldviewproj = worldview * projmatrix;
		auto worldviewinvt = worldview;

		using namespace std::literals;
		//obj
		pEffect->GetParameter("worldview"sv) = lm::transpose(worldview);
		pEffect->GetParameter("worldviewproj"sv) = lm::transpose(worldviewproj);
		pEffect->GetParameter("worldviewinvt"sv) = lm::transpose(worldviewinvt);
		
		//light
		pEffect->GetParameter("view_light_pos"sv) = lm::float3(0, 0,0);
		pEffect->GetParameter("light_radius"sv) =20.f;
		pEffect->GetParameter("light_color"sv) = lm::float3(0.8f, 0.8f, 0.6f);
		pEffect->GetParameter("light_blubsize"sv) = 10.f;

		//mat
		pEffect->GetParameter("albedo"sv) = lm::float3(std::fmod(timer.GetFrameTime(), 1.0f), std::fmod(timer.GetFrameStartTime().count() / 6000000.f, 1.0f), 0.4f);
		pEffect->GetParameter("metalness"sv) = 0.6f;
		pEffect->GetParameter("specular"sv) = lm::float3(1.0f, 0.2f, 0.1f);
		pEffect->GetParameter("alpha"sv) = 1.0f;
		pEffect->GetParameter("smoothness"sv) = 0.8f;

		Context::Instance().Render(*pEffect, pEffect->GetTechniqueByIndex(0), pMesh->GetInputLayout());

		for(auto& entity : pEntities->GetRenderables())
		{
			entity.GetMaterial().UpdateParams(reinterpret_cast<const platform::Renderable*>(&entity));
			Context::Instance().Render(*pEffect, pEffect->GetTechniqueByIndex(0), entity.GetMesh().GetInputLayout());
		}

		Context::Instance().GetDisplay().SwapBuffers();
		//what can i do in this duration?
		Context::Instance().GetDisplay().WaitOnSwapBuffers();

		Context::Instance().EndFrame();

		return Nothing;
	}
	void OnCreate() override {
		auto swap_chain = ::Create(GetNativeHandle());
		Context::Instance().CreateDeviceAndDisplay();

		static platform::chrono::NinthTimer timer = {};

		pEntities = std::make_unique<Entities>("sponza_crytek.entities.lsl");
	}
};


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR cmdLine, int nCmdShow)
{
	leo::FetchCommonLogger().SetSender(platform_ex::SendDebugString);

	EngineTest Test(L"EnginetTest");
	Test.Create();
	Test.Run();

	return 0;
}



