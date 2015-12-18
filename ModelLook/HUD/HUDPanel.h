////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   HUD/HUDPanel.hpp
//  Version:     v1.00
//  Created:     12/12/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: HUD面板
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef HUD_WIDGETCCV_H
#define HUD_WIDGETCCV_H

#include "Widget.h"
#include "HUDControl.h"
#include "HUDContainer.h"
LEO_BEGIN

HUD_BEGIN

class LB_API Panel : public Control, protected MUIContainer
{
public:
	/*!
	\brief 构造：使用指定边界。
	*/
	explicit
	Panel(const Rect& = {});
	DefDeMoveCtor(Panel)

		virtual void
		operator+=(IWidget&);

	virtual bool
		operator-=(IWidget&);

	using MUIContainer::Contains;

	/*!
	\since build 357
	\note 不使用 DefWidgetChildrenGetter 以避免对非必要文件的依赖。
	*/
	DefGetter(override, WidgetRange, Children, WidgetRange(begin(), end()))

		/*!
		\brief 按指定 Z 顺序向部件组添加部件，并设置指针。
		\sa MUIContainer::Add
		\since build 555
		*/
		virtual void
		Add(IWidget&, ZOrder = DefaultZOrder);

	/*!
	\brief 清除内容。

	清除焦点指针和部件组并无效化。
	*/
	void
		ClearContents();

	/*!
	\brief 提升部件至 Z 顺序相等的同组部件的顶端。
	\since build 467

	子部件组中查找指定部件并重新插入至顶端。
	*/
	bool
		MoveToFront(IWidget&);

	//! \since build 496
	using MUIContainer::QueryZ;

	/*!
	\brief 刷新：按指定参数绘制界面并更新状态。
	\since build 294
	*/
	void
		Refresh(PaintEventArgs&&) override;

	//!
	//@{
	using MUIContainer::begin;

	using MUIContainer::end;
	//@}
};

HUD_END

LEO_END


#endif
