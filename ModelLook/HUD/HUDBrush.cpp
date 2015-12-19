#include "HUDBrush.h"

void leo::HUD::SolidBrush::operator()(const leo::HUD::PaintContext & pc) const
{
	LAssert(bool(pc.Target), "Invalid graphics context found.");
	//FillRect(pc.Target, pc.ClipArea, Color);
}
