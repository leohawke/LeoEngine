#ifndef LE_SHADINGPIPELINE_H
#define LE_SHADINGPIPELINE_H 1

#pragma once

#include <LBase/lmacro.h>
#include "../Render/ShadingObject.h"
#include "../Render/ShadingElement.h"
#include "../Render/Effect/Effect.hpp"

namespace LeoEngine::Render {
	struct lalignas(32)  ShadingItem {
		ShadingObject* shading_object;
		ShadingElement* shading_element;
		RenderEffect::EffectItem effect_item;
	};
}

#endif