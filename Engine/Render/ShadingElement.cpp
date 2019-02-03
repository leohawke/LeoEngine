#include <LBase/lmacro.h>
#include "ShadingElement.h"

using namespace LeoEngine::Render;

ImplDeDtor(IShadingElement)

uint32 ShadingElement::ElementId = 0;

ShadingElement::ShadingElement(ShadingElementDataType _type)
	:type(_type)
{
	id = ShadingElement::ElementId++;
}

ShadingElement::~ShadingElement() {

}

