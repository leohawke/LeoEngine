#include "HUDControl.h"

LEO_BEGIN

HUD_BEGIN

ImplDeDtor(AController)
EventMapping::ItemType&
Controller::GetItemRef(VisualEvent id, EventMapping::MappedType(&f)()) const
{
	return GetEvent(EventMap, id, f);
}

WidgetController::WidgetController(bool b)
	: AController(b),
	Paint()
{}

EventMapping::ItemType&
WidgetController::GetItem(VisualEvent id) const
{
	if (id == VisualEvent::Paint)
		return Paint;
	throw BadEvent();
}

Control::ControlEventMap::ControlEventMap()
{
}

Control::Control(const Rect& r)
	:Widget(r,new HUDRenderer(),new Controller(true, parameterize_static_object<const ControlEventMap>()))
{}

Control::Control(const Rect & r, HBrush b)
	:Control(r)
{
	Background = b;
}

Control::Control(const Control & ctl)
	:Widget(ctl)
{
}

ImplDeDtor(Control)

HUD_END

LEO_END


