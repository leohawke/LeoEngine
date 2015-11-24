////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   HUD/HUD.h
//  Version:     v1.00
//  Created:     11/24/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: HUD公共头文件
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef HUD_COMMON_H
#define HUD_COMMON_H

#include<ldef.h>
#include<BaseMacro.h>
#include<any_iterator.hpp>

namespace platform
{
#ifdef PLATFORM_WIN32
	using unit_type = long;
	using unitlength_type = unsigned long;
#else
#error "unsupprot platform"
#endif
}


#ifndef HUD_BEGIN
#define HUD_BEGIN namespace HUD {
#define HUD_END }
#endif

LEO_BEGIN

HUD_BEGIN

using Point = std::pair<platform::unit_type, platform::unit_type>;

struct LB_API Size:std::pair<platform::unitlength_type,platform::unitlength_type>
{
};

struct LB_API Rect :private Point, private Size
{

};

struct LB_API Box :private Rect
{

};

HUD_END

LEO_END

#endif
