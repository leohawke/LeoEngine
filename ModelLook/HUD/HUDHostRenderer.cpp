
#include "HUDHostRenderer.h"
#include "Widget.h"

LEO_BEGIN
HUD_BEGIN

HostRenderer::~HostRenderer()
{
}

void HostRenderer::SetSize(const Size & s)
{
	BufferedRenderer::SetSize(s);
	window->Resize({ static_cast<uint16>(s.GetWidth()),static_cast<uint16>(s.GetHeight())});
}

void HostRenderer::Render()
{
	if(!window->IsMined())
	{
		auto& wgt(widget.get());

 		CommitInvalidation(wgt.GetBox());
		//AdjustSize();

		auto b = false;
		{
			
			//context must have this local scope
			const auto g(GetContext());
			const auto r(GetInvalidatedArea());

			b = bool(Validate(wgt, wgt, { *g,{},r }));
		}
		if(b)
			window->Render();
	}
}

void HostRenderer::InitWidgetView()
{
	//Converte widget to a WidgetCCV
	//donothing
}

HUD_END
LEO_END


