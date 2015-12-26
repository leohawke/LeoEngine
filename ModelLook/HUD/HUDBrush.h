#ifndef HUD_Brush_H
#define HUD_Brush_H

#include "HUDBase.h"
#include <leomathtype.hpp>

LEO_BEGIN

HUD_BEGIN

class LB_API SolidBrush
{
public:
	Drawing::Color Color;

	SolidBrush(const Drawing::Color c)
		:Color(c)
	{}

	void
		operator()(const PaintContext&) const;
};

HUD_END

LEO_END

#endif
