#include "../../asset/EffectX.h"
#include "../IContext.h"

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
		auto hash = std::hash<std::string>()(name);
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
	}
}
