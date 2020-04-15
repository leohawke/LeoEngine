#include "BuiltInRayTracingShaders.h"
#include "RayTracingShader.h"
#include "../../Asset/ShaderAsset.h"
#include "../../Asset/D3DShaderCompiler.h"
#include "Context.h"
#include "RayDevice.h"

using namespace platform_ex::Windows::D3D12;


IMPLEMENT_SHADER(DefaultCHS, "RayTracingBuiltInShaders.lsl", "DefaultCHS", platform::Render::RayHitGroup);

IMPLEMENT_SHADER(DefaultMS, "RayTracingBuiltInShaders.lsl", "DefaultMS", platform::Render::RayMiss);

IMPLEMENT_SHADER(ShadowRG, "RayTracingScreenSpaceShadow.lsl", "RayGen", platform::Render::RayGen);