#ifndef UI_GUI_h
#define UI_GUI_h

#include "GUIBase.h"
#include "Color.h"

LEO_DRAW_BEGIN

using SPos = platform::unit_type;
using SDst = platform::unitlength_type;

struct LB_API Padding
{
	/*
	\brief 空白距离：左、右、上、下。
	*/
	SPos Left = 0, Right = 0, Top = 0, Bottom = 0;

	/*!
	\brief 默认构造：使用零边距。
	*/
	DefDeCtor(Padding)
		/*!
		\brief 构造：使用 4 个相同的边距。
		*/
		explicit lconstfn
		Padding(SPos x)
		: Padding(x, x, x, x)
	{}
	/*!
	\brief 构造：使用 4 个边距。
	*/
	lconstfn
		Padding(SPos l, SPos r, SPos t, SPos b)
		: Left(l), Right(r), Top(t), Bottom(b)
	{}

	/*!
	\brief 加法赋值：对应分量调用 operator+= 。
	*/
	PDefHOp(Padding&, +=, const Padding& m)
		ImplRet(lunseq(Left += m.Left, Right += m.Right, Top += m.Top,
			Bottom += m.Bottom), *this)

		/*!
		\brief 减法赋值：对应分量调用 operator-= 。
		*/
		PDefHOp(Padding&, -=, const Padding& m)
		ImplRet(lunseq(Left -= m.Left, Right -= m.Right, Top -= m.Top,
			Bottom -= m.Bottom), *this)

		/*!
		\brief 乘法赋值：对应分量调用 operator-= 。
		*/
		PDefHOp(Padding&, *=, size_t n)
		// XXX: Conversion to 'SPos' might be implementation-defined.
		ImplRet(lunseq(Left *= SPos(n), Right *= SPos(n), Top *= SPos(n),
			Bottom *= SPos(n)), *this)
};

//! \relates Padding
//@{
/*!
\brief 加法逆元：对应分量调用一元 operator- 。
*/
lconstfn PDefHOp(Padding, -, const Padding& x)
ImplRet(Padding(-x.Left, -x.Right, -x.Top, -x.Bottom))

/*!
\brief 加法：对应分量调用 operator+ 。
*/
lconstfn PDefHOp(Padding, +, const Padding& x, const Padding& y)
ImplRet(Padding(x.Left + y.Left, x.Right + y.Right, x.Top + y.Top,
	x.Bottom + y.Bottom))

	/*!
	\brief 减法：对应分量调用 operator- 。
	\since build 572
	*/
	lconstfn PDefHOp(Padding, -, const Padding& x, const Padding& y)
	ImplRet(Padding(x.Left - y.Left, x.Right - y.Right, x.Top - y.Top,
		x.Bottom - y.Bottom))

	/*!
	\brief 乘法：对应分量调用 operator* 。
	\since build 587
	*/
	lconstfn PDefHOp(Padding, *, const Padding& x, size_t n)
	ImplRet(Padding(SPos(x.Left * ptrdiff_t(n)), SPos(x.Right * ptrdiff_t(n)),
		SPos(x.Top * ptrdiff_t(n)), SPos(x.Bottom * ptrdiff_t(n))))

	/*!
	\brief 加法：缩小屏幕标准矩形，相对位置由指定边距决定。
	\note 若边距过大，则矩形的宽或高可能为 0 。
	*/
	LB_API Rect
	operator+(const Rect&, const Padding&);

/*!
\brief 减法：放大屏幕标准矩形，相对位置由指定边距决定。
\note 若边距过大，则矩形的宽或高可能为 0 。
*/
inline PDefHOp(Rect, -, const Rect& r, const Padding& m)
ImplRet(r + -m)
//@}


/*!
\brief 取水平边距和。
*/
inline PDefH(SDst, GetHorizontalOf, const Padding& m)
ImplRet(SDst(std::max<SPos>(0, m.Left) + std::max<SPos>(0, m.Right)))

/*!
\brief 取竖直边距和。
*/
inline PDefH(SDst, GetVerticalOf, const Padding& m)
ImplRet(SDst(std::max<SPos>(0, m.Top) + std::max<SPos>(0, m.Bottom)))


/*!
\brief 取内边界相对外边界的边距。
*/
LB_API Padding
FetchMargin(const Rect&, const Size&);


/*!
\brief 根据指定源的边界优化绘制上下文的剪切区域。
\return 若边距决定不足以被渲染则为 Point() ，否则为源的起始偏移位置。
\note 当不需要绘制时，不修改偏移坐标。

检查边距限制下需要保留绘制的区域，结果保存至绘制上下文的除渲染目标外的其它成员。
*/
LB_API Point
ClipBounds(Rect&, const Rect&);

LEO_DRAW_END


#endif
