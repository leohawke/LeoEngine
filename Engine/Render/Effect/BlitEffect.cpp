#include "BlitEffect.h"

platform::Render::Effect::BlitEffect::BlitEffect(const std::string & name)
	:Effect(name), BilinearCopy(GetTechnique("BilinearCopy")), src_offset(GetParameterRef<leo::math::float3>("src_offset"))
{
}
