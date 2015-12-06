#include "Font.hpp"

LEO_DRAW_BEGIN

Font::Font(FontSize s,
	FontStyle fs)
	:font_size(s), style(fs)
{}

void
Font::SetSize(FontSize s)
{
	if (LB_LIKELY(s >= MinimalSize && s <= MaximalSize))
		font_size = s;
}
bool
Font::SetStyle(FontStyle fs)
{
	style = fs;
	return true;
}

LEO_DRAW_END