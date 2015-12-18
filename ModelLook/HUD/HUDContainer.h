#ifndef HUD_Container_H
#define HUD_Container_H

#include "Widget.h"

LEO_BEGIN
HUD_BEGIN

//@{
//! \brief Z 顺序类型：覆盖顺序，值越大表示越接近顶层。
using ZOrder = std::uint8_t;
//! \brief 默认 Z 顺序值。
const ZOrder DefaultZOrder(64);
//! \brief 默认窗口 Z 顺序值。
const ZOrder DefaultWindowZOrder(128);
//@}

class LB_API MUIContainer {
public:
	/*!
	\brief 部件组项目类型。
	*/
	using ItemType = lref<IWidget>;
	/*!
	\brief 部件映射表类型：映射 Z 顺序至部件。
	*/
	using WidgetMap = std::multimap<ZOrder, ItemType>;
	using PairType = WidgetMap::value_type;
	using iterator = WidgetIterator;

protected:
	/*
	\brief 部件映射：存储 Z 顺序映射至部件引用。
	*/
	WidgetMap mWidgets;

public:
	//@{
	/*!
	\brief 无参数构造：默认实现。
	*/
	DefDeCtor(MUIContainer)
	DefDeMoveCtor(MUIContainer)

	/*!
	\brief 向部件组添加部件。
	\note 部件已存在时忽略。

	向部件组按默认 Z 顺序值添加部件。
	*/
	PDefHOp(void, +=, IWidget& wgt)
	ImplRet(Add(wgt))

	/*!
	\brief 从部件组移除部件。
	\return 存在指定部件且移除成功。

	从部件组移除部件。
	*/
	bool
	operator-=(IWidget&);
	//@}

	/*!
	\brief 判断是否包含指定部件。
	*/
	bool
		Contains(IWidget&);

	/*!
	\brief 取部件数。
	*/
	DefGetter(const lnothrow, size_t, Count, mWidgets.size())

		/*!
		\brief 向部件组添加部件。
		\note 部件已存在时忽略。

		向焦点对象组添加焦点对象，同时向部件组按指定 Z 顺序值添加部件。
		*/
		void
		Add(IWidget&, ZOrder = DefaultZOrder);

	/*!
	\brief 绘制可视子部件。
	*/
	void
		PaintVisibleChildren(PaintEventArgs&);

	/*!
	\brief 查询指定部件的 Z 顺序。
	\throw std::out_of_range 不包含指定子部件。
	*/
	ZOrder
		QueryZ(IWidget&) const;

	iterator
		begin();

	PDefH(void, clear, ) lnothrow
		ImplExpr(mWidgets.clear())

		iterator end();
};

LB_API bool
RemoveFrom(IWidget& wgt, IWidget& con);

HUD_END

LEO_END

#endif
