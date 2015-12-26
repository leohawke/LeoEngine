#include "HUDBrush.h"
#include "UI/Blit.h"
#include "UI/Draw.h"

void leo::HUD::SolidBrush::operator()(const leo::Drawing::PaintContext & pc) const
{
	FillRect(pc.Target, pc.ClipArea, Color);
}

void leo::HUD::SolidBlendBrush::operator()(const PaintContext & pc) const
{
	BlendRect(pc.Target, pc.ClipArea, Color);
}
