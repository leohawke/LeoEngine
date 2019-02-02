#include <LBase/lmacro.h>
#include "ShadingElement.h"

using namespace LeoEngine::Render;

ImplDeDtor(IShadingElement)


ShadingElement::ShadingElement(ShadingElementDataType _type)
	:type(_type)
{
	id = ShadingElement::ElementId++;
}

ShadingElement::~ShadingElement() {

}

