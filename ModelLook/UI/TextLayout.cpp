#pragma warning(push)
#pragma warning(disable:4244)

#include "TextLayout.h"

LEO_DRAW_BEGIN

using std::max;

SDst
FetchResizedBottomMargin(const TextState& ts, SDst h)
{
	LAssert(GetTextLineHeightExOf(ts) != 0, "Zero line height found.");
	return SDst(max<SPos>(0, ts.Margin.Bottom)) + (h + ts.LineGap
		- GetVerticalOf(ts.Margin)) % GetTextLineHeightExOf(ts);
}

std::uint16_t
FetchResizedLineN(const TextState& ts, SDst h)
{
	LAssert(GetTextLineHeightExOf(ts) != 0, "Zero line height found.");
	return (h + ts.LineGap - GetVerticalOf(ts.Margin))
		/ GetTextLineHeightExOf(ts);
}

SPos
FetchLastLineBasePosition(const TextState& ts, SDst h)
{
	const auto n(FetchResizedLineN(ts, h));

	// XXX: Conversion to 'SPos' might be implementation-defined.
	return GetTextLineBaseOf(ts) + SPos(GetTextLineHeightExOf(ts))
		* SPos(n > 0 ? n - 1 : n);
	//	return h - ts.Margin.Bottom + ts.GetCache().GetDescender() + 1;
}

 SDst FetchCharWidth(const Font & fnt, ucs4_t c)
{
	// TODO: Support negtive horizontal advance.
	return CheckNonnegativeScalar<SDst>(fnt.GetAdvance(c, fnt.GetGlyph(c)));
}

LEO_DRAW_END

#pragma warning(pop)
