#include "../../Engine/test.h"
#include "../../Engine/Asset/MeshX.h"
#include "../../Engine/Render/IContext.h"
#include "../../Engine/Core/Mesh.h"
#include "../../Engine/Render/IFrameBuffer.h"
#include "../../Engine/Render/DataStructures.h"
#include "../../Engine/Renderer/PostProcess/PostProcessCombineLUTs.h"
#include "../../Engine/Asset/EffectX.h"
#include "../../Engine/Asset/MaterialX.h"
#include "../../Engine/Asset/LSLAssetX.h"
#include "../../Engine/Asset/RayTracingX.h"
#include "../../Engine/System/NinthTimer.h"
#include "../../Engine/Core/CameraController.h"
#include "../../Engine/System/SystemEnvironment.h"
#include "TestFramework.h"
#include "EntityComponentSystem/EntitySystem.h"
#include "LSchemEngineUnitTest.h"
#include "../../Engine/Core/Camera.h"
#include "../../Engine/Render/IRayTracingScene.h"
#include "../../Engine/Render/IRayDevice.h"
#include "../../Engine/Render/IRayContext.h"
#include <LFramework/LCLib/Logger.h>
#include <LFramework/LCLib/Debug.h>

#include <LScheme/LScheme.h>
#include <windowsx.h>

#define TEST_CODE 1

#include <filesystem>

using namespace platform::Render;
using namespace LeoEngine::Render;
namespace fs = std::filesystem;

namespace lm = leo::math;

class Entity {
public:
	Entity(const scheme::TermNode& node) {
		auto mesh_name = Access("mesh", node);
		auto material_name = Access("material", node);

		pMesh = platform::X::LoadMesh(mesh_name + ".asset", mesh_name);
		pMaterial = platform::X::LoadMaterial(material_name + ".mat.lsl", material_name);
	}

	const platform::Material& GetMaterial() const {
		return *pMaterial;
	}

	platform::Mesh& GetMesh()  {
		return *pMesh;
	}

	const platform::Mesh& GetMesh() const{
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

		min = leo::math::float3(FLT_MAX, FLT_MAX, FLT_MAX);
		max = leo::math::float3(FLT_MIN, FLT_MIN, FLT_MIN);
		for (auto& entity : entities)
		{
			min = leo::math::min(min, entity.GetMesh().GetBoundingMin());
			max = leo::math::max(max, entity.GetMesh().GetBoundingMax());
		}
	}

	const std::vector<Entity>& GetRenderables() const {
		return entities;
	}

	leo::unique_ptr<platform::Render::RayTracingScene> BuildRayTracingScene()
	{
		platform::Render::RayTracingSceneInitializer initializer;

		std::vector<RayTracingGeometryInstance> Instances;

		for (auto& entity : entities)
		{
			RayTracingGeometryInstance Instance;
			Instance.Geometry = entity.GetMesh().GetRayTracingGeometry();

			Instance.Transform = leo::math::float4x4::identity;

			Instances.push_back(Instance);
		}

		initializer.Instances = leo::make_span(Instances);

		return leo::unique_raw(Context::Instance().GetRayContext().GetDevice().CreateRayTracingScene(initializer));
	}

	leo::math::float3 min;
	leo::math::float3 max;
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
	std::shared_ptr<GraphicsBuffer> pGenShaderConstants;

