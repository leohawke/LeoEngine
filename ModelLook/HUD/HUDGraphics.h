#ifndef HUD_Graphics_H
#define HUD_Graphics_H

#include "HUD.h"

LEO_BEGIN


//2D纹理包装
//warning，一般不会直接使用该类型,using std::share_ptr<texture2d_wrapper>
//warning, HUD的格式一定是R8G8B8A8(not A8B8...)，DYNAMIC,NOMIP,ARRAYSIZE = 1
//warning, R8G8B8A8,某些显卡效果可能不对
class texture2d_wrapper;

HUD_BEGIN

using BitmapPtr = Drawing::Pixel<>*;
using ConstBitmapPtr = const Drawing::Pixel<>*;

template<typename _tOut, typename _tIn>
void
CopyBitmapBuffer(_tOut p_dst, _tIn p_src, const Size& s)
{
	std::copy_n(Nonnull(p_src), GetAreaOf(s), Nonnull(p_dst));
}

/*!
\brief 二维图形接口上下文模板。
\warning 为兼容YSLIB存在
*/
template<typename _tPointer, class _tSize = Size>
class GGraphics
{
	static_assert(std::is_nothrow_move_constructible<_tPointer>::value,
		"Invalid pointer type found.");
	static_assert(std::is_nothrow_copy_constructible<_tSize>::value,
		"Invalid size type found.");

public:
	using PointerType = _tPointer;
	using SizeType = _tSize;
	using PixelType = std::decay_t<decltype(PointerType()[0])>;

protected:
	/*!
	\brief 显示缓冲区指针。
	\warning 除非 PointerType 自身为具有所有权的智能指针，不应视为具有所有权。
	*/
	PointerType pBuffer{};
	//! \brief 图形区域大小。
	SizeType sGraphics{};

public:
	virtual ~GGraphics()
	{}

	//! \brief 默认构造：使用空指针和大小。
	DefDeCtor(GGraphics)
		//! \brief 构造：使用指定位图指针和大小。
		explicit lconstfn
		GGraphics(PointerType p_buf, const SizeType& s = {}) lnothrow
		: pBuffer(std::move(p_buf)), sGraphics(s)
	{}
	//! \brief 构造：使用其它类型的指定位图指针和大小。
	template<typename _tPointer2, class _tSize2>
	explicit lconstfn
		GGraphics(_tPointer2 p_buf, const _tSize2& s = {}) lnothrow
		: GGraphics(static_cast<PointerType>(std::move(p_buf)),
			static_cast<SizeType>(s))
	{}
	//! \brief 构造：使用其它类型的指定位图指针和大小类型的二维图形接口上下文。
	template<typename _tPointer2, class _tSize2>
	lconstfn
		GGraphics(const GGraphics<_tPointer2, _tSize2>& g) lnothrow
		: GGraphics(std::move(g.GetBufferPtr()), g.GetSize())
	{}
	//! \brief 复制构造：浅复制。
	DefDeCopyCtor(GGraphics)
		DefDeMoveCtor(GGraphics)

		DefDeCopyAssignment(GGraphics)
		DefDeMoveAssignment(GGraphics)

		/*!
		\brief 判断无效或有效性。
		\since build 319
		*/
		DefBoolNeg(explicit,
			bool(pBuffer) && sGraphics.Width != 0 && sGraphics.Height != 0)

		/*!
		\brief 取指定行首元素指针。
		\pre 断言：参数不越界。
		\pre 间接断言：缓冲区指针非空。
		*/
		PointerType
		operator[](size_t r) const lnothrow
	{
		LAssert(r < sGraphics.Height, "Access out of range.");
		return Nonnull(pBuffer) + r * sGraphics.Width;
	}

	//! \since build 566
	DefGetter(const lnothrow, const PointerType&, BufferPtr, pBuffer)
		DefGetter(const lnothrow, const SizeType&, Size, sGraphics)
		//! \since build 196
		DefGetter(const lnothrow, platform::unitlength_type, Width, sGraphics.Width)
		//! \since build 196
		DefGetter(const lnothrow, platform::unitlength_type, Height, sGraphics.Height)
		//! \since build 177
		DefGetter(const lnothrow, size_t, SizeOfBuffer,
			sizeof(PixelType) * size_t(GetAreaOf(sGraphics))) //!< 取缓冲区占用空间。

															  /*!
															  \brief 取指定行首元素指针。
															  \throw GeneralEvent 缓冲区指针为空。
															  \throw std::out_of_range 参数越界。
															  */
		PointerType
		at(size_t r) const
	{
		if (LB_UNLIKELY(!pBuffer))
			throw general_event("Null pointer found.");
		if (LB_UNLIKELY(r >= sGraphics.Height))
			throw std::out_of_range("Access out of range.");
		return pBuffer + r * sGraphics.Width;
	}
};

using ConstGraphics = GGraphics<ConstBitmapPtr>;
using Graphics = GGraphics<BitmapPtr>;

/*!
\brief 图像接口。
\warning 该类底层实现一定是一个leo::HUD::details::hud_tex_wrapper对象
*/
DeclDerivedI(LB_API, IImage,cloneable)
DeclIEntry(std::unique_ptr<Graphics> GetContext() const lnothrow)
DeclIEntry(void SetSize(const Size&))

DeclIEntry(IImage* clone() const ImplI(cloneable))
EndDecl


/*
\brief 绘制上下文。
\warning 非虚析构。
*/
struct LB_API PaintContext
{
	//todo:modify it
	//maybe GDI impl
	Graphics Target; //!< 渲染目标：图形接口上下文。
	/*!
	\brief 参考位置。

	指定渲染目标关联的参考点的位置的偏移坐标。
	除非另行约定，选取渲染目标左上角为原点的屏幕坐标系。
	*/
	Point Location;
	/*!
	\brief 剪切区域。

	相对图形接口上下文的标准矩形，指定需要保证被刷新的边界区域。
	除非另行约定，剪切区域的位置坐标选取渲染目标左上角为原点的屏幕坐标系。
	*/
	Rect ClipArea;
};


/*!
\brief 根据指定边距和源的大小优化绘制上下文的剪切区域。
\return 若边距决定不足以被渲染则为 Point() ，否则为源的起始偏移位置。
\note 当不需要绘制时，不修改偏移坐标。

检查边距限制下需要保留绘制的区域，结果保存至绘制上下文的除渲染目标外的其它成员。
*/
LB_API Point
ClipMargin(PaintContext&, const Drawing::Padding&, const Size&);

HUD_END

LEO_END

#endif
