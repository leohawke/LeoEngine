#include "../../Engine/test.h"
#include "../../Engine/Asset/MeshX.h"
#include "../../Engine/Render/IContext.h"
#include "../../Engine/Core/Mesh.h"
#include "TestFramework.h"
#include "EntityComponentSystem/EntitySystem.h"
#include "LSchemEngineUnitTest.h"

#define TEST_CODE 1

using namespace platform::Render;

class EngineTest : public Test::TestFrameWork {
public:
	using base = Test::TestFrameWork;
	using base::base;
private:
	leo::uint32 DoUpdate(leo::uint32 pass) override {
		auto entityId = ecs::EntitySystem::Instance().AddEntity<ecs::Entity>();

		Context::Instance().BeginFrame();
		auto& Device = Context::Instance().GetDevice();
		auto pTex = leo::unique_raw(Device.CreateTexture(512, 0, 1, EFormat::EF_ABGR8, EAccessHint::EA_GenMips | EAccessHint::EA_GPUWrite | EAccessHint::EA_GPURead, {}));
		pTex->BuildMipSubLevels();
		Context::Instance().EndFrame();

		ecs::EntitySystem::Instance().RemoveEntity(entityId);

		auto pMesh = std::make_unique<platform::Mesh>(platform::X::LoadMeshAsset("Broadleaf_Desktop_LOD0.asset"),"Broadleaf_Desktop_LOD0");
		auto pEffect = std::make_unique<platform::Render::Effect::Effect>("ForwardPointLightDiffuseShading");
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



