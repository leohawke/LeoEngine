
#define TEST_CODE 1

#include "EntityComponentSystem/EntitySystem.h"

#include "test.h"
#include "Asset/EffectX.h"
#include "System/NinthTimer.h"
#include "Core/CameraController.h"
#include "Core/Camera.h"
#include "System/SystemEnvironment.h"
#include "Renderer/imgui/imgui_context.h"
#include "Render/ICommandList.h"
#include "Render/DataStructures.h"
#include "Render/IFrameBuffer.h"
#include "Render/DrawEvent.h"
#include "Render/PipelineStateUtility.h"
#include "Renderer/PostProcess/PostProcessCombineLUTs.h"
#include "Renderer/PostProcess/PostProcessToneMap.h"
#include "Renderer/ScreenSpaceDenoiser.h"
#include "Renderer/ShadowRendering.h"
#include "Runtime/DirectionalLight.h"
#include "Math/RotationMatrix.h"
#include "Math/ScaleMatrix.h"
#include "Math/ShadowProjectionMatrix.h"
#include "Math/TranslationMatrix.h"

#include "LFramework/Win32/LCLib/Mingw32.h"
#include "LFramework/Helper/ShellHelper.h"

#include "TestFramework.h"
#include "Entities.h"
#include "LSchemEngineUnitTest.h"
#include "imgui/imgui_impl_win32.h"

#include <windowsx.h>


using namespace platform::Render;
using namespace LeoEngine::Render;
using namespace platform_ex::Windows;

