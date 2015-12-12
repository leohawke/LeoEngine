////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   HUD/Widget.hpp
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

#include "HUDRenderer.h"

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
/*!
\brief 取渲染器。
\note 引用有效性由派生类约定。
*/
DeclIEntry(HUDRenderer& GetRenderer() const)
EndDecl

//! \relates IWidget
//@{
/*!
\brief 判断部件是否可见。
\since build 259
*/
inline PDefH(bool, IsVisible, const IWidget& wgt)
ImplRet(wgt.IsVisible())

LB_API void SetBox(IWidget&, const Box&);
LB_API void SetBoundsOf(IWidget& wgt, const Rect& r);


/*!
\brief 设置部件左上角所在位置（相对容器的偏移坐标）。
\note 设置视图状态后不会触发 Move 事件。
*/
LB_API void
SetLocationOf(IWidget&, const Point&);

/*!
\brief 设置部件大小。
\note 依次设置渲染器和视图状态，不会触发 Resize 事件。
*/
LB_API void
SetSizeOf(IWidget&, const Size&);

/*!
\ingroup helper_functions
\brief 取部件位置。
*/
inline PDefH(Point, GetLocationOf, const IWidget& wgt)
ImplRet(wgt.GetLocationOf())

/*!
\ingroup helper_functions
\brief 取部件大小。
*/
inline PDefH(Size, GetSizeOf, const IWidget& wgt)
ImplRet(wgt.GetSizeOf())


/*!
\brief 调用指定子部件的 Paint 事件绘制参数指定的事件发送者。
\sa Renderer::Paint

以 e.Sender() 作为绘制目标，判断其边界是否和区域 e.ClipArea 相交，
若相交区域非空则调用 wgt 的渲染器的 Paint 方法绘制 。
调用中， e.Location 被修改为相对子部件的坐标， e.ClipArea 被覆盖为相交区域。
之后， e 可继续被 e.GetSender() 的渲染器的 Paint 方法修改。
*/
LB_API void
PaintChild(IWidget& wgt, PaintEventArgs&& e);
/*!
\brief 调用指定子部件的 Paint 事件绘制指定子部件。
\return 实际绘制的区域。
\note 使用指定子部件作为事件发送者并复制参数。

以 wgt 作为绘制目标，判断其边界是否和区域 pc.ClipArea 相交，
若相交区域非空则新建 PaintEventArgs ，调用 wgt 的渲染器的 Paint 方法绘制 。
*/
LB_API Rect
PaintChild(IWidget& wgt, const PaintContext& pc);

/*!
\brief 调用 PaintChild 指定子部件并提交参数的重绘区域。
*/
LB_API void
PaintChildAndCommit(IWidget&, PaintEventArgs&);

/*!
\brief 调用可见的 PaintChild 指定子部件并提交参数的重绘区域。
*/
inline void
PaintVisibleChildAndCommit(IWidget& wgt, PaintEventArgs& e)
ImplExpr(IsVisible(wgt) ? PaintChildAndCommit(wgt, e) : void())
//@}

class LB_API Widget : implements IWidget
{
private:
	std::shared_ptr<HUDRenderer> renderer_ptr;//渲染器指针。
	std::unique_ptr<AController> controller_ptr;//控制器指针。

public:
	/*!
	\brief 背景。
	*/
	mutable HBrush Background;

	explicit
		Widget(const Rect& = {});
	explicit
		Widget(const Rect&, HBrush);
	/*!
	\brief 构造：使用渲染器指针和控制器指针，无背景。
	\param pRenderer_ 渲染器指针。
	\param pController_ 控制器指针。
	\pre <tt>bool(pRenderer_)</tt> 。
	*/
	template<typename _tRenderer, typename _tController>
	explicit inline
		Widget(const Rect& r = {},_tRenderer&& pRenderer_ = std::make_shared<HUDRenderer>(),
			_tController&& pController_ = {})
		:renderer_ptr(lforward(pRenderer_)),
		controller_ptr(lforward(pController_)), Background()
	{
		LAssertNonnull(renderer_ptr);
		SetBoundsOf(*this, r);
		InitializeEvents();
	}
	/*!
	\brief 复制构造：除容器指针为空外深复制。
	*/
	Widget(const Widget&);
	DefDelMoveCtor(Widget)
	/*!
	\brief 析构：虚实现。

	自动释放焦点后释放部件资源。
	\note 由于不完整类型 WidgetController 的依赖性无法使用 inline 实现。
	*/
	virtual
	~Widget();

private:
	/*!
	\brief 初始化事件组。
	*/
	void InitializeEvents();
public:
	DefGetter(ImplI(IWidget), WidgetRange, Children, WidgetRange())

	/*!
	\brief 取透明画刷。
	\return HUD总是alpha blend。
	\note 为减少包含头文件，使用非 inline 实现。
	\sa SolidBrush
	*/
	static HBrush
	MakeAlphaBrush();
	DefGetter(const ImplI(IWidget), AController&, Controller,
		Deref(controller_ptr))
	DefGetter(const ImplI(IWidget), HUDRenderer&, Renderer, Deref(renderer_ptr))

	DefGetterMem(const lnothrow, platform::unitlength_type, Height, GetSizeOf())
	DefGetterMem(const lnothrow, platform::unitlength_type, Width, GetSizeOf())
	DefGetterMem(const lnothrow, platform::unit_type, X, GetLocationOf())
	DefGetterMem(const lnothrow, platform::unit_type, Y, GetLocationOf())

	DefSetterMem( platform::unitlength_type, Height, GetSizeOf())
	DefSetterMem(platform::unitlength_type, Width, GetSizeOf())
	DefSetterMem(platform::unit_type, X, GetLocationOf())
	DefSetterMem(platform::unit_type, Y, GetLocationOf())

	/*!
	\brief 设置渲染器为指定指针指向的对象，同时更新渲染器状态。
	\note 若指针为空，则使用以当前部件边界新建的 HUDRenderer 对象。
	*/
	void
	SetRenderer(std::shared_ptr<HUDRenderer>);


	/*!
	\brief 刷新：按指定参数绘制界面并更新状态。
	\note 默认按 GetChildren() 得到的迭代器范围绘制可见子部件。
	\sa PaintContext

	由参数指定的信息绘制事件发送者。参数的 ClipArea 成员指定边界。
	边界仅为暗示，允许实现忽略，但应保证调用后边界内的区域保持最新显示状态。
	绘制结束后更新边界，表示实际被绘制的区域。
	若部件的内部状态能够保证显示状态最新，则返回时边界区域可能更小。
	*/
	virtual void
	Refresh(PaintEventArgs&&);
public:
	//无抽象视图造成的冗余
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
