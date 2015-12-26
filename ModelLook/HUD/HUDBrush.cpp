#include "HUDBrush.h"
#include "UI/Blit.h"
#include "UI/Draw.h"

void leo::HUD::SolidBrush::operator()(const leo::Drawing::PaintContext & pc) const
{
	LAssert(bool(pc.Target), "Invalid graphics context found.");
	FillRect(pc.Target, pc.ClipArea, Color);
}
