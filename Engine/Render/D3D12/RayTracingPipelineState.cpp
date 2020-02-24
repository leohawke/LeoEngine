#include "RayTracingPipelineState.h"
#include "D3D12RayTracing.h"
#include "Context.h"

using namespace platform_ex::Windows::D3D12;

RayTracingPipelineState::RayTracingPipelineState(const platform::Render::RayTracingPipelineStateInitializer& initializer)
{
	//TODO:facll back to default ones if none were provided
	leo::span<platform::Render::RayTracingShader*> InitializerHitGroups = initializer.HitGroupTable;
	leo::span<platform::Render::RayTracingShader*> InitializerMissShaders = initializer.MissTable;

	leo::span<platform::Render::RayTracingShader*> InitializerRayGenShaders = initializer.RayGenTable;
	leo::span<platform::Render::RayTracingShader*> InitializerCallableShaders = initializer.CallableTable;

	const uint32 MaxTotalShaders =static_cast<uint32>(InitializerRayGenShaders.size() + InitializerMissShaders.size() + InitializerHitGroups.size() + InitializerCallableShaders.size());

	// All raygen and miss shaders must share the same global root signature, so take the first one and validate the rest

	GlobalRootSignature =static_cast<RayTracingShader*>(InitializerRayGenShaders[0])->pRootSignature;

	//TODO:CompieShader GetName
	auto GetPrimaryExportNameChars = [](RayTracingShader* Shader, RayTracingPipelineCache::CollectionType CollectionType)
	{
		return L"";
	};

	bAllowHitGroupIndexing = initializer.HitGroupTable.size() ? initializer.bAllowHitGroupIndexing : false;

	// Add ray generation shaders

	leo::vector<LPCWSTR> RayGenShaderNames;

	RayGenShaders.Reserve(static_cast<uint32>(InitializerRayGenShaders.size()));
	RayGenShaderNames.reserve(InitializerRayGenShaders.size());

	for (auto ShaderInterface : InitializerRayGenShaders)
	{
		auto Shader = static_cast<RayTracingShader*>(ShaderInterface);

		Shader->AddRef();

		LAssert(Shader->pRootSignature == GlobalRootSignature, "All raygen and miss shaders must share the same root signature");

		RayGenShaderNames.emplace_back(GetPrimaryExportNameChars(Shader, RayTracingPipelineCache::CollectionType::RayGen));
		RayGenShaders.Shaders.emplace_back(platform::Render::shared_raw_robject(Shader));
	}

	// Add miss shaders
	leo::vector<LPCWSTR> MissShaderNames;

	MissShaders.Reserve(static_cast<uint32>(InitializerRayGenShaders.size()));
	MissShaderNames.reserve(InitializerRayGenShaders.size());

	for (auto ShaderInterface : InitializerMissShaders)
	{
		auto Shader = static_cast<RayTracingShader*>(ShaderInterface);

		Shader->AddRef();

		LAssert(Shader->pRootSignature == GlobalRootSignature, "All raygen and miss shaders must share the same root signature");

		MissShaderNames.emplace_back(GetPrimaryExportNameChars(Shader, RayTracingPipelineCache::CollectionType::Miss));
		MissShaders.Shaders.emplace_back(platform::Render::shared_raw_robject(Shader));
	}

	// Add hit groups

	MaxHitGroupViewDescriptors = 0;
	MaxLocalRootSignatureSize = 0;

	leo::vector<LPCWSTR> HitGroupNames;
	HitGroupShaders.Reserve(static_cast<uint32>(InitializerHitGroups.size()));
	HitGroupNames.reserve(InitializerHitGroups.size());

	for (auto ShaderInterface : InitializerHitGroups)
	{
		auto Shader = static_cast<RayTracingShader*>(ShaderInterface);

		Shader->AddRef();

		const uint32 ShaderViewDescriptors = Shader->ResourceCounts.NumSRVs + Shader->ResourceCounts.NumUAVs;
		MaxHitGroupViewDescriptors = std::max(MaxHitGroupViewDescriptors, ShaderViewDescriptors);
		MaxLocalRootSignatureSize = std::max(MaxLocalRootSignatureSize, Shader->pRootSignature->GetTotalRootSignatureSizeInBytes());

		HitGroupNames.emplace_back(GetPrimaryExportNameChars(Shader, RayTracingPipelineCache::CollectionType::HitGroup));
		HitGroupShaders.Shaders.emplace_back(platform::Render::shared_raw_robject(Shader));
	}

	leo::vector<LPCWSTR> CallableShaderNames;
	CallableShaders.Reserve(static_cast<uint32>(InitializerCallableShaders.size()));
	CallableShaderNames.reserve(InitializerCallableShaders.size());

	for (auto ShaderInterface : InitializerCallableShaders)
	{
		auto Shader = static_cast<RayTracingShader*>(ShaderInterface);

		Shader->AddRef();

		const uint32 ShaderViewDescriptors = Shader->ResourceCounts.NumSRVs + Shader->ResourceCounts.NumUAVs;
		MaxHitGroupViewDescriptors = std::max(MaxHitGroupViewDescriptors, ShaderViewDescriptors);
		MaxLocalRootSignatureSize = std::max(MaxLocalRootSignatureSize, Shader->pRootSignature->GetTotalRootSignatureSizeInBytes());

		CallableShaderNames.emplace_back(GetPrimaryExportNameChars(Shader, RayTracingPipelineCache::CollectionType::Callable));
		CallableShaders.Shaders.emplace_back(platform::Render::shared_raw_robject(Shader));
	}

	std::vector<D3D12_EXISTING_COLLECTION_DESC> UniqueShaderCollectionDescs;

	auto RayTracingDevice = Context::Instance().GetRayContext().GetDevice().GetRayTracingDevice();

	StateObject = CreateRayTracingStateObject(
		RayTracingDevice,
		{},
		{},
		initializer.MaxPayloadSizeInBytes,
		{},
		GlobalRootSignature->Signature.Get(),
		{},
		{},
		leo::make_span(UniqueShaderCollectionDescs),
		D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE
	);

	HRESULT QueryInterfaceResult = StateObject->QueryInterface(COMPtr_RefParam(PipelineProperties,IID_ID3D12StateObjectProperties));

	LAssert(SUCCEEDED(QueryInterfaceResult), "Failed to query pipeline properties from the ray tracing pipeline state object.");

	auto GetShaderIdentifier = [PipelineProperties = this->PipelineProperties.Get()](LPCWSTR ExportName)->ShaderIdentifier
	{
		ShaderIdentifier Result;

		const void* Data = PipelineProperties->GetShaderIdentifier(ExportName);
		LAssert(Data, "Couldn't find requested export in the ray tracing shader pipeline");

		if (Data)
		{
			Result.SetData(Data);
		}

		return Result;
	};

	// Query shader identifiers from the pipeline state object

	HitGroupShaders.Identifiers.reserve(InitializerHitGroups.size());
	for (int32 HitGroupIndex = 0; HitGroupIndex < HitGroupNames.size(); ++HitGroupIndex)
	{
		LPCWSTR ExportNameChars = HitGroupNames[HitGroupIndex];
		HitGroupShaders.Identifiers[HitGroupIndex] = GetShaderIdentifier(ExportNameChars);
	}

	RayGenShaders.Identifiers.reserve(RayGenShaderNames.size());
	for (int32 ShaderIndex = 0; ShaderIndex < RayGenShaderNames.size(); ++ShaderIndex)
	{
		LPCWSTR ExportNameChars = RayGenShaderNames[ShaderIndex];
		RayGenShaders.Identifiers[ShaderIndex] = GetShaderIdentifier(ExportNameChars);
	}

	MissShaders.Identifiers.reserve(MissShaderNames.size());
	for (int32 ShaderIndex = 0; ShaderIndex < MissShaderNames.size(); ++ShaderIndex)
	{
		LPCWSTR ExportNameChars = MissShaderNames[ShaderIndex];
		MissShaders.Identifiers[ShaderIndex] = GetShaderIdentifier(ExportNameChars);
	}

	CallableShaders.Identifiers.reserve(CallableShaderNames.size());
	for (int32 ShaderIndex = 0; ShaderIndex < CallableShaderNames.size(); ++ShaderIndex)
	{
		LPCWSTR ExportNameChars = CallableShaderNames[ShaderIndex];
		CallableShaders.Identifiers[ShaderIndex] = GetShaderIdentifier(ExportNameChars);
	}

	// Setup default shader binding table, which simply includes all provided RGS and MS plus a single default closest hit shader.
		// Hit record indexing and local resources access is disabled when using using this SBT.

	::RayTracingShaderTable::Initializer SBTInitializer = {};
	SBTInitializer.NumRayGenShaders =static_cast<leo::uint32>(RayGenShaders.Identifiers.size());
	SBTInitializer.NumMissShaders = static_cast<leo::uint32>(MissShaders.Identifiers.size());
	SBTInitializer.NumCallableRecords = 0; // Default SBT does not support callable shaders
	SBTInitializer.NumHitRecords = 0; // Default SBT does not support indexable hit shaders
	SBTInitializer.LocalRootDataSize = 0; // Shaders in default SBT are not allowed to access any local resources

	DefaultShaderTable.Init(SBTInitializer);
	DefaultShaderTable.SetRayGenIdentifiers(RayGenShaders.Identifiers);
	DefaultShaderTable.SetMissIdentifiers(MissShaders.Identifiers);
	DefaultShaderTable.SetDefaultHitGroupIdentifier(HitGroupShaders.Identifiers[0]);
}
