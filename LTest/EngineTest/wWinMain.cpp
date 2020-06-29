#include "Engine/test.h"
#include "Engine/Asset/EffectX.h"
#define TEST_CODE 1

#include "Engine/System/NinthTimer.h"
#include "Engine/Core/CameraController.h"
#include "Engine/Core/Camera.h"
#include "Engine/System/SystemEnvironment.h"
#include "EntityComponentSystem/EntitySystem.h"
#include "Engine/Renderer/imgui/imgui_context.h"
#include "Engine/Render/ICommandList.h"
#include "Engine/Render/DataStructures.h"
#include "Engine/Render/IFrameBuffer.h"
#include "Engine/Renderer/PostProcess/PostProcessCombineLUTs.h"
#include "Engine/Renderer/PostProcess/PostProcessToneMap.h"

#include "LFramework/Win32/LCLib/Mingw32.h"

#include "TestFramework.h"
#include "Entities.h"
#include "LSchemEngineUnitTest.h"
#include "imgui/imgui_impl_win32.h"

#include <windowsx.h>


using namespace platform::Render;
using namespace LeoEngine::Render;
using namespace platform_ex::Windows;

namespace lm = leo::math;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



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

	std::shared_ptr<Texture2D> HDROutput;


	leo::math::float4 clear_color = { 0,0,0,0 };

	platform::CombineLUTSettings lut_params;
	bool lut_dirty = false;
	TexturePtr lut_texture = nullptr;
