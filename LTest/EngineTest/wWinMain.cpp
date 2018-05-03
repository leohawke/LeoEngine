#include "../../Engine/test.h"
#include "../../Engine/Asset/MeshX.h"
#include "../../Engine/Render/IContext.h"
#include "../../Engine/Core/Mesh.h"
#include "../../Engine/Render/IFrameBuffer.h"
#include "TestFramework.h"
#include "EntityComponentSystem/EntitySystem.h"
#include "LSchemEngineUnitTest.h"
#include "../../Engine/Core/Camera.h"
#define TEST_CODE 1

using namespace platform::Render;

namespace lm = leo::math;

class EngineTest : public Test::TestFrameWork {
public:
	using base = Test::TestFrameWork;
	using base::base;
private:
	leo::uint32 DoUpdate(leo::uint32 pass) override {
		auto entityId = ecs::EntitySystem::Instance().AddEntity<ecs::Entity>();

		Context::Instance().BeginFrame();
		Context::Instance().GetScreenFrame()->Clear(FrameBuffer::Color | FrameBuffer::Depth | FrameBuffer::Stencil, { 0,0,0,1 }, 1,0);
		auto& Device = Context::Instance().GetDevice();
		//auto pTex = leo::unique_raw(Device.CreateTexture(512, 0, 1, EFormat::EF_ABGR8, EAccessHint::EA_GenMips | EAccessHint::EA_GPUWrite | EAccessHint::EA_GPURead, {}));

		ecs::EntitySystem::Instance().RemoveEntity(entityId);

		auto pMesh = std::make_unique<platform::Mesh>(*platform::X::LoadMeshAsset("Broadleaf_Desktop_LOD0.asset"),"Broadleaf_Desktop_LOD0");
		auto pEffect = std::make_unique<platform::Render::Effect::Effect>("ForwardPointLightDiffuseShading");

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
		//mat
		pEffect->GetParameter("albedo"sv) = lm::float3(0.8f, 0.6f, 0.4f);
		pEffect->GetParameter("metalness"sv) = 0.6f;
		pEffect->GetParameter("specular"sv) = lm::float3(1.0f, 0.2f, 0.1f);
		pEffect->GetParameter("alpha"sv) = 1.0f;
		pEffect->GetParameter("smoothness"sv) =0.8f;
		//light
		pEffect->GetParameter("view_light_pos"sv) = lm::float3(0, 0,0);
		pEffect->GetParameter("light_radius"sv) =20.f;
		pEffect->GetParameter("light_color"sv) = lm::float3(0.8f, 0.8f, 0.6f);
		pEffect->GetParameter("light_blubsize"sv) = 10.f;

		Context::Instance().Render(*pEffect, pEffect->GetTechniqueByIndex(0), pMesh->GetInputLayout());

		Context::Instance().GetDisplay().SwapBuffers();
		//what can i do in this duration?
		Context::Instance().GetDisplay().WaitOnSwapBuffers();

		Context::Instance().EndFrame();

		return Nothing;
	}
	void OnCreate() override {
		auto swap_chain = ::Create(GetNativeHandle());
		Context::Instance().CreateDeviceAndDisplay();
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



