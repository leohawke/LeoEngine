#include "LSchemEngineUnitTest.h"
#include <LScheme/Configuration.h>

#include "../../Engine/Asset/EffectX.h"
#include "../../Engine/Render/Effect/Effect.hpp"
#include "../../Engine/Render/IContext.h"

#include <sstream>
#include <LFramework/LCLib/Debug.h>

using namespace platform::Descriptions;

void unit_test::ExceuteLSchemEngineUnitTest()
{
	using namespace platform::Render;

	auto& Device = Context::Instance().GetDevice();
	auto pTex = leo::unique_raw(Device.CreateTexture(512,0, 1, EFormat::EF_ABGR8, EAccessHint::EA_GenMips | EAccessHint::EA_GPUWrite | EAccessHint::EA_GPURead, {}));
	pTex->BuildMipSubLevels();
}