	std::shared_ptr<Texture2D> ShadowMap;
	std::shared_ptr<UnorderedAccessView> ShadowMapUAV;
private:
	leo::uint32 DoUpdate(leo::uint32 pass) override {
		auto& timer = platform::chrono::FetchGlobalTimer();
		timer.UpdateOnFrameStart();
		platform::Material::GetInstanceEvaluator().Define("time", timer.GetFrameTime(), true);

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
		auto projmatrix = LeoEngine::X::perspective_fov_lh(3.14f / 4, 720.f / 1280, 1, 1000);
		auto viewmatrix = camera.GetViewMatrix();

		auto viewproj = viewmatrix * projmatrix;

		auto pEffect = platform::X::LoadEffect("ForwardDirectLightShading");

		using namespace std::literals;
		//obj
		pEffect->GetParameter("world"sv) = lm::transpose(worldmatrix);
		//camera
		pEffect->GetParameter("camera_pos"sv) = camera.GetEyePos();
		pEffect->GetParameter("viewproj"sv) = lm::transpose(viewproj);

		//light
		pEffect->GetParameter("light_count"sv) = static_cast<leo::uint32>(lights.size());
		pEffect->GetParameter("inv_sscreen"sv) = lm::float2(1 / 1280.f, 1 / 720.f);

		pEffect->GetParameter("lights") = pLightConstatnBuffer;

		//mat
		pEffect->GetParameter("alpha"sv) = 1.0f;

		//light_ext
		pEffect->GetParameter("ambient_color") = leo::math::float3(0.1f, 0.1f, 0.1f);
		
		auto pPreZEffect = platform::X::LoadEffect("PreZ");
		{
			//obj
			pPreZEffect->GetParameter("world"sv) = lm::transpose(worldmatrix);
			//camera
			pPreZEffect->GetParameter("camera_pos"sv) = camera.GetEyePos();
			pPreZEffect->GetParameter("viewproj"sv) = lm::transpose(viewproj);
		}

		//pre-z
		for (auto& entity : pEntities->GetRenderables())
		{
			Context::Instance().Render(*pPreZEffect, pPreZEffect->GetTechniqueByIndex(0), entity.GetMesh().GetInputLayout());
		}

		//ray-shadow
		//unbind
		Context::Instance().SetFrame(nullptr);

		platform::Render::GenShadowConstants shadowconstant;
		{
			shadowconstant.LightDirection = lights[0].direction;
			shadowconstant.CameraToWorld = lm::transpose(lm::inverse(viewproj));
			shadowconstant.Resolution = lm::float2(1280, 720);
			pGenShaderConstants->UpdateSubresource(0, static_cast<leo::uint32>(sizeof(shadowconstant)), &shadowconstant);
		}
		auto Scene = pEntities->BuildRayTracingScene();
		Context::Instance().GetRayContext().GetDevice().BuildAccelerationStructure(Scene.get());

		Context::Instance().GetRayContext().RayTraceShadow(Scene.get(),
			Context::Instance().GetScreenFrame().get(),
			ShadowMapUAV.get(),
			pGenShaderConstants.get());

		pEffect->GetParameter("shadow_tex") = TextureSubresource(ShadowMap, 0, ShadowMap->GetArraySize(), 0, ShadowMap->GetNumMipMaps());

		//re-bind
		Context::Instance().SetFrame(Context::Instance().GetScreenFrame());
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

		platform::ColorCorrectParameters params;
		auto pTex = platform::CombineLUTPass(params);

		pEntities = std::make_unique<Entities>("sponza_crytek.entities.lsl");

		float modelRaidus = leo::math::length(pEntities->max - pEntities->min) * .5f;

		auto  eye = (pEntities->max + pEntities->min) *.5f;

		eye = eye + leo::math::float3(modelRaidus * .51f, 1.2f, 0.f);

		leo::math::float3 up_vec{ 0,1.f,0 };
		leo::math::float3 view_vec{ 0.94f,-0.1f,0.2f };

		camera.SetViewMatrix(LeoEngine::X::look_at_lh(eye,leo::math::float3(0,0,0), up_vec));

		pCameraMainpulator = std::make_unique<LeoEngine::Core::TrackballCameraManipulator>(10.0f);
		pCameraMainpulator->Attach(camera);
		pCameraMainpulator->SetSpeed(0.005f, 0.1f);

		/*DirectLight point_light;
		point_light.type = POINT_LIGHT;
		point_light.position = lm::float3(0, 40, 0);
		point_light.range = 80;
		point_light.blub_innerangle = 40;
		point_light.color = lm::float3(1.0f, 1.0f, 1.0f);*/
		//lights.push_back(point_light);

		DirectLight directioal_light;
		directioal_light.type = DIRECTIONAL_LIGHT;
		directioal_light.direction = lm::float3(0.335837096f,0.923879147f,-0.183468640f);
		directioal_light.color = lm::float3(4.0f, 4.0f, 4.0f);
		lights.push_back(directioal_light);

		auto& Device = Context::Instance().GetDevice();
		pLightConstatnBuffer = leo::share_raw(Device.CreateConstanBuffer(Buffer::Usage::Dynamic, EAccessHint::EA_GPURead | EAccessHint::EA_GPUStructured, sizeof(DirectLight)*lights.size(), static_cast<EFormat>(sizeof(DirectLight)),lights.data()));

		pGenShaderConstants = leo::share_raw(Device.CreateConstanBuffer(platform::Render::Buffer::Usage::Dynamic, 0, sizeof(platform::Render::GenShadowConstants), EFormat::EF_Unknown));

		ShadowMap = leo::share_raw(Device.CreateTexture(1280, 720, 1, 1, EFormat::EF_R32F,EA_GPURead | EA_GPUWrite | EA_GPUUnordered, {}));

		ShadowMapUAV = leo::share_raw(Device.CreateUnorderedAccessView(ShadowMap.get()));

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
	leo::FetchCommonLogger().SetSender([](platform::Descriptions::RecordLevel lv, platform::Logger& logger, const char* str) {return
		platform_ex::SendDebugString(lv,str); });

	EngineTest Test(L"EnginetTest");
	Test.Create();
	Test.Run();

	return 0;
}



