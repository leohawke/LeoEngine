#include "../../asset/EffectX.h"
#include "../IContext.h"

namespace platform::Render {
	ShaderInfo::ShaderInfo(ShaderCompose::Type t)
		:Type(t)
	{}

}

namespace platform::Render::Effect {
	

	Pass & platform::Render::Effect::Technique::GetPass(leo::uint8 index)
	{
		return passes[index];
	}
	void Pass::Bind(Effect & effect)
	{
		Context::Instance().Push(state);
		effect.GetShader(bind_index).Bind();
	}
	void Pass::UnBind(Effect & effect)
	{
		effect.GetShader(bind_index).UnBind();
	}
	ShaderCompose& Pass::GetShader(Effect & effect)
	{
		return effect.GetShader(bind_index);
	}
	PipleState & Pass::GetState()
	{
		return state;
	}
}

namespace platform::Render::Effect {
	

	void platform::Render::Effect::Effect::Bind(leo::uint8 index)
	{
		shaders[index]->Bind();
	}

	ShaderCompose& Effect::GetShader(leo::uint8 index)
	{
		return *shaders[index];
	}

	const Technique & Effect::GetTechnique(const std::string & name) const
	{
		auto hash =leo::constfn_hash(name);
		return GetTechnique(hash);
	}

	const Technique & Effect::GetTechnique(size_t hash) const
	{
		return *std::find_if(techniques.begin(), techniques.end(), [&](const NameKey& key) {
			return key.Hash == hash;
		});
	}

	const Technique & Effect::GetTechniqueByIndex(size_t index) const
	{
		return techniques[index];
	}

	Parameter & Effect::GetParameter(const std::string & name)
	{
		return GetParameter(std::hash<std::string>()(name));
	}

	Parameter & Effect::GetParameter(size_t hash)
	{
		return parameters.find(hash)->second;
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

	void Effect::LoadAsset(leo::observer_ptr<asset::EffectAsset> pEffectAsset)
	{
		std::set<size_t> expect_parameters;
		auto params = pEffectAsset->GetParams();
		for (auto & cbuff : pEffectAsset->GetCBuffersRef()) {
			//Create GPU Buffer Depend reflect info
			GraphicsBuffer* pGPUBuffer = nullptr;
			
			auto pConstantBuffer = std::make_shared<ConstantBuffer>(cbuff.GetName(), cbuff.GetNameHash());
			
			//pConstantBuffer->gpu_buffer.reset(pGPUBuffer);
			pConstantBuffer->cpu_buffer.resize(2048);

			for (auto& param_index : cbuff.GetParamIndices()) {
				expect_parameters.insert(param_index);
				auto& param = params[param_index];
				Parameter Param { param.GetName(), param.GetNameHash() };
				Param.var.Bind(pConstantBuffer, 0, 16);//Depend reflect info
				parameters.emplace(Param.Hash, std::move(Param));\

				//TODO bind values
			}
		}

		//other param
		for (size_t i = 0; i != params.size(); ++i) {
			if (expect_parameters.count(i) > 0)
				continue;

			auto& param = params[i];
			Parameter Param{ param.GetName(), param.GetNameHash() };
			parameters.emplace(Param.Hash, std::move(Param)); \

			//TODO bind values
		}

		//create shadercompose depend reflect
	}
}