namespace lm = leo::math;
namespace le = LeoEngine;

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

	LeoEngine::DirectionalLight sun_light;
	le::SceneInfo scene;


	std::shared_ptr<Texture2D> RayShadowMask;
	std::shared_ptr<UnorderedAccessView> RayShadowMaskUAV;

	std::shared_ptr<Texture2D> RayShadowMaskDenoiser;
	std::shared_ptr<UnorderedAccessView> RayShadowMaskDenoiserUAV;

	std::shared_ptr<Texture2D> ShadowMap;


	std::shared_ptr<Texture2D> HDROutput;
	std::shared_ptr<Texture2D> NormalOutput;


	leo::math::float4 clear_color = { 0,0,0,0 };

	platform::CombineLUTSettings lut_params;
	bool lut_dirty = false;
	TexturePtr lut_texture = nullptr;

	float LightHalfAngle = 0.5f;
	int SamplesPerPixel = 1;
	unsigned StateFrameIndex = 0;
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
		OnRaytracingUI();
		OnCombineLUTUI();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	leo::uint32 DoUpdate(leo::uint32 pass) override {
		auto& timer = platform::chrono::FetchGlobalTimer();
		timer.UpdateOnFrameStart();
		platform::Material::GetInstanceEvaluator().Define("time", timer.GetFrameTime(), true);

		auto entityId = ecs::EntitySystem::Instance().AddEntity<ecs::Entity>();

		auto& CmdList = platform::Render::GetCommandList();

		SCOPED_GPU_EVENT(CmdList, Frame);


		CmdList.BeginFrame();
		Context::Instance().BeginFrame();
		auto& screen_frame = Context::Instance().GetScreenFrame();
		auto screen_tex = screen_frame->Attached(FrameBuffer::Target0);
		auto depth_tex = screen_frame->Attached(FrameBuffer::DepthStencil);

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
		auto pPreZEffect = platform::X::LoadEffect("PreZ");

		using namespace std::literals;

		//viewinfo materialinfo
		{
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

			{
				//obj
				pPreZEffect->GetParameter("world"sv) = lm::transpose(worldmatrix);
				//camera
				pPreZEffect->GetParameter("camera_pos"sv) = camera.GetEyePos();
				pPreZEffect->GetParameter("viewproj"sv) = lm::transpose(viewproj);
			}
		}

		{
			SCOPED_GPU_EVENT(CmdList, PreZ);
			platform::Render::RenderPassInfo prezPass(
				NormalOutput.get(), platform::Render::RenderTargetActions::Clear_Store, 
				depth_tex, platform::Render::DepthStencilTargetActions::ClearDepthStencil_StoreDepthStencil);
			CmdList.BeginRenderPass(prezPass, "PreZ");

			//pre-z
			for (auto& entity : pEntities->GetRenderables())
			{
				//copy normal map params
				entity.GetMaterial().UpdateParams(reinterpret_cast<const platform::Renderable*>(&entity), pPreZEffect.get());

				Context::Instance().Render(CmdList, *pPreZEffect, pPreZEffect->GetTechniqueByIndex(0), entity.GetMesh().GetInputLayout());
			}
		}

		OnDrawLights(viewmatrix,projmatrix);

		if(RayShadowMaskDenoiser)
			pEffect->GetParameter("shadow_tex") = TextureSubresource(RayShadowMaskDenoiser, 0, RayShadowMask->GetArraySize(), 0, RayShadowMaskDenoiser->GetNumMipMaps());

		{
			SCOPED_GPU_EVENT(CmdList, GeometryShading);
			platform::Render::RenderPassInfo GeometryPass(
				HDROutput.get(), platform::Render::RenderTargetActions::Clear_Store,
				depth_tex, platform::Render::DepthStencilTargetActions::LoadDepthStencil_StoreDepthStencil);
			CmdList.BeginRenderPass(GeometryPass, "Geometry");

			for (auto& entity : pEntities->GetRenderables())
			{
				entity.GetMaterial().UpdateParams(reinterpret_cast<const platform::Renderable*>(&entity));
				Context::Instance().Render(CmdList, *pEffect, pEffect->GetTechniqueByIndex(0), entity.GetMesh().GetInputLayout());
			}
		}

		OnPostProcess();
		OnDrawUI();

		Context::Instance().EndFrame();
		CmdList.EndFrame();
		
		Context::Instance().GetDisplay().SwapBuffers();
		//what can i do in this duration?
		Context::Instance().GetDisplay().WaitOnSwapBuffers();

		++StateFrameIndex;

		return Nothing;
	}

	Texture* GetScreenTex()
	{
		return Context::Instance().GetScreenFrame()->Attached(FrameBuffer::Target0);
	}

	void RenderShadowDepth()
	{
		le::ProjectedShadowInitializer initializer;
		sun_light.GetProjectedShadowInitializer(scene,initializer);

		const auto WorldToLightScaled = initializer.WorldToLight * le::ScaleMatrix(initializer.Scales);

		const auto MaxSubjectZ = lm::transformpoint(initializer.SubjectBounds.Origin, WorldToLightScaled).z + initializer.SubjectBounds.Radius;

		const auto MinSubjectZ = (MaxSubjectZ - initializer.SubjectBounds.Radius * 2);

		const auto SubjectAndReceiverMatrix = WorldToLightScaled * le::ShadowProjectionMatrix(MinSubjectZ, MaxSubjectZ, initializer.WAxis);

		const auto ProjectionMatrix = le::TranslationMatrix(initializer.ShadowTranslation) * SubjectAndReceiverMatrix;

		le::ProjectedShadowInfo ProjectedShadowInfo;

		platform::Render::RenderPassInfo shadowDepth(
			ShadowMap.get(), platform::Render::DepthStencilTargetActions::ClearDepthStencil_StoreDepthNotStencil);

		ProjectedShadowInfo.PassInfo = &shadowDepth;

		ProjectedShadowInfo.PreShadowTranslation = initializer.ShadowTranslation;
		ProjectedShadowInfo.SubjectAndReceiverMatrix = SubjectAndReceiverMatrix;

		ProjectedShadowInfo.X = 0;
		ProjectedShadowInfo.Y = 0;
		ProjectedShadowInfo.ResolutionX = ShadowMap->GetWidth(0);
		ProjectedShadowInfo.ResolutionY = ShadowMap->GetHeight(0);

		ProjectedShadowInfo.BorderSize = 0;

		auto& CmdList = platform::Render::GetCommandList();

		SCOPED_GPU_EVENT(CmdList, RenderShadowDepth);

		auto psoInit = le::SetupShadowDepthPass(ProjectedShadowInfo, CmdList);

		for (auto& entity : pEntities->GetRenderables())
		{
			auto& layout = entity.GetMesh().GetInputLayout();

			DrawInputLayout(CmdList, psoInit, layout);
		}
	}

	void DrawInputLayout(platform::Render::CommandList& CmdList, platform::Render::GraphicsPipelineStateInitializer psoInit, const platform::Render::InputLayout& layout)
	{
		psoInit.ShaderPass.VertexDeclaration = layout.GetVertexDeclaration();
		psoInit.Primitive = layout.GetTopoType();

		platform::Render::SetGraphicsPipelineState(CmdList, psoInit);

		auto num_vertex_streams = layout.GetVertexStreamsSize();
		for (auto i = 0; i != num_vertex_streams; ++i) {
			auto& stream = layout.GetVertexStream(i);
			auto& vb = static_cast<GraphicsBuffer&>(*stream.stream);
			CmdList.SetVertexBuffer(i, &vb);
		}

		auto index_stream = layout.GetIndexStream();
		auto vertex_count = index_stream ? layout.GetNumIndices() : layout.GetNumVertices();
		auto prim_count = vertex_count;
		switch (psoInit.Primitive)
		{
		case platform::Render::PrimtivteType::LineList:
			prim_count /= 2;
			break;
		case platform::Render::PrimtivteType::TriangleList:
			prim_count /= 3;
			break;
		case platform::Render::PrimtivteType::TriangleStrip:
			prim_count -= 2;
			break;
		}

		auto num_instances = layout.GetVertexStream(0).instance_freq;
		if (index_stream) {
			auto num_indices = layout.GetNumIndices();
			CmdList.DrawIndexedPrimitive(index_stream.get(),
				layout.GetVertexStart(), 0, layout.GetNumVertices(),
				layout.GetIndexStart(), prim_count, num_instances
			);
		}
		else {
			auto num_vertices = layout.GetNumVertices();
			CmdList.DrawPrimitive(layout.GetVertexStart(), 0, prim_count, num_instances);
		}
	}

	void OnPostProcess()
	{
		auto& CmdList = platform::Render::GetCommandList(); 
		SCOPED_GPU_EVENT(CmdList, PostProcess);

		//PostProcess
		if (/*true ||*/ lut_dirty || !lut_texture)
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

	void OnDrawLights(const leo::math::float4x4& viewmatrix,const leo::math::float4x4& projmatrix)
	{
		auto& screen_frame = Context::Instance().GetScreenFrame();
		auto screen_tex = screen_frame->Attached(FrameBuffer::Target0);
		auto depth_tex =static_cast<Texture2D*>(screen_frame->Attached(FrameBuffer::DepthStencil));

		auto width = depth_tex->GetWidth(0);
		auto height = depth_tex->GetHeight(0);

		auto viewproj = viewmatrix * projmatrix;

		auto invproj = LeoEngine::X::InvertProjectionMatrix(projmatrix);
		auto invview = leo::math::inverse(viewmatrix);

		auto invviewproj = invproj * invview;

		auto ViewSizeAndInvSize = leo::math::float4(width, height, 1.0f / width, 1.0f / height);
		// setup a matrix to transform float4(SvPosition.xyz,1) directly to World (quality, performance as we don't need to convert or use interpolator)

		float Mx = 2.0f * ViewSizeAndInvSize.z;
		float My = -2.0f * ViewSizeAndInvSize.w;
		float Ax = -1.0f;
		float Ay = 1.0f;

		leo::math::float4x4 SVPositionToWorld = leo::math::float4x4(
			leo::math::float4(Mx, 0, 0, 0),
			leo::math::float4(0, My, 0, 0),
			leo::math::float4(0, 0, 1, 0),
			leo::math::float4(Ax, Ay, 0, 1)
		) * invviewproj;

		platform::Render::ShadowRGParameters shadowconstant{
			.SVPositionToWorld = leo::math::transpose(SVPositionToWorld),
			.WorldCameraOrigin = camera.GetEyePos(),
			.BufferSizeAndInvSize = leo::math::float4(width,height,1.0f/width,1.0f/height),
			.NormalBias = 0.1,
			.LightDirection = lights[0].direction,
			.SourceRadius = sinf(LightHalfAngle * 3.14159265f / 180),
			.SamplesPerPixel =static_cast<unsigned>(SamplesPerPixel),
			.StateFrameIndex = StateFrameIndex,
			.WorldNormalBuffer = NormalOutput.get(),
			.Depth = depth_tex,
			.Output = RayShadowMaskUAV.get()
		};
		
		auto Scene = pEntities->BuildRayTracingScene();
		Context::Instance().GetRayContext().GetDevice().BuildAccelerationStructure(Scene.get());

		auto& CmdList = platform::Render::GetCommandList();

		SCOPED_GPU_EVENT(CmdList, DrawLights);

		

		//clear rt?
		Context::Instance().GetRayContext().RayTraceShadow(Scene.get(),
			shadowconstant);


		platform::ScreenSpaceDenoiser::ShadowVisibilityInput svinput =
		{
			.Mask = RayShadowMask.get(),
			.SceneDepth = depth_tex,
			.WorldNormal = NormalOutput.get(),
			.LightHalfRadians = LightHalfAngle * 3.14159265f / 180,
		};

		platform::ScreenSpaceDenoiser::ShadowVisibilityOutput svoutput =
		{
			.Mask = RayShadowMaskDenoiser.get(),
			.MaskUAV = RayShadowMaskDenoiserUAV.get()
		};

		platform::ScreenSpaceDenoiser::ShadowViewInfo viewinfo =
		{
			.StateFrameIndex = StateFrameIndex,
			.ScreenToTranslatedWorld = leo::math::float4x4(
			leo::math::float4(1, 0, 0, 0),
			leo::math::float4(0, 1, 0, 0),
			leo::math::float4(0, 0, projmatrix[2][2], 1),
			leo::math::float4(0, 0, projmatrix[3][2], 0)
			) * invviewproj,
			.ViewToClip = projmatrix,
			.TranslatedWorldToView = viewmatrix,
			.InvProjectionMatrix = invproj,
			.InvDeviceZToWorldZTransform = LeoEngine::X::CreateInvDeviceZToWorldZTransform(projmatrix),
		};

		{
			SCOPED_GPU_EVENT(CmdList, ShadowDenoise);

			platform::ScreenSpaceDenoiser::DenoiseShadowVisibilityMasks(
				CmdList,
				viewinfo,
				svinput,
				svoutput
			);
		}

		RenderShadowDepth();
	}

	void OnDrawUI()
	{
		auto& CmdList = platform::Render::GetCommandList();

		platform::Render::RenderPassInfo passInfo(GetScreenTex(), RenderTargetActions::Load_Store);

		SCOPED_GPU_EVENT(CmdList, Imgui);
		CmdList.BeginRenderPass(passInfo, "imguiPass");

		platform::imgui::Context_RenderDrawData(ImGui::GetDrawData());
	}

	void OnCreate() override {
		LFL_DEBUG_DECL_TIMER(creater_timer,__FUNCTION__);

		auto swap_chain = ::Create(GetNativeHandle());

		platform::Render::DisplaySetting display_setting;

		Context::Instance().CreateDeviceAndDisplay(display_setting);

		static auto pInitGuard = LeoEngine::System::InitGlobalEnvironment();

		
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		
		ImGui_ImplWin32_Init(GetNativeHandle());

		platform::imgui::Context_Init(Context::Instance());

		auto& Device = Context::Instance().GetDevice();

		Context::Instance().BeginFrame();

		ElementInitData data;
		data.clear_value = &ClearValueBinding::Black;

		HDROutput = leo::share_raw(Device.CreateTexture(1280, 720, 1, 1, EFormat::EF_ABGR16F, EA_GPURead | EA_RTV, {}, &data));
		NormalOutput = leo::share_raw(Device.CreateTexture(1280, 720, 1, 1, EFormat::EF_ABGR16F, EA_GPURead | EA_RTV, {}, &data));

		pEntities = std::make_unique<Entities>("sponza_crytek.entities.lsl");

		scene.AABBMin = pEntities->min;
		scene.AABBMax = pEntities->max;

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
		directioal_light.color = lm::float3(1.0f, 1.0f, 1.0f);
		lights.push_back(directioal_light);

		sun_light.SetTransform(le::RotationMatrix(le::Rotator(directioal_light.direction)));


		pLightConstatnBuffer = leo::share_raw(Device.CreateConstanBuffer(Buffer::Usage::Dynamic, EAccessHint::EA_GPURead | EAccessHint::EA_GPUStructured, sizeof(DirectLight)*lights.size(), static_cast<EFormat>(sizeof(DirectLight)),lights.data()));

		RayShadowMask = leo::share_raw(Device.CreateTexture(1280, 720, 1, 1, EFormat::EF_ABGR16F, EA_GPURead | EA_GPUWrite | EA_GPUUnordered, {}));
		RayShadowMaskDenoiser = leo::share_raw(Device.CreateTexture(1280, 720, 1, 1, EFormat::EF_R32F,EA_GPURead | EA_GPUWrite | EA_GPUUnordered, {}));

		ShadowMap = leo::share_raw(Device.CreateTexture(2048, 2048, 1, 1, EFormat::EF_D32F, EA_GPURead | EA_DSV, {}));

		RayShadowMaskUAV = leo::share_raw(Device.CreateUnorderedAccessView(RayShadowMask.get()));
		RayShadowMaskDenoiserUAV = leo::share_raw(Device.CreateUnorderedAccessView(RayShadowMaskDenoiser.get()));

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

		Context::Instance().GetDisplay().SwapBuffers();
		//what can i do in this duration?
		Context::Instance().GetDisplay().WaitOnSwapBuffers();

		Context::Instance().EndFrame();

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

	void OnRaytracingUI()
	{
		if (ImGui::CollapsingHeader("Raytracing"))
		{
			ImGui::SliderFloat("SoftShadowAngle", &LightHalfAngle, 0.1f, 10);
			ImGui::SliderInt("RayPerPixel", &SamplesPerPixel, 1, 8);
		}
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



