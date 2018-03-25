#include "../../asset/EffectX.h"
#include "../IContext.h"
#include "../IGraphicsBuffer.hpp"
#include "../ITexture.hpp"

#include <LFramework/LCLib/Debug.h>
#include <LBase/memory.hpp>
namespace platform::Render {
	ShaderInfo::ShaderInfo(ShaderCompose::Type t)
		:Type(t)
	{}

	ShaderCompose::~ShaderCompose() = default;
}

namespace platform::Render::Effect {


	const Pass & platform::Render::Effect::Technique::GetPass(leo::uint8 index) const
	{
		return passes[index];
	}

	Pass & platform::Render::Effect::Technique::GetPass(leo::uint8 index)
	{
		return passes[index];
	}
	void Pass::Bind(const Effect & effect) const
	{
		Context::Instance().Push(Deref(state));
		effect.GetShader(bind_index).Bind();
	}
	void Pass::UnBind(const Effect & effect) const
	{
		effect.GetShader(bind_index).UnBind();
	}
	ShaderCompose& Pass::GetShader(const Effect & effect) const
	{
		return effect.GetShader(bind_index);
	}
	const PipleState & Pass::GetState() const
	{
		return Deref(state);
	}

	void ConstantBuffer::Update() {
		if (dirty) {
			gpu_buffer->UpdateSubresource(0, static_cast<leo::uint32>(cpu_buffer.size()), cpu_buffer.data());
			dirty = false;
		}
	}

	GraphicsBuffer* ConstantBuffer::GetGraphicsBuffer() const lnothrow {
		return gpu_buffer.get();
	}
}

namespace platform::Render::Effect {


	void platform::Render::Effect::Effect::Bind(leo::uint8 index)
	{
		shaders[index]->Bind();
	}

	ShaderCompose& Effect::GetShader(leo::uint8 index) const
	{
		return *shaders[index];
	}

	const Technique & Effect::GetTechnique(const std::string & name) const
	{
		auto hash = leo::constfn_hash(name);
		return GetTechnique(hash);
	}

	const Technique & Effect::GetTechnique(size_t hash) const
	{
		return Deref(std::find_if(techniques.begin(), techniques.end(), [&](const NameKey& key) {
			return key.Hash == hash;
		}));
	}

	const Technique & Effect::GetTechniqueByIndex(size_t index) const
	{
		return techniques[index];
	}

	Parameter & Effect::GetParameter(const std::string & name)
	{
		return GetParameter(leo::constfn_hash(name));
	}

	Parameter & Effect::GetParameter(const std::string_view & name)
	{
		return GetParameter(leo::constfn_hash(name));
	}

	Parameter & Effect::GetParameter(size_t hash)
	{
		return parameters.find(hash)->second;
	}
	

	ConstantBuffer & Effect::GetConstantBuffer(size_t index)
	{
		return Deref(constantbuffs[index]);
	}

	size_t Effect::ConstantBufferIndex(const std::string& name) {
		auto hash = leo::constfn_hash(name);
		return ConstantBufferIndex(hash);
	}
	size_t Effect::ConstantBufferIndex(size_t hash) {
		return std::distance(constantbuffs.begin(),std::find_if(constantbuffs.begin(), constantbuffs.end(), [&](const std::shared_ptr<ConstantBuffer>& key) {
			return key->Hash == hash;
		}));
	}

	Technique & Effect::GetTechnique(const std::string & name)
	{
		auto hash = leo::constfn_hash(name);
		return Deref(std::find_if(techniques.begin(), techniques.end(), [&](const NameKey& key) {
			return key.Hash == hash;
		}));
	}
}

namespace platform::Render::Effect {
	Effect::Effect(const std::string & name)
		:NameKey(name)
	{
		//TODO Name Rule
		auto EffectAsset = X::LoadEffectAsset(name + ".lsl");
		LoadAsset(leo::make_observer(&EffectAsset));
	}

	Effect::~Effect() {
	}

