#include "Shader.h"
#include "BuiltInRayTracingShader.h"
#include "IContext.h"
#include "IRayContext.h"
#include "../Asset/D3DShaderCompiler.h"
#include "LFramework/Helper/ShellHelper.h"
#include "../Core/Coroutine/Task.h"
#include "../Core/Coroutine/ThreadScheduler.h"
#include "../Core/Coroutine/SyncWait.h"
#include "../Core/Coroutine/WhenAllReady.h"
#include "../System/SystemEnvironment.h"
#include "../Core/Hash/CityHash.h"
#include "BuiltInShader.h"


using namespace platform::Render;

namespace platform::Render::Shader
{
	std::list<ShaderMeta*>* GShaderTypeList;

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
		if (GShaderTypeList == nullptr)
			GShaderTypeList = new std::list<ShaderMeta*>();
		return *GShaderTypeList;
	}

	RenderShader* ShaderMeta::Construct() const
	{
		return (*ConstructRef)();
	}

	ShaderMeta::ShaderMeta(EShaderMetaForDownCast InShaderMetaForDownCast, const char* InName, const char* InSourceFileName, const char* InEntryPoint, platform::Render::ShaderType InFrequency, int32 InTotalPermutationCount,
		ConstructType InConstructRef)
		:
		ShaderMetaForDownCast(InShaderMetaForDownCast),
		TypeName(InName), 
		Hash(std::hash<std::string>()(TypeName)),
		SourceFileName(InSourceFileName),
		HashedShaderFilename(std::hash<std::string>()(SourceFileName)),
		EntryPoint(InEntryPoint), 
		Frequency(InFrequency), 
		TotalPermutationCount(InTotalPermutationCount),
		ConstructRef(InConstructRef)
	{
		GetTypeList().emplace_front(this);
	}

	ImplDeCtor(RenderShader);
	ImplDeDtor(RenderShader);

	RenderShader::RenderShader(const CompiledShaderInitializer& initializer)
	{
		Shader = leo::unique_raw(initializer.Shader);
	}

	static void FillParameterMapByShaderInfo(ShaderParameterMap& target, const ShaderInfo& src);

	BuiltInShaderMap GGlobalShaderMap;


	leo::coroutine::Task<void> CompileShader(ShaderMeta* meta,int32 )
	{
		auto& Device = Context::Instance().GetDevice();
		auto& RayDevice = Context::Instance().GetRayContext().GetDevice();

		asset::X::Shader::ShaderCompilerInput input;
		input.EntryPoint = meta->GetEntryPoint();
		input.Type = meta->GetShaderType();
		input.SourceName = meta->GetSourceFileName();

		LFL_DEBUG_DECL_TIMER(Commpile, sfmt("CompileShader %s- Entry:%s ", input.SourceName.data(), input.EntryPoint.data()));

		auto Code = co_await platform::X::GenHlslShaderAsync(meta->GetSourceFileName());
		input.Code = Code;

		auto final_macros = asset::X::Shader::AppendCompileMacros({}, input.Type);

		ShaderInfo Info{ input.Type };

		auto blob = asset::X::Shader::CompileAndReflect(input, final_macros,
#ifndef NDEBUG
			D3DFlags::D3DCOMPILE_DEBUG
#else
			D3DFlags::D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif
			, &Info
		);

		if (IsRayTracingShader(meta->GetShaderType()))
		{
			platform::Render::RayTracingShaderInitializer initializer;
			initializer.pBlob = &blob;
			initializer.pInfo = &Info;

			auto pRayTracingShaderRHI = RayDevice.CreateRayTracingSahder(initializer);

			auto pShader = static_cast<BuiltInRayTracingShader*>(meta->Construct());

			pShader->SetRayTracingShader(pRayTracingShaderRHI);

			GGlobalShaderMap.FindOrAddShader(meta,0, pShader);
		}
		else if (auto pBuiltInMeta = meta->GetBuiltInShaderType()) {
			platform::Render::ShaderInitializer initializer;
			initializer.pBlob = &blob;
			initializer.pInfo = &Info;

			auto pShaderRHI = Device.CreateShader(initializer);

			RenderShader::CompiledShaderInitializer compileOuput;
			compileOuput.Shader = pShaderRHI;
			FillParameterMapByShaderInfo(compileOuput.ParameterMap, Info);

			auto pShader = pBuiltInMeta->Construct(compileOuput);

			GGlobalShaderMap.FindOrAddShader(meta,0, pShader);
		}

		co_return;
	}

	void CompileShaderMap()
	{
		LFL_DEBUG_DECL_TIMER(Commpile, "CompileGlobalShaderMap");

		std::vector< leo::coroutine::Task<void>> tasks;
		for (auto meta : ShaderMeta::GetTypeList())
		{
			int32 PermutationCountToCompile = 0;
			for (int32 PermutationId = 0; PermutationId < meta->GetPermutationCount(); PermutationId++)
			{
				auto task = Environment->Scheduler->Schedule(CompileShader(meta, PermutationId));
				tasks.emplace_back(std::move(task));
			}
		}

		leo::coroutine::SyncWait(leo::coroutine::WhenAllReady(std::move(tasks)));
	}

	void FillParameterMapByShaderInfo(ShaderParameterMap& target, const ShaderInfo& src)
	{
		for (auto& cb : src.ConstantBufferInfos)
		{
			auto& name = cb.name;

			bool bGlobalCB = name == "$Globals";
			if (bGlobalCB)
			{
				for (auto& var : cb.var_desc)
				{
					target.AddParameterAllocation(var.name, cb.bind_point,
						var.start_offset, var.size, ShaderParamClass::LooseData);
				}
			}
			else
			{
				target.AddParameterAllocation(name, 0,cb.bind_point,
					 cb.size, ShaderParamClass::UniformBuffer);
			}
		}
		for (auto& br : src.BoundResourceInfos)
		{
			ShaderParamClass Class = static_cast<ShaderParamClass>(br.type);

			target.AddParameterAllocation(br.name, 0, br.bind_point,
				 1, Class);
		}
	}

	BuiltInShaderMap* Shader::GetBuiltInShaderMap()
	{
		return &GGlobalShaderMap;
	}
}

