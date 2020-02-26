#include "RayTracingX.h"
#include "ShaderAsset.h"
#include "D3DShaderCompiler.h"
#include "../Core/AssetResourceScheduler.h"
#include "LFramework/Helper/ShellHelper.h"
#include "ShaderLoadingDesc.h"
#include <unordered_map>

using namespace platform;
using namespace asset;
using namespace platform::Render::Shader;
using namespace leo;

struct RayTracingShaderAsset :public asset::ShadersAsset,public asset::AssetName
{
	std::unordered_map<ShaderType, std::string> EntryPoints;
};

class RayTracingShaderLoadingDesc : public asset::AssetLoading<RayTracingShaderAsset>, public platform::X::ShaderLoadingDesc<RayTracingShaderAsset> {
private:
	using Super = platform::X::ShaderLoadingDesc<RayTracingShaderAsset>;
public:
	explicit RayTracingShaderLoadingDesc(platform::X::path const& shaderpath)
		:Super(shaderpath)
	{
	}

	std::size_t Type() const override {
		return leo::type_id<RayTracingShaderLoadingDesc>().hash_code();
	}

	std::size_t Hash() const override {
		return leo::hash_combine_seq(Type(), Super::Hash());
	}

	const asset::path& Path() const override {
		return Super::Path();
	}

	std::experimental::generator<std::shared_ptr<AssetType>> Coroutine() override {
		co_yield PreCreate();
		co_yield LoadNode();
		co_yield ParseNode();
		co_yield CreateAsset();
	}
private:
	std::shared_ptr<AssetType> PreCreate()
	{
		Super::PreCreate();
		return nullptr;
	}

	std::shared_ptr<AssetType> LoadNode()
	{
		Super::LoadNode();
		return  nullptr;
	}

	std::shared_ptr<AssetType> ParseNode()
	{
		Super::ParseNode();
		ParseEntryPoints();
		return nullptr;
	}

	std::shared_ptr<AssetType> CreateAsset()
	{
		Compile();
		return ReturnValue();
	}

	void ParseEntryPoints()
	{
		auto& node = GetNode();
		if (auto pRayGen = AccessPtr("raygen_shader", node))
		{
			GetAsset()->EntryPoints.emplace(ShaderType::RayGen, *pRayGen);
		}
		if (auto pRayMiss = AccessPtr("raymiss_shader", node))
		{
			GetAsset()->EntryPoints.emplace(ShaderType::RayMiss, *pRayMiss);
		}
		if (auto pRayHitGroup = AccessPtr("rayhitgroup_shader", node))
		{
			GetAsset()->EntryPoints.emplace(ShaderType::RayHitGroup, scheme::Deliteralize(*pRayHitGroup));
		}
		if (auto pRayCallable = AccessPtr("raycallable_shader", node))
		{
			GetAsset()->EntryPoints.emplace(ShaderType::RayCallable, *pRayCallable);
		}
	}

	void Compile()
	{
		auto path = Path().string();
#ifndef NDEBUG
		path += ".hlsl";
		{
			std::ofstream fout(path);
			fout << GetCode();
		}
#endif

		asset::X::Shader::ShaderCompilerInput input;
		input.Code = GetCode();
		input.SourceName = path;

		std::vector<ShaderMacro> macros{ GetAsset()->GetMacros() };
		for (auto& pair : GetAsset()->EntryPoints)
		{
			input.Type = pair.first;
			input.EntryPoint = pair.second;

			auto final_macros =  asset::X::Shader::AppendCompileMacros(macros, input.Type);

			auto pInfo = std::make_unique<ShaderInfo>(input.Type);

			LFL_DEBUG_DECL_TIMER(Commpile, sfmt("CompileAndReflect Type:%s ", input.EntryPoint.data()));
			auto blob =asset::X::Shader::CompileAndReflect(input, final_macros,
#ifndef NDEBUG
				D3DFlags::D3DCOMPILE_DEBUG
#else
				D3DFlags::D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif
				, pInfo.get()
			);

			auto blob_hash = leo::constfn_hash(input.EntryPoint);
			GetAsset()->EmplaceBlob(blob_hash, asset::ShaderBlobAsset(input.Type, std::move(blob), pInfo.release()));
		}
	}
};

#include "../Render/IContext.h"
#include "../Render/IRayContext.h"

using namespace platform::Render;

std::shared_ptr<Render::RayTracingShader> platform::X::LoadRayTracingShader(Render::RayDevice& Device, const path& filepath)
{
	auto pAsset = AssetResourceScheduler::Instance().SyncLoad<RayTracingShaderLoadingDesc>(filepath);

	auto& RayDevice = Context::Instance().GetRayContext().GetDevice();

	for (auto& pair : pAsset->EntryPoints)
	{
		auto blob_hash = leo::constfn_hash(pair.second);

		RayTracingShaderInitializer initializer;

		initializer.pBlob = &(pAsset->GetBlob(blob_hash));

		auto RayTracingInfos = initializer.pBlob->GetInfo().RayTracingInfos.value();

		initializer.EntryPoint = RayTracingInfos.EntryPoint;
		initializer.AnyHitEntryPoint = RayTracingInfos.AnyHitEntryPoint;
		initializer.IntersectionEntryPoint = RayTracingInfos.IntersectionEntryPoint;

		return shared_raw_robject(RayDevice.CreateRayTracingSahder(initializer));
	}

	return std::shared_ptr<Render::RayTracingShader>();
}
