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
