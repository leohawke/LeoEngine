%module Engine

%apply void *VOID_INT_PTR { void * }

%include "csharp.swg"
%include "typemaps.i"
%include "LBase\linttype.i"

#define lconstexpr constexpr
#define lnoexcept

%include "..\..\Engine\emacro.h"
%include "..\..\Engine\Render\IFormat.hpp"
%include "..\..\Engine\Render\Color_T.hpp"

%ignore platform::Render::DepthStencilDesc::to_op;
%ignore platform::Render::SamplerDesc::to_op;
%ignore platform::Render::BlendDesc::to_op;
%ignore platform::Render::BlendDesc::to_factor;
%ignore platform::Render::SamplerDesc::to_mode;
%ignore platform::Render::RasterizerDesc::to_mode;
%rename (TexFilterOpElement) platform::Render::details::TexFilterOp;
%rename (RasterizerDescLess) operator<(const RasterizerDesc& lhs, const RasterizerDesc& rhs);
%rename (DepthStencilDescLess) operator<(const DepthStencilDesc& lhs, const DepthStencilDesc& rhs);
%rename (BlendDescLess) operator<(const BlendDesc& lhs, const BlendDesc& rhs);
%rename (SamplerDescLess) operator<(const SamplerDesc& lhs, const SamplerDesc& rhs);
%include "..\..\Engine\Render\PipleState.h"

%ignore operator+(const uint8& lhs,const CubeFaces& face);
%ignore operator-(const uint8& lhs,const CubeFaces& face);
%include "..\..\Engine\Render\ITexture.hpp"

%include "..\..\Engine\Render\IDevice.h"
%include "..\..\Engine\Render\IContext.h"


%{
#include "..\..\Engine\emacro.h"
#include "..\..\Engine\Render\IFormat.hpp"
#include "..\..\Engine\Render\Color_T.hpp"
#include "..\..\Engine\Render\PipleState.h"
#include "..\..\Engine\Render\ITexture.hpp"
#include "..\..\Engine\Render\IContext.h"
#include "..\..\Engine\Render\IDevice.h"
%}






