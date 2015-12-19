#ifndef HUD_Brush_H
#define HUD_Brush_H

#include "HUDGraphics.h"
#include <leomathtype.hpp>

LEO_BEGIN

HUD_BEGIN

class LB_API SolidBrush
{
public:
	Pixel<> Color;

	SolidBrush(const float4& c)
		:Color({ static_cast<stdex::byte>(c.x*255),
			static_cast<stdex::byte>(c.y*255),
			static_cast<stdex::byte>(c.z*255),
			static_cast<stdex::byte>(c.w*255) })
	{}

	void
		operator()(const PaintContext&) const;
};

HUD_END

LEO_END

#endif
