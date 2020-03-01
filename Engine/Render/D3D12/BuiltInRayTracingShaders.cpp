#include "BuiltInRayTracingShaders.h"
#include "RayTracingShader.h"
#include "../../Asset/ShaderAsset.h"
#include "LFramework/Helper/ShellHelper.h"
#include "../../Asset/D3DShaderCompiler.h"
#include "Context.h"
#include "RayDevice.h"

using namespace platform_ex::Windows::D3D12;

std::list<ShaderMeta*> GShaderTypeList;
ShaderMap<ShaderMeta> GGlobalShaderMap;

std::list<ShaderMeta*>& platform_ex::Windows::D3D12::ShaderMeta::GetTypeList()
{
	return GShaderTypeList;
}

Shader* platform_ex::Windows::D3D12::ShaderMeta::Construct() const
{
	return (*ConstructRef)();
}

ShaderMap<ShaderMeta>* platform_ex::Windows::D3D12::GetGlobalShaderMap()
{
	if (GGlobalShaderMap.IsEmpty())
		CompileGlobalShaderMap();

	return &GGlobalShaderMap;
}

void platform_ex::Windows::D3D12::BuiltInRayTracingShader::SetRayTracingShader(RayTracingShader* pShader)
{
	pRayTracingShader = platform::Render::shared_raw_robject(pShader);
}

RayTracingShader* platform_ex::Windows::D3D12::BuiltInRayTracingShader::GetRayTracingShader()
{
	return pRayTracingShader.get();
}

platform_ex::Windows::D3D12::ShaderMeta::ShaderMeta(const char* InName, const char* InSourceFileName, const char* InEntryPoint, platform::Render::ShaderType InFrequency,
	ConstructType InConstructRef)
	:TypeName(InName), SourceFileName(InSourceFileName),
	EntryPoint(InEntryPoint), Frequency(InFrequency)
	, ConstructRef(InConstructRef)
{
	GetTypeList().emplace_front(this);
}

void platform_ex::Windows::D3D12::CompileGlobalShaderMap()
{
	auto& RayDevice = Context::Instance().GetRayContext().GetDevice();

	for (auto meta : ShaderMeta::GetTypeList())
	{
		if (!IsRayTracingShader(meta->GetShaderType()))
			continue;

		asset::X::Shader::ShaderCompilerInput input;

		input.Code = platform::X::GenHlslShader(meta->GetSourceFileName());
		input.EntryPoint = meta->GetEntryPoint();
		input.Type = meta->GetShaderType();

		auto final_macros = asset::X::Shader::AppendCompileMacros({}, input.Type);

		ShaderInfo Info{input.Type};

		LFL_DEBUG_DECL_TIMER(Commpile, sfmt("CompileGlobalShaderMap EntryPoint:%s ", input.EntryPoint.data()));
		auto blob = asset::X::Shader::CompileAndReflect(input, final_macros,
#ifndef NDEBUG
			D3DFlags::D3DCOMPILE_DEBUG
#else
			D3DFlags::D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif
			,&Info
		);

		

		platform::Render::RayTracingShaderInitializer initializer;
		initializer.pBlob = &blob;
		initializer.pInfo = &Info;

		auto pRayTracingShaderRHI = RayDevice.CreateRayTracingSahder(initializer);

		auto pShader =static_cast<BuiltInRayTracingShader*>(meta->Construct());

		pShader->SetRayTracingShader(pRayTracingShaderRHI);

		GGlobalShaderMap.AddShader(meta, pShader);
	}
}

IMPLEMENT_SHADER(DefaultCHS, "RayTracingBuiltInShaders.lsl", "DefaultCHS ", platform::Render::RayHitGroup);

IMPLEMENT_SHADER(DefaultMS, "RayTracingBuiltInShaders.lsl", "DefaultMS ", platform::Render::RayMiss);