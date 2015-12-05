#include <LAssert.h>
#include "Widget.h"
#include "HUDRenderer.h"
#include "HUDControl.h"

LEO_BEGIN

HUD_BEGIN

Rect HUDRenderer::Paint(IWidget & wgt, PaintEventArgs && e)
{
	LAssert(&e.GetSender() == &wgt, "Invalid widget found.");
	if (LB_LIKELY(!e.ClipArea.IsUnStrictlyEmpty()))
		CallEvent<HUD::Paint>(wgt, e);
	return e.ClipArea;
}

HUD_END
LEO_END