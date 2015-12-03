////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   HUD/HUDRenderer.h
//  Version:     v1.00
//  Created:     11/29/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: HUD部件渲染器
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef HUD_RENDERER_H
#define HUD_RENDERER_H

#include <utility.hpp>
#include "WidgetEvent.h"

LEO_BEGIN
HUD_BEGIN

/*!
\brief HUD部件渲染器
\note 无状态
*/
class LB_API HUDRenderer {
public:
	DefDeCtor(HUDRenderer)
	DefDeCopyMoveCtorAssignment(HUDRenderer)
	DefVrDtor(HUDRenderer)
	/*!
	\brief 设置缓冲区大小
	\note  空实现。
	*/
	virtual void SetSize(const Size&)
	{}

	/*
	\brief 按参数绘制部件
	*/
	virtual Rect Paint(IWidget& wgt,PaintEventArgs&&);
};

/*!
\brief 伪渲染器
*/
class LB_API HUDPseudoRenderer : public HUDRenderer
{
	~HUDPseudoRenderer() override;
	PDefH(Rect,Paint,IWidget&,PaintEventArgs&&) override
		ImplRet({})
};

/*!
\brief 渲染到D3D RT对象上的渲染器
\note D3D RT对象被引擎管理
*/
class LB_API HUDD3DRTRenderer:public HUDRenderer
{
public:
	/*!
	\brief 设置缓冲区大小。
	*/
	void SetSize(const Size&) override;

	/*
	\brief 按参数绘制部件
	*/
	Rect Paint(IWidget& wgt, PaintEventArgs&&) override;
};

HUD_END
LEO_END

#endif
