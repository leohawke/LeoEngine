#include "TestFramework.h"
#include "LFramework/Helper/Initialization.H"

Test::TestFrameWork::TestFrameWork(const std::string_view & name)
{
	auto init_node = Test::FetchRoot()["LTest"]["Init"];
}

Test::TestFrameWork::~TestFrameWork()
{
}

void Test::TestFrameWork::Create()
{
}

void Test::TestFrameWork::Run()
{
}

leo::uint32 Test::TestFrameWork::Update(leo::uint32 pass)
{
	return leo::uint32();
}

void Test::TestFrameWork::OnCreate()
{
}

void Test::TestFrameWork::OnDestroy()
{
}
