#include "RayTracingPipelineState.h"
#include "D3D12RayTracing.h"

using namespace platform_ex::Windows::D3D12;

RayTracingPipelineState::RayTracingPipelineState(const platform::Render::RayTracingPipelineStateInitializer& initializer)
{
	//TODO:facll back to default ones if none were provided
	leo::span<platform::Render::RayTracingShader*> InitializerHitGroups = initializer.HitGroupTable;
	leo::span<platform::Render::RayTracingShader*> InitializerMissShaders = initializer.MissTable;

	leo::span<platform::Render::RayTracingShader*> InitializerRayGenShaders = initializer.RayGenTable;
	leo::span<platform::Render::RayTracingShader*> InitializerCallableShaders = initializer.CallableTable;

	const uint32 MaxTotalShaders = InitializerRayGenShaders.size() + InitializerMissShaders.size() + InitializerHitGroups.size() + InitializerCallableShaders.size();

	// All raygen and miss shaders must share the same global root signature, so take the first one and validate the rest

	GlobalRootSignature =static_cast<RayTracingShader*>(InitializerRayGenShaders[0])->pRootSigneature;

	//TODO:CompieShader GetName
	auto GetPrimaryExportNameChars = [](RayTracingShader* Shader, RayTracingPipelineCache::CollectionType CollectionType)
	{
		return L"";
	};

	bAllowHitGroupIndexing = initializer.HitGroupTable.size() ? initializer.bAllowHitGroupIndexing : false;

	// Add ray generation shaders

	leo::vector<LPCWSTR> RayGenShaderNames;

	RayGenShaders.Reserve(InitializerRayGenShaders.size());
	RayGenShaderNames.reserve(InitializerRayGenShaders.size());

	for (auto ShaderInterface : InitializerRayGenShaders)
	{
		auto Shader = static_cast<RayTracingShader*>(ShaderInterface);

		Shader->AddRef();

		LAssert(Shader->pRootSigneature == GlobalRootSignature, "All raygen and miss shaders must share the same root signature");

		RayGenShaderNames.emplace_back(GetPrimaryExportNameChars(Shader, RayTracingPipelineCache::CollectionType::RayGen));
		RayGenShaders.Shaders.emplace_back(platform::Render::shared_raw_robject(Shader));
	}

	// Add miss shaders
	leo::vector<LPCWSTR> MissShaderNames;

	MissShaders.Reserve(InitializerRayGenShaders.size());
	MissShaderNames.reserve(InitializerRayGenShaders.size());

	for (auto ShaderInterface : InitializerMissShaders)
	{
		auto Shader = static_cast<RayTracingShader*>(ShaderInterface);

		Shader->AddRef();

		LAssert(Shader->pRootSigneature == GlobalRootSignature, "All raygen and miss shaders must share the same root signature");

		MissShaderNames.emplace_back(GetPrimaryExportNameChars(Shader, RayTracingPipelineCache::CollectionType::Miss));
		MissShaders.Shaders.emplace_back(platform::Render::shared_raw_robject(Shader));
	}

	// Add hit groups

	MaxHitGroupViewDescriptors = 0;
	MaxLocalRootSignatureSize = 0;

	leo::vector<LPCWSTR> HitGroupNames;
	HitGroupShaders.Reserve(InitializerHitGroups.size());
	HitGroupNames.reserve(InitializerHitGroups.size());

	for (auto ShaderInterface : InitializerHitGroups)
	{
		auto Shader = static_cast<RayTracingShader*>(ShaderInterface);

		Shader->AddRef();

		const uint32 ShaderViewDescriptors = Shader->ResourceCounts.NumSRVs + Shader->ResourceCounts.NumUAVs;
		MaxHitGroupViewDescriptors = std::max(MaxHitGroupViewDescriptors, ShaderViewDescriptors);
		MaxLocalRootSignatureSize = std::max(MaxLocalRootSignatureSize, Shader->pRootSigneature->GetTotalRootSignatureSizeInBytes());

		HitGroupNames.emplace_back(GetPrimaryExportNameChars(Shader, RayTracingPipelineCache::CollectionType::HitGroup));
		HitGroupShaders.Shaders.emplace_back(platform::Render::shared_raw_robject(Shader));
	}
}
