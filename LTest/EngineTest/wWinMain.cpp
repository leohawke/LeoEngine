#include "../../Engine/test.h"

#include "../../Engine/Render/IContext.h"
#include "TestFramework.h"
#include "LSchemEngineUnitTest.h"

#define TEST_CODE 1

using namespace platform::Render;

class EngineTest : public Test::TestFrameWork {
public:
	using base = Test::TestFrameWork;
	using base::base;
private:
	leo::uint32 DoUpdate(leo::uint32 pass) override {
		unit_test::ExceuteLSchemEngineUnitTest();
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



