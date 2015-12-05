#include "Widget.h"
#include "WidgetEvent.h"

LEO_BEGIN

HUD_BEGIN

UIEventArgs::~UIEventArgs()
{}

HUD::PaintEventArgs::PaintEventArgs(IWidget & wgt)
	:UIEventArgs(wgt),PaintContext()
{}
PaintEventArgs::PaintEventArgs(IWidget & wgt, const PaintContext & pc)
	:UIEventArgs(wgt),PaintContext(pc)
{}
ImplDeDtor(PaintEventArgs)


HUD_END

LEO_END


