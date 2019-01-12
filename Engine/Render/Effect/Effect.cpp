#include <LBase/memory.hpp>
#include <LBase/linttype.hpp>
#include <LBase/lmathtype.hpp>
#include <LBase/any.h>
#include <LBase/utility.hpp>

#include <LFramework/LCLib/Debug.h>

#include "../../asset/EffectX.h"
#include "../IContext.h"
#include "../IGraphicsBuffer.hpp"
#include "../ITexture.hpp"

#include "../../Core/AssetResourceScheduler.h"

namespace platform::Render {
	ShaderInfo::ShaderInfo(ShaderCompose::Type t)
		:Type(t)
	{}

	ShaderCompose::~ShaderCompose() = default;
}

namespace platform::Render::Effect {
	Parameter & Parameter::operator=(const leo::any & val)
	{
		static std::unordered_map<EffectParamType, std::function<void(Parameter &, const leo::any &)>> map_fuctors;
		struct InitBlock final
		{
			InitBlock() {
#define REGISTER_MAP_FUNCTOR(enum_type,value_type) map_fuctors[enum_type] = [](Parameter & paramenter, const leo::any & val) { \
				paramenter = leo::any_cast<value_type>(val); \
			}
				REGISTER_MAP_FUNCTOR(asset::EPT_bool, bool);
				REGISTER_MAP_FUNCTOR(asset::EPT_string, std::string);
				REGISTER_MAP_FUNCTOR(asset::EPT_uint, leo::uint32);
				//REGISTER_MAP_FUNCTOR(asset::EPT_uint2, leo::math::uint2);
				//REGISTER_MAP_FUNCTOR(asset::EPT_uint3,
				//REGISTER_MAP_FUNCTOR(asset::EPT_uint4,
				REGISTER_MAP_FUNCTOR(asset::EPT_int, leo::int32);
				//REGISTER_MAP_FUNCTOR(asset::EPT_int2,
				//REGISTER_MAP_FUNCTOR(asset::EPT_int3,
				//REGISTER_MAP_FUNCTOR(asset::EPT_int4,
				REGISTER_MAP_FUNCTOR(asset::EPT_float, float);
				REGISTER_MAP_FUNCTOR(asset::EPT_float2, leo::math::float2);
				//REGISTER_MAP_FUNCTOR(asset::EPT_float2x2
				//REGISTER_MAP_FUNCTOR(asset::EPT_float2x3
				//REGISTER_MAP_FUNCTOR(asset::EPT_float2x4
				REGISTER_MAP_FUNCTOR(asset::EPT_float3, leo::math::float3);
				//REGISTER_MAP_FUNCTOR(asset::EPT_float3x2
				REGISTER_MAP_FUNCTOR(asset::EPT_float3x3, leo::math::float3x3);
				//REGISTER_MAP_FUNCTOR(asset::EPT_float3x4
				REGISTER_MAP_FUNCTOR(asset::EPT_float4, leo::math::float4);
				//REGISTER_MAP_FUNCTOR(asset::EPT_float4x2
				//REGISTER_MAP_FUNCTOR(asset::EPT_float4x3
				REGISTER_MAP_FUNCTOR(asset::EPT_float4x4, leo::math::float4x4);
#undef  REGISTER_MAP_FUNCTOR
			}
		};

		static leo::call_once_init<InitBlock, leo::once_flag> init {};


