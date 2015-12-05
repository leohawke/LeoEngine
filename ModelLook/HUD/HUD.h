////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   HUD/HUD.h
//  Version:     v1.00
//  Created:     11/24/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: HUD公共头文件
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef HUD_COMMON_H
#define HUD_COMMON_H


#include "HUDBase.h"

#include<algorithm.hpp>
#include<any_iterator.hpp>

//平台相关接口与数据结构
namespace platform
{
#ifdef PLATFORM_WIN32
	//平台相关的屏幕坐标单位
	using unit_type = long;
	//平台相关的屏幕坐标距离
	using unitlength_type = unsigned long;
#else
#error "unsupprot platform"
#endif
}




LEO_BEGIN

HUD_BEGIN

/*!
\brief 屏幕二维点(直角坐标)
*/
using Point = GBinaryGroup<platform::unit_type>;
using Vec = GBinaryGroup<platform::unit_type>;

/*!
\brief 屏幕区域大小
*/
struct LB_API Size
{
	/*!
	\brief 无效对象
	*/
	static const Size Invalid;

	using PDst = platform::unitlength_type;
	platform::unitlength_type Width, Height;

	/*!
	\brief 无参数构造
	\warning 零初始化。
	*/
	lconstfn Size() lnothrow
		:Width(0),Height(0)
	{}

	lconstfn Size(const Size& s) lnothrow
		:Width(s.Width),Height(s.Height)
	{}

	template<typename Scalar1,typename Scalar2>
	lconstfn Size(Scalar1 w,Scalar2 h) lnothrow
		:Width(static_cast<PDst>(w)),Height(static_cast<PDst>(h))
	{}

	DefDeCopyAssignment(Size)

	lconstfn DefBoolNeg(lconstfn explicit,Width != 0 || Height != 0)

	/*!
	\brief 求与另外一个区域的交
	*/
	PDefHOp(Size&, &=, const Size& s) lnothrow
		ImplRet(lunseq(Width = std::min(Width, s.Width),
			Height = std::min(Height, s.Height)), *this)

		/*!
		\brief 求与另外一个区域的并
		*/
		PDefHOp(Size&, |=, const Size& s) lnothrow
		ImplRet(lunseq(Width = std::max(Width, s.Width),
			Height = std::max(Height, s.Height)), *this)

		/*!
		\brief 判断是否为线段
		*/
		lconstfn DefPred(const lnothrow,LineSegment,!((Width == 0) ^ (Height == 0)))

		lconstfn DefPred(const lnothrow, UnStrictlyEmpty,Width == 0 || Height == 0)
};

lconstfn PDefHOp(bool,==,const Size& x,const Size& y) lnothrow
	ImplRet(x.Width == y.Width && x.Height == y.Height)

lconstfn PDefHOp(bool,!= , const Size& x, const Size& y) lnothrow
	ImplRet(!(x == y))

lconstfn PDefHOp(Size, &, const Size& x, const Size& y) lnothrow
	ImplRet({ std::min(x.Width, y.Width),std::min(x.Height, y.Height) })

lconstfn PDefHOp(Size, | , const Size& x, const Size& y) lnothrow
	ImplRet({ std::max(x.Width, y.Width), std::max(x.Height, y.Height) })

lconstfn PDefH(size_t, GetAreaOf, const Size& s) lnothrow
	ImplRet(size_t(s.Width * s.Height))


template<size_t _vIdx>
lconstfn platform::unitlength_type&
get(Size& s)
{
	static_assert(_vIdx < 2, "Invalid index found.");

	return _vIdx == 0 ? s.Width : s.Height;
}
template<size_t _vIdx>
lconstfn const platform::unitlength_type&
get(const Size& s)
{
	static_assert(_vIdx < 2, "Invalid index found.");

	return _vIdx == 0 ? s.Width : s.Height;
}


template<typename _type>
lconstfn GBinaryGroup<_type>
operator+(GBinaryGroup<_type> val, const Size& s) lnothrow
{
	// XXX: Conversion to '_type' might be implementation-defined.
	return{ val.X + _type(s.Width), val.Y + _type(s.Height) };
}

template<typename _type>
lconstfn GBinaryGroup<_type>
operator-(GBinaryGroup<_type> val, const Size& s) lnothrow
{
	// XXX: Conversion to '_type' might be implementation-defined.
	return{ val.X - _type(s.Width), val.Y - _type(s.Height) };
}

lconstfn PDefH(Size, Transpose, const Size& s) lnothrow
	ImplRet({ s.Height, s.Width })

struct LB_API Rect :private Point, private Size
{
	static const Rect Invalid;
	using Point::X;
	using Point::Y;
	using Size::Width;
	using Size::Height;

	DefDeCtor(Rect)

	explicit lconstfn
	Rect(const Point& pt) lnothrow
		:Point(pt),Size()
	{}