	void Effect::LoadAsset(leo::observer_ptr<asset::EffectAsset> pEffectAsset)
	{
		std::set<size_t> expect_parameters;
		auto asset_params = pEffectAsset->GetParams();
		for (auto & cbuff : pEffectAsset->GetCBuffersRef()) {
			auto ConstantBufferInfo = leo::any_cast<ShaderInfo::ConstantBufferInfo>(pEffectAsset->GetInfo(cbuff.GetName()).value());
			GraphicsBuffer* pGPUBuffer = Context::Instance().GetDevice().CreateConstanBuffer(platform::Render::Buffer::Usage::Dynamic, 0, ConstantBufferInfo.size, EFormat::EF_Unknown);

			auto pConstantBuffer = std::make_shared<ConstantBuffer>(cbuff.GetName(), cbuff.GetNameHash());

			pConstantBuffer->gpu_buffer.reset(pGPUBuffer);
			pConstantBuffer->cpu_buffer.resize(ConstantBufferInfo.size);

			for (auto& param_index : cbuff.GetParamIndices()) {
				expect_parameters.insert(param_index);
				auto& asset_param = asset_params[param_index];
				Parameter Param{ asset_param.GetName(), asset_param.GetNameHash(),asset_param.GetType() };
				auto VariableInfo = leo::any_cast<ShaderInfo::ConstantBufferInfo::VariableInfo>(pEffectAsset->GetInfo(asset_param.GetName()).value());
				uint32 stride;
				if (VariableInfo.elements > 0) {
					if (asset_param.GetType() == asset::EPT_float4x4)
						stride = 64;
					else
						stride = 16;
				}
				else {
					if (asset_param.GetType() == asset::EPT_float4x4)
						stride = 16;
					else
						stride = 4;
				}
				Param.var.Bind(pConstantBuffer, VariableInfo.start_offset, stride);//Depend reflect info
				parameters.emplace(Param.Hash, std::move(Param));

				//stride 必须正确！
				//Design TODO [Material Also Have Values!]
				auto optional_value = pEffectAsset->GetValue(param_index);
				if (optional_value.has_value()) {
					Param = optional_value.value().get();
				}
			}

			constantbuffs.emplace_back(pConstantBuffer);
		}

		//other param
		for (size_t i = 0; i != asset_params.size(); ++i) {
			if (expect_parameters.find(i) != expect_parameters.end())
				continue;

			auto& param = asset_params[i];
			Parameter Param{ param.GetName(), param.GetNameHash(),param.GetType() };

			//其他类型的value不需要类型转换
			auto optional_value = pEffectAsset->GetValue(i);
			if (optional_value.has_value())
				Param.var.value = optional_value.value();

			parameters.emplace(Param.Hash, std::move(Param));
		}

		auto& asset_techns = pEffectAsset->GetTechniquesRef();
		for (auto& asset_tech : asset_techns) {
			Technique technique = { asset_tech.GetName(),asset_tech.GetNameHash() };

			auto asset_passes = asset_tech.GetPasses();
			for (auto& asset_pass : asset_passes) {
				Pass pass{};

				auto& asset_blob_hashs = asset_pass.GetBlobs();
				std::unordered_map<ShaderCompose::Type, leo::observer_ptr<const asset::ShaderBlobAsset>> asset_blobs;
				for (auto&pair : asset_blob_hashs) {
					asset_blobs.emplace(pair.first, &pEffectAsset->GetBlob(pair.second));
				}
				ShaderCompose* pCompose = Context::Instance().GetDevice().CreateShaderCompose(asset_blobs, leo::make_observer(this));
				pass.bind_index = static_cast<leo::uint8>(shaders.size());
				shaders.emplace_back(pCompose);
				pass.state = leo::unique_raw(Context::Instance().GetDevice().CreatePipleState(asset_pass.GetPipleState()));
				technique.passes.emplace_back(std::move(pass));
			}
			techniques.emplace_back(std::move(technique));
		}
	}
}


