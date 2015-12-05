//  File name:   HUD/Label.hpp
//  Version:     v1.00
//  Created:     12/1/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 标准部件事件定义

#ifndef HUD_WIDGETEVENT_HPP
#define HUD_WIDGETEVENT_HPP

#include "HUDGraphics.h"
#include "HUDEvent.hpp"

LEO_BEGIN

HUD_BEGIN
	/*!
	\brief 用户界面绘制优先级。
	*/
	//@{
	lconstexpr const EventPriority BackgroundPriority(0xC0);
	lconstexpr const EventPriority BoundaryPriority(0x60);
	lconstexpr const EventPriority ForegroundPriority(0x40);
	//@}

	/*!
	\brief 用户界面事件参数基类。
	*/
	struct LB_API UIEventArgs
	{
	private:
		IWidget* pSender;

	public:
		explicit
			UIEventArgs(IWidget& wgt)
			: pSender(&wgt)
		{}
		/*!
		\brief 复制构造：默认实现。
		\since build 295
		*/
		DefDeCopyCtor(UIEventArgs)
			/*!
			\brief 虚析构：类定义外默认实现。
			\since build 423
			*/
			virtual ~UIEventArgs();

		/*!
		\brief 复制赋值：默认实现。
		\since build 295
		*/
		DefDeCopyAssignment(UIEventArgs)

			DefGetter(const lnothrow, IWidget&, Sender, *pSender)
			PDefH(void, SetSender, IWidget& wgt)
			ImplExpr(pSender = &wgt)
	};

	/*!
	\brief 部件绘制参数。
	*/
	struct LB_API PaintEventArgs : public UIEventArgs,public PaintContext
	{
		PaintEventArgs(IWidget&);
		PaintEventArgs(IWidget&, const PaintContext&);
		DefDeCopyCtor(PaintEventArgs)
			/*!
			\brief 虚析构：类定义外默认实现。
			*/
			~PaintEventArgs() override;
	};

	using HPaintEvent = GHEvent<void(PaintEventArgs&&)>;

HUD_END
LEO_END

#endif
