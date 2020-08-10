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



using namespace platform::Render;

namespace platform::Render::Shader
{
	std::list<ShaderMeta*>* GShaderTypeList;
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
		if (GShaderTypeList == nullptr)
			GShaderTypeList = new std::list<ShaderMeta*>();
		return *GShaderTypeList;
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

	ShaderMeta::ShaderMeta(EShaderMetaForDownCast InShaderMetaForDownCast, const char* InName, const char* InSourceFileName, const char* InEntryPoint, platform::Render::ShaderType InFrequency,
		ConstructType InConstructRef)
		:
		ShaderMetaForDownCast(InShaderMetaForDownCast),
		TypeName(InName), 
		SourceFileName(InSourceFileName),
		EntryPoint(InEntryPoint), 
		Frequency(InFrequency), 
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

	leo::coroutine::Task<void> CompileShader(ShaderMeta* meta)
	{
		auto& Device = Context::Instance().GetDevice();
		auto& RayDevice = Context::Instance().GetRayContext().GetDevice();

		asset::X::Shader::ShaderCompilerInput input;

		auto ShaderCode =co_await platform::X::GenHlslShaderAsync(meta->GetSourceFileName());
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

		if (IsRayTracingShader(meta->GetShaderType()))
		{
			platform::Render::RayTracingShaderInitializer initializer;
			initializer.pBlob = &blob;
			initializer.pInfo = &Info;

			auto pRayTracingShaderRHI = RayDevice.CreateRayTracingSahder(initializer);

			auto pShader = static_cast<BuiltInRayTracingShader*>(meta->Construct());

			pShader->SetRayTracingShader(pRayTracingShaderRHI);

			GGlobalShaderMap.AddShader(meta, pShader);
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

			GGlobalShaderMap.AddShader(meta, pShader);
		}

		co_return;
	}

	void CompileGlobalShaderMap()
	{
		LFL_DEBUG_DECL_TIMER(Commpile, "CompileGlobalShaderMap");

		std::vector< leo::coroutine::Task<void>> tasks;
		for (auto meta : ShaderMeta::GetTypeList())
		{
			auto task = Environment->Scheduler->Schedule(CompileShader(meta));
			tasks.emplace_back(std::move(task));
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
}


