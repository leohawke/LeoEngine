#ifndef UI_GUI_h
#define UI_GUI_h

#include "GUIBase.h"

LEO_DRAW_BEGIN

using SPos = platform::unit_type;

struct LB_API Padding
{
	/*
	\brief 空白距离：左、右、上、下。
	\since build 365
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
	\since build 365
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
		\since build 587
		*/
		PDefHOp(Padding&, *=, size_t n)
		// XXX: Conversion to 'SPos' might be implementation-defined.
		ImplRet(lunseq(Left *= SPos(n), Right *= SPos(n), Top *= SPos(n),
			Bottom *= SPos(n)), *this)
};

LEO_DRAW_END


#endif
