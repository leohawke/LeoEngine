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
#ifndef HUD_WIDGET_H
#define HUD_WIDGET_H

#include "WidgetEvent.h"

LEO_BEGIN

HUD_BEGIN

using WidgetIterator = any_input_iterator<IWidget>;
using WidgetRange = std::pair<WidgetIterator, WidgetIterator>;

DeclI(LB_API, IWidget)
DeclIEntry(WidgetRange GetChildren())

/*
视图接口<未抽象视图>
*/
DeclIEntry(bool IsVisible() const) //判断是否可见
DeclIEntry(bool Contains(platform::unit_type, platform::unit_type) const) //判断点是否在部件的可视区域内
DeclIEntry(Size GetSizeOf() const) //取部件大小
DeclIEntry(Box GetBox() const) //取部件包围盒
DeclIEntry(Point GetLocationOf() const) //取部件大小

DeclIEntry(void SetLocationOf(const Point&)) //设置部件左上角位置
DeclIEntry(void SetSizeOf(const Size&)) //设置部件大小
DeclIEntry(void SetVisible(bool b)) //设置部件可见性

/*!
\brief 取控制器。
\note 引用有效性由派生类约定。
*/
DeclIEntry(AController& GetController() const)
EndDecl

LB_API void SetBox(IWidget&, const Box&);

class LB_API Widget : implements IWidget
{
public:
	Widget() = default;
	~Widget() = default;

	DefIEntryImpl(bool IsVisible() const)
	DefIEntryImpl(bool Contains(platform::unit_type, platform::unit_type) const) 
	DefIEntryImpl(Size GetSizeOf() const) 
	DefIEntryImpl(Box GetBox() const) 
	DefIEntryImpl(Point GetLocationOf() const) 

	DefIEntryImpl(void SetLocationOf(const Point&)) 
	DefIEntryImpl(void SetSizeOf(const Size&)) 
	DefIEntryImpl(void SetVisible(bool b))
private:
	Size mSize;
	Point mLocation;
	bool mVisible;
};


HUD_END

LEO_END


#endif
