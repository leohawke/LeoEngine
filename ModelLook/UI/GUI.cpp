#include "GUI.h"

LEO_DRAW_BEGIN

Rect
operator+(const Rect& r, const Padding& m)
{
	// XXX: Conversion to 'SPos' might be implementation-defined.
	return Rect(r.X + m.Left, r.Y + m.Top,
		SDst(std::max<SPos>(0, SPos(r.Width) - m.Left - m.Right)),
		SDst(std::max<SPos>(0, SPos(r.Height) - m.Top - m.Bottom)));
}


Padding
FetchMargin(const Rect& r, const Size& s)
{
	// XXX: Conversion to 'SPos' might be implementation-defined.
	return Padding(r.X, SPos(s.Width) - r.X - SPos(r.Width),
		r.Y, SPos(s.Height) - r.Y - SPos(r.Height));
}


Point
ClipBounds(Rect& clip, const Rect& bounds)
{
	if (!clip.IsUnStrictlyEmpty() && Clip(clip, bounds))
		return clip.GetPoint() - bounds.GetPoint();
	clip.GetSizeRef() = {};
	return{};
}

LEO_DRAW_END