RenderShader* Shader::ShaderMapContent::GetShader(size_t TypeNameHash, int32 PermutationId) const
{
	auto Hash = (uint16)CityHash128to64({ TypeNameHash,(uint64)PermutationId });

	auto range = ShaderHash.equal_range(Hash);
	for (auto itr = range.first; itr != range.second; ++itr)
	{
		auto Index = itr->second;
		if (ShaderTypes[Index] == TypeNameHash && ShaderPermutations[Index] == PermutationId)
		{
			return Shaders[Index];
		}
	}
	return nullptr;
}

RenderShader* Shader::ShaderMapContent::FindOrAddShader(size_t TypeNameHash, int32 PermutationId, RenderShader* Shader)
{
	auto Shader = GetShader(TypeNameHash, PermutationId);

	if (Shader != nullptr)
		return Shader;

	auto Hash = (uint16)CityHash128to64({ TypeNameHash,(uint64)PermutationId });

	const int32 Index = Shaders.size();

	ShaderHash.emplace(Hash, Index);
	Shaders.emplace_back(Shader);
	ShaderTypes.emplace_back(TypeNameHash);
	ShaderPermutations.emplace_back(PermutationId);
}

void Shader::ShaderMapContent::Clear()
{
	for (auto Shader : Shaders)
	{
		delete Shader;
	}

	Shaders.clear();
	ShaderTypes.clear();
	ShaderPermutations.clear();
	ShaderHash.clear();
}

uint32 Shader::ShaderMapContent::GetNumShaders() const
{
	return Shaders.size();
}
