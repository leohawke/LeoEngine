#include "../../Engine/test.h"
#include "../../Engine/Asset/MeshX.h"
#include "../../Engine/Render/IContext.h"
#include "../../Engine/Core/Mesh.h"
#include "../../Engine/Render/IFrameBuffer.h"
#include "../../Engine/Render/DataStructures.h"
#include "../../Engine/GraphicsPipeline/GraphicsView.h"
#include "../../Engine/Core/LeoEngine/ShadingElements/SEMesh.h"
#include "../../Engine/Asset/EffectX.h"
#include "../../Engine/Asset/MaterialX.h"
#include "../../Engine/Asset/LSLAssetX.h"
#include "../../Engine/System/NinthTimer.h"
#include "../../Engine/Core/CameraController.h"
#include "../../Engine/System/SystemEnvironment.h"
#include "TestFramework.h"
#include "EntityComponentSystem/EntitySystem.h"
#include "LSchemEngineUnitTest.h"
#include "../../Engine/Core/Camera.h"

#include <LScheme/LScheme.h>
#include <windowsx.h>

#define TEST_CODE 1

#include <filesystem>

using namespace platform::Render;
using namespace LeoEngine::Render;
using namespace LeoEngine::GraphicsEngine;
namespace fs = std::filesystem;

namespace lm = leo::math;

class Entity {
public:
	Entity(const scheme::TermNode& node) {
		auto mesh_name = Access("mesh", node);
		auto material_name = Access("material", node);

		pMesh = platform::X::LoadMesh(mesh_name + ".asset", mesh_name);
		pMaterial = platform::X::LoadMaterial(material_name + ".mat.lsl", material_name);

		pShadingElement = leo::unique_raw(Environment->LeoEngine->CreateShadingElement(SED_Mesh));
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

	std::unique_ptr<ShadingElement> pShadingElement;
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
	LeoEngine::Core::Camera camera;
	std::unique_ptr<LeoEngine::Core::TrackballCameraManipulator> pCameraMainpulator;



	static_assert(sizeof(DirectLight) == sizeof(lm::float4) + sizeof(lm::float4) + sizeof(lm::float4) + 4);

	std::vector<DirectLight> lights;
	std::shared_ptr<GraphicsBuffer> pLightConstatnBuffer;
private:
	leo::uint32 DoUpdate(leo::uint32 pass) override {
		auto& timer = platform::chrono::FetchGlobalTimer();
		timer.UpdateOnFrameStart();
		platform::Material::GetInstanceEvaluator().Define("time", timer.GetFrameTime(), true);

		auto passInfo = GraphicsPassInfo::CreateGeneralPassGraphicsInfo(camera);

		auto entityId = ecs::EntitySystem::Instance().AddEntity<ecs::Entity>();

		Context::Instance().BeginFrame();
		Context::Instance().GetScreenFrame()->Clear(FrameBuffer::Color | FrameBuffer::Depth | FrameBuffer::Stencil, { 0,0,0,1 }, 1, 0);
		auto& Device = Context::Instance().GetDevice();




		ecs::EntitySystem::Instance().RemoveEntity(entityId);

		lm::float4x4 worldmatrix = {
			{1,0,0,0},
			{0,1,0,0},
			{0,0,1,0},
			{0,0,0,1}
		};
		auto projmatrix = LeoEngine::X::perspective_fov_lh(3.14f / 6, 600.0f / 800, 1, 1000);
		auto viewmatrix = camera.GetViewMatrix();

		auto worldview = worldmatrix * viewmatrix;
		auto worldviewproj = worldview * projmatrix;
		auto worldviewinv = lm::inverse(worldview);

		DirectLight& point_light = lights[0];
		point_light.position = transformpoint(lm::float3(0, 40, 0), viewmatrix);
		pLightConstatnBuffer->UpdateSubresource(0, static_cast<leo::uint32>(sizeof(DirectLight)*lights.size()),&lights[0]);

		auto pEffect = platform::X::LoadEffect("ForwardDirectLightShading");

		using namespace std::literals;
		//obj
		pEffect->GetParameter("worldview"sv) = lm::transpose(worldview);
		pEffect->GetParameter("worldviewproj"sv) = lm::transpose(worldviewproj);
		pEffect->GetParameter("worldviewinvt"sv) = worldviewinv;

		//light
		pEffect->GetParameter("light_count"sv) = static_cast<leo::uint32>(lights.size());
		pEffect->GetParameter("lights") = pLightConstatnBuffer;

		//mat
		pEffect->GetParameter("alpha"sv) = 1.0f;
		
		auto pView = passInfo.GetRenderView();
		 

		for (auto& entity : pEntities->GetRenderables())
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

		static auto pInitGuard = LeoEngine::System::InitGlobalEnvironment();

		pEntities = std::make_unique<Entities>("sponza_crytek.entities.lsl");

		leo::math::float3 eye{ -38.47f,3.53f,2.19f };
		leo::math::float3 up_vec{ 0.12f,0.98f,-0.08f };
		leo::math::float3 view_vec{ 0.94f,-0.1f,0.2f };

		camera.SetViewMatrix(LeoEngine::X::look_at_lh(eye, eye + view_vec * 10, up_vec));

		pCameraMainpulator = std::make_unique<LeoEngine::Core::TrackballCameraManipulator>(10.0f);
		pCameraMainpulator->Attach(camera);
		pCameraMainpulator->SetSpeed(0.005f, 0.1f);

		DirectLight point_light;
		point_light.type = POINT_LIGHT;
		point_light.position = lm::float3(0, 40, 0);
		point_light.range = 80;
		point_light.blub_innerangle = 40;
		point_light.color = lm::float3(1.0f, 1.0f, 1.0f);

		lights.push_back(point_light);

		auto& Device = Context::Instance().GetDevice();
		pLightConstatnBuffer = leo::share_raw(Device.CreateConstanBuffer(Buffer::Usage::Dynamic, EAccessHint::EA_GPURead | EAccessHint::EA_GPUStructured, sizeof(DirectLight)*lights.size(), static_cast<EFormat>(sizeof(DirectLight)),nullptr));

		GetMessageMap()[WM_MOUSEMOVE] += [&](::WPARAM wParam, ::LPARAM lParam) {
			static auto lastxPos = GET_X_LPARAM(lParam);
			static auto lastyPos = GET_Y_LPARAM(lParam);
			auto xPos = GET_X_LPARAM(lParam);
			auto yPos = GET_Y_LPARAM(lParam);
			leo::math::float2 offset(static_cast<float>(xPos - lastxPos), static_cast<float>(yPos - lastyPos));
			lastxPos = xPos;
			lastyPos = yPos;
			if (wParam & MK_LBUTTON) {
				pCameraMainpulator->Rotate(offset);
			}
			if (wParam & MK_MBUTTON) {
				pCameraMainpulator->Move(offset);
			}
			if (wParam & MK_RBUTTON) {
				pCameraMainpulator->Zoom(offset);
			}
		};
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



