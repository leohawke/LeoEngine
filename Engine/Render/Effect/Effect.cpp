#include "Effect.hpp"
#include "../IContext.h"

namespace platform::Render::Effect {

	void platform::Render::Effect::Effect::Bind(leo::uint8 index)
	{
		shaders[index].Bind();
	}

	ShaderCompose& Effect::GetShader(leo::uint8 index)
	{
		return shaders[index];
	}

	Pass & platform::Render::Effect::Technique::GetPass(leo::uint8 index)
	{
		return passes[index];
	}
	void Pass::Bind(Effect & effect)
	{
		Context::Instance().Push(state);
		effect.GetShader(bind_index).Bind();
	}
	ShaderCompose& Pass::GetShader(Effect & effect)
	{
		return effect.GetShader(bind_index);
	}
}
