#include "Blend.h"


namespace leo
{
	namespace Drawing
	{
		void
			BlendRect(const Graphics& g, const Rect& r, Color c)
		{
			LAssert(bool(g), "Invalid graphics context found.");
			BlendRectRaw(g.GetBufferPtr(), Pixel<>(c), g.GetSize(), r);
		}
	}
}