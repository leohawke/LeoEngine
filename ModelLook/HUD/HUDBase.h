#ifndef HUD_Base_H
#define HUD_Base_H

#include <ldef.h>
#include <BaseMacro.h>
#include <LAssert.h>
#include <exception.hpp>
#include <limits>

#include "..\UI\GUI.h"

#ifndef HUD_BEGIN
#define HUD_BEGIN namespace HUD {
#define HUD_END }
#endif

LEO_BEGIN
HUD_BEGIN

using Drawing::Size;
using Drawing::Rect;
using Drawing::Point;
using Drawing::Box;
using Drawing::Graphics;
using Drawing::PaintContext;
using Drawing::IImage;


HUD_END
LEO_END

#endif