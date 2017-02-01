#include "LSchemEngineUnitTest.h"
#include <LScheme/Configuration.h>

#include "../../Engine/Asset/EffectX.h"
#include "../../Engine/Render/Effect/Effect.hpp"

#include <sstream>
#include <LBase/Debug.h>

using namespace platform::Descriptions;

void unit_test::ExceuteLSchemEngineUnitTest()
{
	auto effect_asset = platform::X::LoadEffectAsset("Bilt.lsl");
	auto shader = effect_asset.GenHLSLShader();
	{
		std::ofstream fout("Bilt.lsl.hlsl");
		fout << shader;
	}
	platform::Render::Effect::Effect blit{ "Bilt" };
}



