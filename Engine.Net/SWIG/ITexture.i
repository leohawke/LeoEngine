%import "LBase\linttype.i"

%{
#include "..\..\LBase\linttype.hpp"
using namespace leo::inttype;
#include "..\..\Engine\Render\ITexture.hpp"
using namespace platform::Render::IFormat;
%}

%import "LBase\ldef.i"
%import "LBase\lmacro.i"
%import "..\..\Engine\emacro.h"
%import "IFormat.i"

%ignore operator+(const uint8& lhs,const CubeFaces& face);
%ignore operator-(const uint8& lhs,const CubeFaces& face);

%ignore platform::Render::Mapper;


%ignore platform::Render::Texture::HWResourceCreate;
%ignore platform::Render::Texture::HWResourceDelete;
%ignore platform::Render::Texture::HWResourceReady;

%rename(GetTextureType) platform::Render::Texture::GetType;
%include "..\..\Engine\Render\ITexture.hpp"