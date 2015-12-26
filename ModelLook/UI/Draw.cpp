#include "Blit.h"
#include "Draw.h"

namespace leo
{
	namespace Drawing
	{
		void
			FillRect(const Graphics& g, const Rect& r, Color c)
		{
			LAssert(bool(g), "Invalid graphics context found.");
			FillRectRaw(g.GetBufferPtr(), Pixel<>(c), g.GetSize(), r);
		}
	}
}
