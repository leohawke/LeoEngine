#include "BiltEffect.h"

platform::Render::Effect::BiltEffect::BiltEffect(const std::string & name)
	:Effect(name), BlitLinear2D(GetTechnique("BlitLinear2D")),
	src_offset(GetParameterRef<leo::math::float3>("src_offset")),
	src_scale(GetParameterRef<leo::math::float3>("src_scale")),
	src_level(GetParameterRef<int>("src_level"))
{
}
