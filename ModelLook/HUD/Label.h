////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   HUD/Label.hpp
//  Version:     v1.00
//  Created:     11/24/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: HUD标签
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef HUD_LABEL_H
#define HUD_LABEL_H

#include "Widget.h"

LEO_BEGIN

HUD_BEGIN
//文本对齐样式
enum class TextAlignment
{
	Left = 1,
	Center = 2,
	Right = 3,
	Up = 4,
	Down = 5
};

class LB_API MLabel {

};

class LB_API Label : public Widget, protected MLabel
{

};

HUD_END

LEO_END


#endif
