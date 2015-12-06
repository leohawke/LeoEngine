#include "WidgetEvent.h"

LEO_BEGIN

HUD_BEGIN

ImplDeDtor(AController)

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

HUD_END

LEO_END