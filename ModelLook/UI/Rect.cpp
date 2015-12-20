#include "GUIBase.h"

LEO_DRAW_BEGIN

using std::max;
using std::min;

const Size Size::Invalid(std::numeric_limits<platform::unitlength_type>::lowest(),
	std::numeric_limits<platform::unitlength_type>::lowest());
const Rect Rect::Invalid(Size::Invalid);

using PPos = platform::unit_type;
using PDst = platform::unitlength_type;

namespace
{
	bool
		RectContainsRaw(const Rect& r, PPos px, PPos py) lnothrow
	{
		LAssert(r.Width > 0, "Invalid width found."),
		LAssert(r.Height > 0, "Invalid height found.");

		// XXX: Conversion to 'PPos' might be implementation-defined.
		return IsInInterval<PPos>(px - r.X, PPos(r.Width))
			&& IsInInterval<PPos>(py - r.Y, PPos(r.Height));
	}
	inline bool
		RectContainsRaw(const Rect& r, const Point& pt) lnothrow
	{
		return RectContainsRaw(r, pt.X, pt.Y);
	}

	bool
		RectContainsStrictRaw(const Rect& r, PPos px, PPos py) lnothrow
	{
		LAssert(r.Width > 1, "Invalid width found."),
			LAssert(r.Height > 1, "Invalid height found.");
		// XXX: Conversion to 'PPos' might be implementation-defined.
		return IsInOpenInterval<PPos>(px - r.X, PPos(r.Width - 1))
			&& IsInOpenInterval<PPos>(py - r.Y, PPos(r.Height - 1));
	}
	inline bool
		RectContainsStrictRaw(const Rect& r, const Point& pt) lnothrow
	{
		return RectContainsStrictRaw(r, pt.X, pt.Y);
	}
}

bool
Rect::Contains(PPos px, PPos py) const lnothrow
{
	return !IsUnStrictlyEmpty() && RectContainsRaw(*this, px, py);
}
bool
Rect::Contains(const Rect& r) const lnothrow
{
	return !IsUnStrictlyEmpty() && RectContainsRaw(*this, r.GetPoint())
		&& RectContainsRaw(*this, r.GetPoint() + r.GetSize() - Vec(1, 1));
}

bool
Rect::ContainsStrict(PPos px, PPos py) const lnothrow
{
	return Width > 1 && Height > 1 && RectContainsStrictRaw(*this, px, py);
}
bool
Rect::ContainsStrict(const Rect& r) const lnothrow
{
	return Width > 1 && Height > 1 && !r.IsUnStrictlyEmpty()
		&& RectContainsStrictRaw(*this, r.GetPoint())
		&& RectContainsStrictRaw(*this, r.GetPoint() + r.GetSize() - Vec(1, 1));
}

Rect&
Rect::operator&=(const Rect& r) lnothrow
{
	const PPos x1(max(X, r.X)), x2(min(GetRight(), r.GetRight())),
		y1(max(Y, r.Y)), y2(min(GetBottom(), r.GetBottom()));

	return *this = x2 < x1 || y2 < y1 ? Rect()
		: Rect(x1, y1, PDst(x2 - x1), PDst(y2 - y1));
}

Rect&
Rect::operator|=(const Rect& r) lnothrow
{
	if (!*this)
		return *this = r;
	if (!r)
		return *this;

	const PPos mx(min(X, r.X)), my(min(Y, r.Y));

	return *this = Rect(mx, my, PDst(max(GetRight(), r.GetRight())
		- mx), PDst(max(GetBottom(), r.GetBottom()) - my));
}

LEO_DRAW_END