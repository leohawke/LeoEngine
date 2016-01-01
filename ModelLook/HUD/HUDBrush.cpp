#include "HUDBrush.h"
#include "UI/Blit.h"
#include "UI/Draw.h"
#include "UI/Blend.h"

void leo::HUD::NullBrush::operator()(const leo::Drawing::PaintContext & pc) const
{}

void leo::HUD::SolidBrush::operator()(const leo::Drawing::PaintContext & pc) const
{
	leo::Drawing::FillRect(pc.Target, pc.ClipArea, Color);
}

void leo::HUD::SolidBlendBrush::operator()(const PaintContext & pc) const
{
	leo::Drawing::BlendRect(pc.Target, pc.ClipArea, Color);
}
