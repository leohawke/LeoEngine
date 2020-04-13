#include "Shader.h"
#include "IContext.h"
#include "IRayContext.h"
#include "../Asset/D3DShaderCompiler.h"
#include "LFramework/Helper/ShellHelper.h"


using namespace platform::Render;

namespace platform::Render::Shader
{
	std::list<ShaderMeta*> GShaderTypeList;
	ShaderMap<ShaderMeta> GGlobalShaderMap;

	bool IsRayTracingShader(ShaderType type)
	{
		switch (type)
		{
		case ShaderType::RayGen:
		case ShaderType::RayMiss:
		case ShaderType::RayHitGroup:
		case ShaderType::RayCallable:
			return true;
		default:
			return false;
		}
	}

	std::list<ShaderMeta*>& ShaderMeta::GetTypeList()
	{
		return GShaderTypeList;
	}

	RenderShader* ShaderMeta::Construct() const
	{
		return (*ConstructRef)();
	}

	ShaderMap<ShaderMeta>* GetGlobalShaderMap()
	{
		if (GGlobalShaderMap.IsEmpty())
			CompileGlobalShaderMap();

		return &GGlobalShaderMap;
	}

	ShaderMeta::ShaderMeta(const char* InName, const char* InSourceFileName, const char* InEntryPoint, platform::Render::ShaderType InFrequency,
		ConstructType InConstructRef)
		:TypeName(InName), SourceFileName(InSourceFileName),
		EntryPoint(InEntryPoint), Frequency(InFrequency)
		, ConstructRef(InConstructRef)
	{
		GetTypeList().emplace_front(this);
	}

	void CompileGlobalShaderMap()
	{
		auto& RayDevice = Context::Instance().GetRayContext().GetDevice();

		for (auto meta : ShaderMeta::GetTypeList())
		{
			if (!IsRayTracingShader(meta->GetShaderType()))
				continue;

			asset::X::Shader::ShaderCompilerInput input;

			auto ShaderCode = platform::X::GenHlslShader(meta->GetSourceFileName());
			input.Code = ShaderCode;
			input.EntryPoint = meta->GetEntryPoint();
			input.Type = meta->GetShaderType();

			auto final_macros = asset::X::Shader::AppendCompileMacros({}, input.Type);

			ShaderInfo Info{ input.Type };

			LFL_DEBUG_DECL_TIMER(Commpile, sfmt("CompileGlobalShaderMap EntryPoint:%s ", input.EntryPoint.data()));
			auto blob = asset::X::Shader::CompileAndReflect(input, final_macros,
#ifndef NDEBUG
				D3DFlags::D3DCOMPILE_DEBUG
#else
				D3DFlags::D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif
				, &Info
			);



			platform::Render::RayTracingShaderInitializer initializer;
			initializer.pBlob = &blob;
			initializer.pInfo = &Info;

			auto pRayTracingShaderRHI = RayDevice.CreateRayTracingSahder(initializer);

			auto pShader = static_cast<BuiltInRayTracingShader*>(meta->Construct());

			pShader->SetRayTracingShader(pRayTracingShaderRHI);

			GGlobalShaderMap.AddShader(meta, pShader);
		}
	}
}