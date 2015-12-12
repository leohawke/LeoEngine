
#include "HUDHostRenderer.h"

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
	if(!window->IsMinimized())
	{
		//AdjustSize();

		auto& wgt(widget.get());
		const auto& g(GetContext());
		const auto r(GetInvalidatedArea());

		if (Validate(wgt, wgt, { g,{},r }))
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


