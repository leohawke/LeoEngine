#include "PipelineStateUtility.h"
#include "IContext.h"
#include <LBase/smart_ptr.hpp>
#include "../Core/Hash/CityHash.h"
using namespace platform::Render;

namespace platform::Render::PipelineStateCache
{
	class FGraphicsPipelineState
	{
	public:
		std::shared_ptr<GraphicsPipelineState> IPipeline;
	};

	GraphicsPipelineState* ExecuteSetGraphicsPipelineState(FGraphicsPipelineState* PipelineState)
	{
		return PipelineState->IPipeline.get();
	}
}

void platform::Render::SetGraphicsPipelineState(CommandList& cmdlist, const GraphicsPipelineStateInitializer& initializer)
{
	auto PipelineState = PipelineStateCache::GetAndOrCreateGraphicsPipelineState(cmdlist, initializer);

	auto IPipeline = PipelineStateCache::ExecuteSetGraphicsPipelineState(PipelineState);

	cmdlist.SetGraphicsPipelineState(IPipeline);
}

struct PipelineStateCacheKeyFuncs
{
	typedef GraphicsPipelineStateInitializer KeyInitType;

	bool operator()(const KeyInitType& A, const KeyInitType& B) const
	{
		if (A.ShaderPass.VertexDeclaration != B.ShaderPass.VertexDeclaration ||
			A.ShaderPass.VertexShader != B.ShaderPass.VertexShader ||
			A.ShaderPass.PixelShader != B.ShaderPass.PixelShader ||
			A.ShaderPass.GeometryShader != B.ShaderPass.GeometryShader ||
			A.ShaderPass.DomainShader != B.ShaderPass.DomainShader ||
			A.ShaderPass.HullShader != B.ShaderPass.HullShader ||
			A.RasterizerState != B.RasterizerState ||
			A.DepthStencilState != B.DepthStencilState ||
			A.BlendState != B.BlendState ||
			A.Primitive != B.Primitive ||
			A.RenderTargetsEnabled != B.RenderTargetsEnabled ||
			A.RenderTargetFormats != B.RenderTargetFormats ||
			A.DepthStencilTargetFormat != B.DepthStencilTargetFormat ||
			A.NumSamples != B.NumSamples
			)
		{
			return false;
		}
		return true;
	}

	std::size_t operator()(const ShaderPassInput& ShaderPass) const
	{
		auto offset = reinterpret_cast<const char*>(&ShaderPass.VertexShader) - reinterpret_cast<const char*>(&ShaderPass);

		auto TailHash = CityHash32(reinterpret_cast<const char*>(&ShaderPass.VertexShader), static_cast<uint32>(sizeof(ShaderPassInput) - offset));

		auto HeadHash = CityHash32(reinterpret_cast<const char*>(ShaderPass.VertexDeclaration.data()), static_cast<uint32>(ShaderPass.VertexDeclaration.size() * sizeof(VertexElement)));

		return ((size_t)HeadHash << 32) | TailHash;
	}

	std::size_t operator()(const KeyInitType& Key) const
	{
		auto offset = reinterpret_cast<const char*>(&Key.RasterizerState) - reinterpret_cast<const char*>(&Key);

		auto TailHash = CityHash64(reinterpret_cast<const char*>(&Key.RasterizerState),static_cast<uint32>(sizeof(KeyInitType) - offset));

		auto HeadHash = operator()(Key.ShaderPass);
		leo::hash_combine(HeadHash, TailHash);

		return HeadHash;
	}
};

//TODO: local thread cache
template<class TMyKey, class TMyValue>
class TSharedPipelineStateCache
{
public:
	bool Find(const TMyKey& InKey, TMyValue& OutResult)
	{
		auto itr = CurrentMap.find(InKey);
		if (itr != CurrentMap.end())
		{
			OutResult = itr->second;
			return true;
		}
		return false;
	}

	bool Add(const TMyKey& InKey, TMyValue& OutResult)
	{
		return CurrentMap.emplace(InKey, OutResult).second;
	}
private:
	std::unordered_map< TMyKey, TMyValue, PipelineStateCacheKeyFuncs, PipelineStateCacheKeyFuncs> CurrentMap;
};

using FGraphicsPipelineCache = TSharedPipelineStateCache< GraphicsPipelineStateInitializer, PipelineStateCache::FGraphicsPipelineState*>;

FGraphicsPipelineCache GGraphicsPipelineCache;

PipelineStateCache::FGraphicsPipelineState* PipelineStateCache::GetAndOrCreateGraphicsPipelineState(CommandList& cmdlist, const GraphicsPipelineStateInitializer& OriginalInitializer)
{
	const auto Initializer = &OriginalInitializer;

	FGraphicsPipelineState* OutCachedState = nullptr;

	bool bWasFound = GGraphicsPipelineCache.Find(*Initializer, OutCachedState);

	if (bWasFound == false)
	{
		// create new graphics state
		OutCachedState = new FGraphicsPipelineState();

		OutCachedState->IPipeline = leo::share_raw(Context::Instance().GetDevice().CreateGraphicsPipelineState(*Initializer));

		GGraphicsPipelineCache.Add(*Initializer, OutCachedState);
	}

	return OutCachedState;
}
