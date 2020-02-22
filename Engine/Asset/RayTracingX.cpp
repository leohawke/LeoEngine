#include "RayTracingX.h"
#include "ShaderAsset.h"
#include "../Core/AssetResourceScheduler.h"
#include "ShaderLoadingDesc.h"

using namespace platform;
using namespace asset;

class RayTracingShaderAsset :public asset::ShadersAsset,public asset::AssetName
{

};

class RayTracingShaderLoadingDesc : public asset::AssetLoading<RayTracingShaderAsset>, public X::ShaderLoadingDesc<RayTracingShaderAsset> {
private:
	using Super = X::ShaderLoadingDesc<RayTracingShaderAsset>;
public:
	explicit RayTracingShaderLoadingDesc(X::path const& shaderpath)
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

		return nullptr;
	}

	std::shared_ptr<AssetType> CreateAsset()
	{
		return ReturnValue();
	}
};


std::shared_ptr<Render::RayTracingShader> platform::X::LoadRayTracingShader(Render::RayDevice& Device, const path& filepath)
{
	auto pAsset = AssetResourceScheduler::Instance().SyncLoad<RayTracingShaderLoadingDesc>(filepath);

	return std::shared_ptr<Render::RayTracingShader>();
}
