%import "IFormat.i"

%{
#include "..\..\Engine\Render\DeviceCaps.h"
#include "..\..\LBase\linttype.hpp"
using namespace leo::inttype;
%}

%ignore LE_RENDER_DeviceCaps_h;
%ignore platform::Render::Caps::TextureFormatSupport;
%ignore platform::Render::Caps::VertexFormatSupport;
%ignore platform::Render::Caps::RenderTargetMSAASupport;

%include "..\..\Engine\Render\DeviceCaps.h"

