#ifndef LE_SHADINGPIPELINE_H
#define LE_SHADINGPIPELINE_H 1

#pragma once

#include <LBase/lmacro.h>
#include "../Render/ShadingObject.h"
#include "../Render/ShadingElement.h"

namespace LeoEngine::Render {
	struct lalignas(32)  ShadingItem {
		ShadingObject* shading_object;
		ShadingElement* shading_element;
	};
}

#endif