	/*!
	\brief 构造：使用 Size 对象。
	*/
	lconstfn
		Rect(const Size& s) lnothrow
		: Point(), Size(s)
	{}
	/*!
	\brief 构造：使用屏幕二维点和 Size 对象。
	*/
	lconstfn
		Rect(const Point& pt, const Size& s) lnothrow
		: Point(pt), Size(s)
	{}
	/*!
	\brief 构造：使用屏幕二维点和表示长宽的两个 platform::unitlength_type 值。
	*/
	lconstfn
		Rect(const Point& pt, platform::unitlength_type w, platform::unitlength_type h) lnothrow
		: Point(pt.X, pt.Y), Size(w, h)
	{}
	/*!
	\brief 构造：使用表示位置的两个 platform::unit_type 值和 Size 对象。
	*/
	lconstfn
		Rect(platform::unit_type x, platform::unit_type y, const Size& s) lnothrow
		: Point(x, y), Size(s.Width, s.Height)
	{}
	/*!
	\brief 构造：使用表示位置的两个 platform::unit_type 值和表示大小的两个 platform::unitlength_type 值。
	*/
	lconstfn
		Rect(platform::unit_type x, platform::unit_type y, platform::unitlength_type w, platform::unitlength_type h) lnothrow
		: Point(x, y), Size(w, h)
	{}
	/*!
	\brief 复制构造：默认实现。
	*/
	lconstfn DefDeCopyCtor(Rect)

		DefDeCopyAssignment(Rect)
		//! \since build 319
		//@{
		Rect&
		operator=(const Point& pt) lnothrow
	{
		lunseq(X = pt.X, Y = pt.Y);
		return *this;
	}
	Rect&
		operator=(const Size& s) lnothrow
	{
		lunseq(Width = s.Width, Height = s.Height);
		return *this;
	}
	//@}

	/*!
	\brief 求与另一个屏幕标准矩形的交。
	\note 若相离结果为 Rect() ，否则为包含于两个参数中的最大矩形。
	*/
	Rect&
		operator&=(const Rect&) lnothrow;

	/*!
	\brief 求与另一个屏幕标准矩形的并。
	\note 结果为包含两个参数中的最小矩形。
	*/
	Rect&
		operator|=(const Rect&) lnothrow;

	/*!
	\brief 判断是否为空。
	\sa Size::operator!
	*/
	using Size::operator!;

	/*!
	\brief 判断是否非空。
	\sa Size::bool
	*/
	using Size::operator bool;

	/*!
	\brief 判断点 (px, py) 是否在矩形内或边上。
	*/
	bool
		Contains(platform::unit_type px, platform::unit_type py) const lnothrow;
	/*!
	\brief 判断点 pt 是否在矩形内或边上。
	*/
	PDefH(bool, Contains, const Point& pt) const lnothrow
		ImplRet(Contains(pt.X, pt.Y))
		/*!
		\brief 判断矩形是否在矩形内或边上。
		\note 空矩形总是不被包含。
		*/
		bool
		Contains(const Rect&) const lnothrow;
	/*!
	\brief 判断点 (px, py) 是否在矩形内。
	*/
	bool
		ContainsStrict(platform::unit_type px, platform::unit_type py) const lnothrow;
	/*!
	\brief 判断点 pt 是否在矩形内。
	*/
	PDefH(bool, ContainsStrict, const Point& pt) const lnothrow
		ImplRet(ContainsStrict(pt.X, pt.Y))
		/*!
		\brief 判断矩形是否在矩形内或边上。
		\note 空矩形总是不被包含。
		*/
		bool
		ContainsStrict(const Rect&) const lnothrow;
	/*!
	\brief 判断矩形是否为线段：长和宽中有且一个数值等于 0 。
	\sa Size::IsLineSegment
	*/
	using Size::IsLineSegment;

	using Size::IsUnStrictlyEmpty;

	// XXX: Conversion to 'platform::unit_type' might be implementation-defined.
	lconstfn DefGetter(const lnothrow, platform::unit_type, Bottom, Y + platform::unit_type(Height))
		/*!
		\brief 取左上角位置。
		*/
		lconstfn DefGetter(const lnothrow, const Point&, Point,
			static_cast<const Point&>(*this))
		/*!
		\brief 取左上角位置引用。
		*/
		DefGetter(lnothrow, Point&, PointRef, static_cast<Point&>(*this))
		// XXX: Conversion to 'platform::unit_type' might be implementation-defined.
		lconstfn DefGetter(const lnothrow, platform::unit_type, Right, X + platform::unit_type(Width))
		/*!
		\brief 取大小。
		*/
		lconstfn DefGetter(const lnothrow, const Size&, Size,
			static_cast<const Size&>(*this))
		/*!
		\brief 取大小引用。
		*/
		DefGetter(lnothrow, Size&, SizeRef, static_cast<Size&>(*this))

		using Point::GetX;
	using Point::GetY;

	using Point::SetX;
	using Point::SetY;
	//@}
};

struct LB_API Box :public Rect
{

};


struct IWidget;
class AController;

HUD_END

LEO_END

#endif