private:
	bool SubWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) override
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		ImGuiIO& io = ImGui::GetIO();

		if (io.WantCaptureMouse || io.WantCaptureKeyboard)
			return true;

		return false;
	}

	void OnGUI()
	{
		ImGui::Begin("Settings");
		OnCombineLUTUI();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	leo::uint32 DoUpdate(leo::uint32 pass) override {
		auto& timer = platform::chrono::FetchGlobalTimer();
		timer.UpdateOnFrameStart();
		platform::Material::GetInstanceEvaluator().Define("time", timer.GetFrameTime(), true);

		auto entityId = ecs::EntitySystem::Instance().AddEntity<ecs::Entity>();

		Context::Instance().BeginFrame();
		auto& screen_frame = Context::Instance().GetScreenFrame();
		auto screen_tex = screen_frame->Attached(FrameBuffer::Target0);

		auto& Device = Context::Instance().GetDevice();

		platform::imgui::Context_NewFrame();
		
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		OnGUI();
		ImGui::Render();

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

		Context::Instance().SetFrame(nullptr);

		screen_frame->Detach(FrameBuffer::Target0);
		screen_frame->Clear(FrameBuffer::Depth | FrameBuffer::Stencil, clear_color, 1, 0);
		Context::Instance().SetFrame(screen_frame);
		//pre-z
		for (auto& entity : pEntities->GetRenderables())
		{
			Context::Instance().Render(*pPreZEffect, pPreZEffect->GetTechniqueByIndex(0), entity.GetMesh().GetInputLayout());
		}

		//ray-shadow
		//unbind
		Context::Instance().SetFrame(nullptr);

		GenShadowConstants shadowconstant;
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

		RenderTarget hdrTarget;
		hdrTarget.Texture = HDROutput.get();
		screen_frame->Attach(FrameBuffer::Target0, hdrTarget);

		//re-bind
		Context::Instance().SetFrame(screen_frame);
		screen_frame->Clear(FrameBuffer::Color, clear_color, 1, 0);

		for (auto& entity : pEntities->GetRenderables())
		{
			entity.GetMaterial().UpdateParams(reinterpret_cast<const platform::Renderable*>(&entity));
			Context::Instance().Render(*pEffect, pEffect->GetTechniqueByIndex(0), entity.GetMesh().GetInputLayout());
		}

		RenderTarget prevTarget;
		prevTarget.Texture = screen_tex;
		screen_frame->Attach(FrameBuffer::Target0, prevTarget);

		OnPostProcess();
		OnDrawUI();
		
		Context::Instance().GetDisplay().SwapBuffers();
		//what can i do in this duration?
		Context::Instance().GetDisplay().WaitOnSwapBuffers();

		Context::Instance().EndFrame();

		return Nothing;
	}

	Texture* GetScreenTex()
	{
		return Context::Instance().GetScreenFrame()->Attached(FrameBuffer::Target0);
	}

	void OnPostProcess()
	{
		//PostProcess
		if (true || lut_dirty || !lut_texture)
		{
			lut_texture = platform::CombineLUTPass(lut_params);
		}

		platform::TonemapInputs tonemap_inputs;
		tonemap_inputs.OverrideOutput.Texture = GetScreenTex();
		tonemap_inputs.OverrideOutput.LoadAction = RenderTargetLoadAction::NoAction;
		tonemap_inputs.OverrideOutput.StoreAction = RenderTargetStoreAction::Store;
		tonemap_inputs.ColorGradingTexture = lut_texture;
		tonemap_inputs.SceneColor = HDROutput;

		platform::TonemapPass(tonemap_inputs);
	}

	void OnDrawUI()
	{
		auto& CmdList = platform::Render::GetCommandList();

		platform::Render::RenderPassInfo passInfo(GetScreenTex(), RenderTargetActions::Load_Store);

		CmdList.BeginRenderPass(passInfo, "imguiPass");

		platform::imgui::Context_RenderDrawData(ImGui::GetDrawData());
	}

	void OnCreate() override {
		auto swap_chain = ::Create(GetNativeHandle());
		Context::Instance().CreateDeviceAndDisplay();

		static auto pInitGuard = LeoEngine::System::InitGlobalEnvironment();

		
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		
		ImGui_ImplWin32_Init(GetNativeHandle());

		platform::imgui::Context_Init(Context::Instance());

		auto& Device = Context::Instance().GetDevice();

		ElementInitData data;
		data.clear_value = &ClearValueBinding::Black;

		HDROutput = leo::share_raw(Device.CreateTexture(1280, 720, 1, 1, EFormat::EF_ABGR16F, EA_GPURead | EA_RTV, {}, &data));

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

		pLightConstatnBuffer = leo::share_raw(Device.CreateConstanBuffer(Buffer::Usage::Dynamic, EAccessHint::EA_GPURead | EAccessHint::EA_GPUStructured, sizeof(DirectLight)*lights.size(), static_cast<EFormat>(sizeof(DirectLight)),lights.data()));

		pGenShaderConstants = leo::share_raw(Device.CreateConstanBuffer(Buffer::Usage::Dynamic, 0, sizeof(GenShadowConstants), EFormat::EF_Unknown));

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

	void OnCombineLUTUI()
	{
#define FLOAT4_FIELD(Field) lut_dirty|=ImGui::ColorEdit4(#Field, lut_params.Field.data)
#define FLOAT_FIELD(Field) lut_dirty|=ImGui::SliderFloat(#Field, &lut_params.Field,0,1)

		if (ImGui::CollapsingHeader("ColorCorrect"))
		{
			ImGui::Indent(16);
			FLOAT4_FIELD(ColorSaturation);
			FLOAT4_FIELD(ColorContrast);
			FLOAT4_FIELD(ColorGamma);
			FLOAT4_FIELD(ColorGain);
			FLOAT4_FIELD(ColorOffset);
			FLOAT4_FIELD(ColorSaturationShadows);
			FLOAT4_FIELD(ColorContrastShadows);
			FLOAT4_FIELD(ColorGammaShadows);
			FLOAT4_FIELD(ColorGainShadows);
			FLOAT4_FIELD(ColorOffsetShadows);
			FLOAT4_FIELD(ColorSaturationMidtones);
			FLOAT4_FIELD(ColorContrastMidtones);
			FLOAT4_FIELD(ColorGammaMidtones);
			FLOAT4_FIELD(ColorGainMidtones);
			FLOAT4_FIELD(ColorOffsetMidtones);
			FLOAT4_FIELD(ColorSaturationHighlights);
			FLOAT4_FIELD(ColorContrastHighlights);
			FLOAT4_FIELD(ColorGammaHighlights);
			FLOAT4_FIELD(ColorGainHighlights);
			FLOAT4_FIELD(ColorOffsetHighlights);
			FLOAT_FIELD(ColorCorrectionShadowsMax);
			FLOAT_FIELD(ColorCorrectionHighlightsMin);
			ImGui::Unindent(16);
		}

		if (ImGui::CollapsingHeader("WhiteBlance"))
		{
			ImGui::Indent(16);
			lut_dirty |= ImGui::SliderFloat("WhiteTemp", &lut_params.WhiteTemp, 2000, 15000);
			lut_dirty |= ImGui::SliderFloat("WhiteTint", &lut_params.WhiteTint, -1, 1);
			ImGui::Unindent(16);
		}

		if (ImGui::CollapsingHeader("ACES"))
		{
			ImGui::Indent(16);
			FLOAT_FIELD(FilmSlope);
			FLOAT_FIELD(FilmToe);
			FLOAT_FIELD(FilmShoulder);
			FLOAT_FIELD(FilmBlackClip);
			FLOAT_FIELD(FilmWhiteClip);
			ImGui::Unindent(16);
		}

		lut_dirty |= ImGui::SliderFloat("Gamma", &Environment->Gamma, 1, 2.5f);
		FLOAT_FIELD(BlueCorrection);
		FLOAT_FIELD(ExpandGamut);
#undef FLOAT4_FIELD
#undef FLOAT_FIELD
	}
};


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR cmdLine, int nCmdShow)
{
	leo::FetchCommonLogger().SetSender([](platform::Descriptions::RecordLevel lv, platform::Logger& logger, const char* str) {return
		platform_ex::SendDebugString(lv,str); });

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	EngineTest Test(L"EnginetTest");
	Test.Create();
	Test.Run();

	return 0;
}



