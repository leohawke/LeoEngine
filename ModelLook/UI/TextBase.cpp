#include "TextBase.h"

using namespace leo::Drawing;

TextState::TextState(const leo::Drawing::Font &font, const Padding &m)
	:PenStyle(font),Margin(m)
{
}

leo::Drawing::TextState::TextState(FontCache &, const Padding & m)
	: TextState(leo::Drawing::Font(),m)
{
}

void
TextState::PutNewline()
{
	CarriageReturn(*this);
	// XXX: Conversion to 'SPos' might be implementation-defined.
	Pen.Y += SPos(GetTextLineHeightExOf(*this));
}

void
TextState::ResetPen()
{
	//	Pen.Y = Margin.Top + GetTextLineHeightExOf(*this);
	//	Pen.Y = Margin.Top + pCache->GetAscender();
	CarriageReturn(*this),
		Pen.Y = GetTextLineBaseOf(*this);
}
void
TextState::ResetPen(const Point& pt, const Padding& m)
{
	Pen = Point(pt.X + m.Left, pt.Y + Font.GetAscender() + m.Top);
}
