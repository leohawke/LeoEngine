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
class LB_API HUDRenderer : public cloneable {
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

	DefClone(const ImplI(cloneable), HUDRenderer)

	/*!
	\brief 提交无效区域。
	*/
	virtual PDefH(Rect, CommitInvalidation, const Rect& r)
	ImplRet(r)

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
public:
	DefDeCtor(HUDPseudoRenderer)
	DefDeCopyMoveCtorAssignment(HUDPseudoRenderer)

	~HUDPseudoRenderer() override;

	DefClone(const ImplI(cloneable), HUDPseudoRenderer)

	PDefH(Rect,Paint,IWidget&,PaintEventArgs&&) override
		ImplRet({})
};

/*!
\brief 渲染到D3D RT对象上的渲染器
\note D3D RT对象被引擎管理
\note PaintContext 包好 绘制命令?
*/
class LB_API BufferedRenderer :public HUDRenderer
{
protected:
	//!<无效区域:包含所有新绘制请求的区域(GDI实现有效)。
	mutable Rect rInvalidated;

	/*!
	\brief RT对象
	*/
	std::shared_ptr<IImage> pImageBuffer;
public:
	/*!
	\brief 指定验证时忽略上层缓冲区背景。
	*/
	bool IgnoreBackground = true;

	/*!
	\brief 构造：指定是否忽略上层缓冲区背景。
	\note 当指针为空时新建缓冲区。
	*/
	BufferedRenderer(bool = {},std::shared_ptr<IImage> = {});
	//note 会copy一份IImage
	BufferedRenderer(const BufferedRenderer&);
	DefDeMoveCtor(BufferedRenderer)

	/*!
	\brief 判断是否需要刷新。
	\note 若无效区域长宽都不为零，则需要刷新。
	*/
	bool
	RequiresRefresh() const;

	DefGetter(const lnothrow, IImage&, ImageBuffer, *pImageBuffer)
	/*!
	\brief 取无效区域。
	*/
	DefGetter(const lnothrow, const Rect&, InvalidatedArea, rInvalidated)
	/*!
	\brief 取图形接口上下文。
	\warning 非GDI实现可能为空。
	\return 缓冲区图形接口上下文。
	*/
	DefGetterMem(const lnothrow, Graphics, Context, GetImageBuffer())

	/*!
	\brief 设置缓冲区大小。
	\warning 可能导致原缓冲区指针失效。
	*/
	void
	SetSize(const Size&) override;
	void
	SetImageBuffer(std::shared_ptr<IImage>,bool clone = false);

	DefClone(const override, BufferedRenderer)

	/*!
	\brief 提交无效区域，使之合并至现有无效区域中。
	\return 新的无效区域。
	\note 由于无效区域的形状限制，可能会存在部分有效区域被合并。
	*/
	Rect
	CommitInvalidation(const Rect&) override;

	/*!
	\brief 按参数绘制。
	\pre 间接断言： <tt>&e.GetSender().GetRenderer() == this</tt> 。
	\note 在 Validate 后 Update 。
	\note 不检查部件可见性。
	*/
	Rect Paint(IWidget& wgt, PaintEventArgs&&) override;

	/*!
	\brief 更新至指定图形设备上下文的指定点。
	\note 复制显示缓冲区内容。
	*/
	void
	UpdateTo(const PaintContext&) const;

	/*!
	\brief 验证并按需绘制。
	\pre 断言： <tt>&sender.GetRenderer() == this</tt> 。
	\return 验证中被刷新的区域边界。

	验证 sender 的指定图形接口上下文的关联的缓冲区，
	若存在无效区域则新建 PaintEventArgs ， 调用 wgt 的 Paint 事件绘制。
	*/
	Rect
	Validate(IWidget& wgt, IWidget& sender, const PaintContext&);
};

HUD_END
LEO_END

#endif