		if (!var.bind.target)
			var = val;
		else {
			auto search = map_fuctors.find(type);
			if (search != map_fuctors.end())
				std::invoke(search->second,*this, val);
			else
				throw leo::unsupported("the type is not a value class");
		}
		return *this;
	}

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


	//effect will be discard
	Parameter & Effect::GetParameter(const std::string_view & name)
	{
#ifndef NDEBUG
		auto hash = leo::constfn_hash(name);
		auto itr = parameters.find(hash);
		if (itr != parameters.end())
			return itr->second;
		LF_Trace(platform::Descriptions::Warning, "The Effect(%s) ctor pseudo paramter(%s),It's waste memory!", GetName().c_str(), name.data());
		return parameters.emplace(hash,Parameter(name,EffectParamType::EPT_ElemEmpty)).first->second;
#endif
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
		return std::distance(constantbuffs.begin(), std::find_if(constantbuffs.begin(), constantbuffs.end(), [&](const std::shared_ptr<ConstantBuffer>& key) {
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
		LoadAsset(leo::make_observer(EffectAsset.get()));
	}

	Effect::~Effect() {
	}

	void Effect::LoadAsset(leo::observer_ptr<asset::EffectAsset> pEffectAsset)
	{
		std::set<size_t> expect_parameters;
		auto asset_params = pEffectAsset->GetParams();
		for (auto & cbuff : pEffectAsset->GetCBuffersRef()) {
			auto OptionalCBInfo = pEffectAsset->GetInfo<ShaderInfo::ConstantBufferInfo>(cbuff.GetName());
			if (OptionalCBInfo.has_value()) {
				auto ConstantBufferInfo = OptionalCBInfo.value();
				GraphicsBuffer* pGPUBuffer = Context::Instance().GetDevice().CreateConstanBuffer(platform::Render::Buffer::Usage::Dynamic, 0, ConstantBufferInfo.size, EFormat::EF_Unknown);

				auto pConstantBuffer = std::make_shared<ConstantBuffer>(cbuff.GetName(), cbuff.GetNameHash());

				pConstantBuffer->gpu_buffer.reset(pGPUBuffer);
				pConstantBuffer->cpu_buffer.resize(ConstantBufferInfo.size);

				for (auto& param_index : cbuff.GetParamIndices()) {
					expect_parameters.insert(param_index);
					auto& asset_param = asset_params[param_index];
					Parameter Param{ asset_param.GetName(), asset_param.GetNameHash(),asset_param.GetType() };
					//当CB存在时,其param必须存在
					//TODO:支持宏屏蔽
					auto VariableInfo = pEffectAsset->GetInfo<ShaderInfo::ConstantBufferInfo::VariableInfo>(asset_param.GetName()).value();
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
			else {
				auto& param_indices = cbuff.GetParamIndices();
				std::vector<ShaderInfo::ConstantBufferInfo::VariableInfo> pseudo_varinfos;
				std::vector<leo::uint32> pseudo_strides;
				auto cbuff_size = std::accumulate(param_indices.begin(), param_indices.end(), 0, [&](leo::uint32 init, leo::uint32 param_index) {
					auto& asset_param = asset_params[param_index];

					auto value = 0;
					switch (asset_param.GetType()) {
					case asset::EPT_bool:
						value = 1;
						break;
					case asset::EPT_uint:
					case asset::EPT_int:
					case asset::EPT_float:
						value = 4;
						break;
					case asset::EPT_uint2:
					case asset::EPT_int2:
					case asset::EPT_float2:
						value = 8;
						break;
					case asset::EPT_uint3:
					case asset::EPT_int3:
					case asset::EPT_float3:
						value = 12;
						break;
					case asset::EPT_uint4:
					case asset::EPT_int4:
					case asset::EPT_float2x2:
					case asset::EPT_float4:
						value = 16;
						break;
					case asset::EPT_float2x3:
						value = 20;
						break;
					case asset::EPT_float2x4:
					case asset::EPT_float3x2:
						value = 24;
						break;
					case asset::EPT_float4x2:
						value = 32;
						break; 
					case asset::EPT_float3x3:
						value = 36;
						break;
					case asset::EPT_float3x4:
					case asset::EPT_float4x3:
						value = 48;
						break;
					case asset::EPT_float4x4:
						value = 64;
						break;
					default:
						break;
					}
					if(asset_param.GetArraySize() != 0)
						value *= asset_param.GetArraySize();
					uint32 stride;
					if (asset_param.GetArraySize() != 0) {
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

					pseudo_strides.emplace_back(stride);
					
					auto init_ceil16 = (init + 15)&(~15);
					if(init + value > init_ceil16)
						init = init_ceil16;

					ShaderInfo::ConstantBufferInfo::VariableInfo VariableInfo;
					VariableInfo.name = asset_param.GetName();
					VariableInfo.start_offset = init;
					pseudo_varinfos.emplace_back(VariableInfo);

					return init + value;
				});
				GraphicsBuffer* pGPUBuffer = Context::Instance().GetDevice().CreateConstanBuffer(platform::Render::Buffer::Usage::Dynamic, 0, cbuff_size, EFormat::EF_Unknown);

				auto pConstantBuffer = std::make_shared<ConstantBuffer>(cbuff.GetName(), cbuff.GetNameHash());

				pConstantBuffer->gpu_buffer.reset(pGPUBuffer);
				pConstantBuffer->cpu_buffer.resize(cbuff_size);

				auto pseudo_index = 0;
				for (auto& param_index : cbuff.GetParamIndices()) {
					expect_parameters.insert(param_index);
					auto& asset_param = asset_params[param_index];
					Parameter Param{ asset_param.GetName(), asset_param.GetNameHash(),asset_param.GetType() };
					auto VariableInfo = pseudo_varinfos[pseudo_index];
					uint32 stride = pseudo_strides[pseudo_index++];
					Param.var.Bind(pConstantBuffer, VariableInfo.start_offset, stride);
					parameters.emplace(Param.Hash, std::move(Param));

					auto optional_value = pEffectAsset->GetValue(param_index);
					if (optional_value.has_value()) {
						Param = optional_value.value().get();
					}
				}
				constantbuffs.emplace_back(pConstantBuffer);
				LF_Trace(platform::Descriptions::Warning, "The Effect(%s) ctor pseudo constanbuffer[name:%s,size:%d] ,It's waste videomemory!", pEffectAsset->GetName().c_str(),cbuff.GetName().c_str(),cbuff_size);
			}
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

	const std::string& Effect::GetName() const lnothrow {
		return Name;
	}

	EffectsHolder::EffectsHolder()
		:loaded_effects(&pool_resource)
	{
	}

	EffectsHolder::~EffectsHolder()
	{
	}

	std::shared_ptr<void> EffectsHolder::FindResource(const leo::any& key) {
		if (auto pstr = leo::any_cast<std::string>(&key))
			return FindResource(*pstr);
		if (auto passet = leo::any_cast<std::shared_ptr<asset::EffectAsset>>(&key))
			return FindResource(*passet);
		return {};
	}
	std::shared_ptr<Effect> EffectsHolder::FindResource(const std::string& name) {
		for(auto& pair : loaded_effects)
		{
			if (pair.second->GetName() == name)
				return pair.second;
			if (auto sp = pair.first.lock())
				if (sp->GetName() == name)
					return pair.second;
		}
		return {};
	}

	std::shared_ptr<Effect> EffectsHolder::FindResource(const std::shared_ptr<asset::EffectAsset>& asset) {
		return FindResource(asset->GetName());//Name reslove
	}

	void EffectsHolder::Connect(const std::shared_ptr<asset::EffectAsset>& asset, const std::shared_ptr<Effect>& effect)
	{
		loaded_effects.emplace_back(asset, effect);
	}

	EffectsHolder& EffectsHolder::Instance() {
		static EffectsHolder instance;
		return instance;
	}
}

namespace platform {
	using namespace Render::Effect;

	template<>
	std::shared_ptr<Effect> AssetResourceScheduler::SyncSpawnResource<Effect,const std::string&>(const std::string& name) {
		if (auto pEffect = EffectsHolder::Instance().FindResource(name))
			return pEffect;
		auto pAsset = X::LoadEffectAsset(name + ".lsl");
		auto pEffect = std::make_shared<Effect>(name);
		EffectsHolder::Instance().Connect(pAsset, pEffect);
		return pEffect;
	}

	template std::shared_ptr<Effect> AssetResourceScheduler::SyncSpawnResource<Effect, const std::string&>(const std::string& name);
}


