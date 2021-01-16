#include "BuiltInRayTracingShaders.h"
#include "RayTracingShader.h"
#include "../../Asset/ShaderAsset.h"
#include "../../Asset/D3DShaderCompiler.h"
#include "Context.h"
#include "RayDevice.h"

using namespace platform_ex::Windows::D3D12;


IMPLEMENT_BUILTIN_SHADER(DefaultCHS, "RayTracing/RayTracingBuiltInShaders.lsl", "DefaultCHS", platform::Render::RayHitGroup);

IMPLEMENT_BUILTIN_SHADER(DefaultMS, "RayTracing/RayTracingBuiltInShaders.lsl", "DefaultMS", platform::Render::RayMiss);

IMPLEMENT_BUILTIN_SHADER(ShadowRG, "RayTracing/RayTracingScreenSpaceShadow.lsl", "RayGen", platform::Render::RayGen);