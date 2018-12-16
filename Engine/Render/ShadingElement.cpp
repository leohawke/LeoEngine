#include <LBase/lmacro.h>
#include "ShadingElement.h"

using namespace LeoEngine::Render;

ImplDeDtor(IShadingElement)

ShadingElement::ShadingElement() {
	id = ShadingElement::ElementId++;
}

ShadingElement::~ShadingElement() {

}

