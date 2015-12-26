#include "Blit.h"

namespace leo
{
	namespace Drawing
	{
		namespace
		{
			using std::max;
			using std::min;

			inline SDst
				blit_min(SPos s, SPos d)
			{
				return SDst(max<SPos>(max(SPos(), s), s - d));
			}

			inline SPos
				blit_max(SPos s, SPos d, SDst sl, SDst dl, SDst cl)
			{
				// XXX: Conversion to 'SPos' might be implementation-defined.
				return min<SPos>(min<SPos>(SPos(sl), s + SPos(cl)), s + SPos(dl) - d);
			}

		} // unnamed namespace;

	}
}

bool leo::Drawing::BlitBounds(const Point& dp, const Point& sp,
	const Size& ds, const Size& ss, const Size& sc,
	SDst& min_x, SDst& min_y, SDst& delta_x, SDst& delta_y)
{
	SPos max_x, max_y;

	lunseq(min_x = blit_min(sp.X, dp.X), min_y = blit_min(sp.Y, dp.Y),
		max_x = blit_max(sp.X, dp.X, ss.Width, ds.Width, sc.Width),
		max_y = blit_max(sp.Y, dp.Y, ss.Height, ds.Height, sc.Height));
	if (max_x >= 0 && max_y >= 0 && min_x < SDst(max_x) && min_y < SDst(max_y))
	{
		lunseq(delta_x = SDst(max_x) - min_x, delta_y = SDst(max_y) - min_y);
		return true;
	}
	return{};
}
