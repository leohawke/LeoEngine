%import "LBase\linttype.i"


%{
#include "..\..\Engine\Render\IGraphicsBuffer.hpp"
#include "..\..\LBase\linttype.hpp"
using namespace leo::inttype;
%}

%ignore LE_RENDER_IGraphicsBuffer_hpp;
%ignore platform::Render::Buffer::Mapper;

%ignore platform::Render::GraphicsBuffer::HWResourceCreate;
%ignore platform::Render::GraphicsBuffer::HWResourceDelete;
%ignore platform::Render::GraphicsBuffer::UpdateSubresource;

%import "LBase\ldef.i"
%import "LBase\lmacro.i"

%include "..\..\Engine\Render\IGraphicsBuffer.hpp"
