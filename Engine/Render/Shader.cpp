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

	bool Shader::ShaderMeta::ShouldCompilePermutation(const FShaderPermutationParameters& Parameters) const
	{
		return (*ShouldCompilePermutationRef)(Parameters);
	}

	void ShaderMeta::ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment) const
	{
		(*ModifyCompilationEnvironmentRef)(Parameters, OutEnvironment);
	}

	ShaderMeta::ShaderMeta(EShaderMetaForDownCast InShaderMetaForDownCast, const char* InName, const char* InSourceFileName, const char* InEntryPoint, platform::Render::ShaderType InFrequency, int32 InTotalPermutationCount,
		ConstructType InConstructRef, 
		ModifyCompilationEnvironmentType InModifyCompilationEnvironmentRef,
		ShouldCompilePermutationType InShouldCompilePermutationRef,
		uint32 InTypeSize,
		const ShaderParametersMetadata* InRootParametersMetadata
	)
		:
		ShaderMetaForDownCast(InShaderMetaForDownCast),
		TypeName(InName), 
		Hash(std::hash<std::string>()(TypeName)),
		SourceFileName(InSourceFileName),
		HashedShaderFilename(std::hash<std::string>()(SourceFileName)),
		EntryPoint(InEntryPoint), 
		Frequency(InFrequency), 
		TotalPermutationCount(InTotalPermutationCount),
		ConstructRef(InConstructRef),
		ModifyCompilationEnvironmentRef(InModifyCompilationEnvironmentRef),
		ShouldCompilePermutationRef(InShouldCompilePermutationRef),
		RootParametersMetadata(InRootParametersMetadata)
	{
		GetTypeList().emplace_front(this);
	}

	ImplDeCtor(RenderShader);
	ImplDeDtor(RenderShader);

	RenderShader::RenderShader(const CompiledShaderInitializer& initializer)
		:
		Shader(initializer.Shader),
		Meta(initializer.Meta)
	{
	}

	static void FillParameterMapByShaderInfo(ShaderParameterMap& target, const ShaderInfo& src);

	BuiltInShaderMap GGlobalBuiltInShaderMap;


	leo::coroutine::Task<void> CompileBuiltInShader(BuiltInShaderMeta* meta,int32 PermutationId)
	{
		auto& Device = Context::Instance().GetDevice();
		auto& RayDevice = Context::Instance().GetRayContext().GetDevice();

		asset::X::Shader::ShaderCompilerInput input;

		input.EntryPoint = meta->GetEntryPoint();
		input.Type = meta->GetShaderType();
		input.SourceName = meta->GetSourceFileName();
		// Allow the shader type to modify the compile environment.
		meta->SetupCompileEnvironment(PermutationId, input.Environment);

		LFL_DEBUG_DECL_TIMER(Commpile, sfmt("CompileShader %s- Entry:%s ", input.SourceName.data(), input.EntryPoint.data()));

		auto Code = co_await platform::X::GenHlslShaderAsync(meta->GetSourceFileName());
		input.Code = Code;

		asset::X::Shader::AppendCompilerEnvironment(input.Environment, input.Type);

		ShaderInfo Info{ input.Type };

		auto blob = asset::X::Shader::CompileAndReflect(input,
#ifndef NDEBUG
			D3DFlags::D3DCOMPILE_DEBUG
#else
			D3DFlags::D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif
			, &Info
		);

		auto pBuiltInMeta = meta->GetBuiltInShaderType();
		if (!pBuiltInMeta)
			co_return;

		if (IsRayTracingShader(meta->GetShaderType()))
		{
			platform::Render::RayTracingShaderInitializer initializer;
			initializer.pBlob = &blob;
			initializer.pInfo = &Info;

			auto pRayTracingShaderRHI = RayDevice.CreateRayTracingSahder(initializer);

			RenderShader::CompiledShaderInitializer compileOuput;
			compileOuput.Shader = pRayTracingShaderRHI;
			compileOuput.Meta = meta;
			FillParameterMapByShaderInfo(compileOuput.ParameterMap, Info);

			auto pShader = static_cast<BuiltInRayTracingShader*>(pBuiltInMeta->Construct(compileOuput));

			GGlobalBuiltInShaderMap.FindOrAddShader(meta, PermutationId, pShader);
		}
		else{
			platform::Render::ShaderInitializer initializer;
			initializer.pBlob = &blob;
			initializer.pInfo = &Info;

			auto pShaderRHI = Device.CreateShader(initializer);

			RenderShader::CompiledShaderInitializer compileOuput;
			compileOuput.Shader = pShaderRHI;
			compileOuput.Meta = meta;

			FillParameterMapByShaderInfo(compileOuput.ParameterMap, Info);

			auto pShader = pBuiltInMeta->Construct(compileOuput);

			GGlobalBuiltInShaderMap.FindOrAddShader(meta, PermutationId, pShader);
		}

		co_return;
	}

	void CompileShaderMap()
	{
		LFL_DEBUG_DECL_TIMER(Commpile, "CompileGlobalShaderMap");

		std::vector< leo::coroutine::Task<void>> tasks;
		for (auto meta : ShaderMeta::GetTypeList())
		{
			//TODO:dispatch type
			if (auto pBuiltInMeta = meta->GetBuiltInShaderType())
			{
				int32 PermutationCountToCompile = 0;
				for (int32 PermutationId = 0; PermutationId < meta->GetPermutationCount(); PermutationId++)
				{
					if (!pBuiltInMeta->ShouldCompilePermutation(PermutationId))
						continue;

					auto task = Environment->Scheduler->Schedule(CompileBuiltInShader(pBuiltInMeta, PermutationId));
					tasks.emplace_back(std::move(task));
				}
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
		return &GGlobalBuiltInShaderMap;
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
	{
		std::shared_lock lock{ ShaderMutex };
		auto FindShader = GetShader(TypeNameHash, PermutationId);

		if (FindShader != nullptr)
			return FindShader;
	}

	std::unique_lock lock{ ShaderMutex };

	auto Hash = (uint16)CityHash128to64({ TypeNameHash,(uint64)PermutationId });

	const int32 Index = GetNumShaders();

	ShaderHash.emplace(Hash, Index);
	Shaders.emplace_back(Shader);
	ShaderTypes.emplace_back(TypeNameHash);
	ShaderPermutations.emplace_back(PermutationId);

	return Shader;
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
	return static_cast<int32>(Shaders.size());
}
