#include "CopyEffect.h"

platform::Render::Effect::CopyEffect::CopyEffect(const std::string & name)
	:Effect(name), BilinearCopy(GetTechnique("BilinearCopy"))
{
}
