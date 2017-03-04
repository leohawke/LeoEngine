#include "LSchemEngineUnitTest.h"
#include <LScheme/Configuration.h>

#include "../../Engine/Asset/EffectX.h"
#include "../../Engine/Render/Effect/Effect.hpp"
#include "../../Engine/Render/IContext.h"

#include <sstream>
#include <LBase/Debug.h>

using namespace platform::Descriptions;

void unit_test::ExceuteLSchemEngineUnitTest()
{
	{
		auto effect_asset = platform::X::LoadEffectAsset("Bilt.lsl");
		auto shader = effect_asset.GenHLSLShader();
		{
			std::ofstream fout("Bilt.lsl.hlsl");
			fout << shader;
		}
		platform::Render::Effect::Effect blit{ "Bilt" };
	}
	using namespace platform::Render;

	auto& Device = Context::Instance().GetDevice();
	auto pTex = leo::unique_raw(Device.CreateTexture(512,1, 1, EFormat::EF_ABGR8, EAccessHint::EA_GenMips & EAccessHint::EA_GPURead, {}));
	pTex->BuildMipSubLevels();
}



