#ifndef UI_COLOR_H
#define UI_COLOR_H

#include "UI.h"

namespace leo {
	template<size_t _vX, size_t _vY, size_t _vZ, size_t _vA>
	struct XYZATrait
	{
		static lconstexpr const size_t ABitsN = _vA;
		static lconstexpr const size_t XBitsN = _vX;
		static lconstexpr const size_t YBitsN = _vY;
		static lconstexpr const size_t ZBitsN = _vZ;
		static lconstexpr const size_t XYBitsN = XBitsN + YBitsN;
		static lconstexpr const size_t XYZBitsN = XBitsN + YBitsN + ZBitsN;
		static lconstexpr const size_t BitsN = XBitsN + YBitsN + ZBitsN + ABitsN;
		static lconstexpr const size_t BytesN = (BitsN + CHAR_BIT - 1) / CHAR_BIT;

		using AType = typename make_width_int<ABitsN>::unsigned_least_type;
		using BType = typename make_width_int<XBitsN>::unsigned_least_type;
		using GType = typename make_width_int<YBitsN>::unsigned_least_type;
		using RType = typename make_width_int<ZBitsN>::unsigned_least_type;
		using IntegerType
			= typename make_width_int<BitsN>::unsigned_least_type;
		using ArrayType = stdex::byte[BytesN];

		static_assert(BitsN <= 64, "Width larger than 64 unimplemented");
	};

	template<size_t _vX, size_t _vY, size_t _vZ, size_t _vA>
	struct XYZAMaskTrait
	{
		using IntegerType = typename XYZATrait<_vX, _vY, _vZ, _vA>::IntegerType;

		static lconstexpr const IntegerType XMaskN = _vX;
		static lconstexpr const IntegerType XYMaskN = XMaskN + _vY;
		static lconstexpr const IntegerType XYZMaskN = XYMaskN + _vZ;
		static lconstexpr const IntegerType AMask = ((1U << _vA) - 1) << XYZMaskN;
		static lconstexpr const IntegerType XMask = (1U << _vX) - 1;
		static lconstexpr const IntegerType YMask = ((1U << _vY) - 1) << XMaskN;
		static lconstexpr const IntegerType ZMask = ((1U << _vZ) - 1) << XYMaskN;
	};
}


LEO_DRAW_BEGIN

using AlphaType = stdex::byte;

/*!
\warning 为兼容YSLIB存在,兼容DXGI_FORMAT格式
*/
enum PixelFormat {
	C48888,//4个通道，每通道8bit
};
template<PixelFormat = C48888>
union Pixel;

template<>
union lalignas(lalignof(typename XYZATrait<8,8,8,8>::IntegerType)) Pixel<C48888> {
	static lconstexpr const size_t _vB = 8;
	static lconstexpr const size_t _vG = 8;
	static lconstexpr const size_t _vR = 8;
	static lconstexpr const size_t _vA = 8;

	using Trait = XYZATrait<_vB, _vG, _vR, _vA>;
	//! \since build 507
	using MaskTrait = XYZAMaskTrait<_vB, _vG, _vR, _vA>;
	//! \since build 555
	using ArrayType = typename Trait::ArrayType;
	//! \since build 555
	using IntegerType = typename Trait::IntegerType;

	ArrayType Bytes;
	IntegerType Integer;

	DefDeDtor(Pixel)
		lconstfn
		Pixel(typename Trait::IntegerType i)
		: Integer(i)
	{}
	lconstfn
		Pixel(typename Trait::BType b, typename Trait::GType g,
			typename Trait::RType r, typename Trait::AType a)
		: Integer(IntegerType(b) | IntegerType(g) << MaskTrait::XMaskN
			| IntegerType(r) << MaskTrait::XYMaskN
			| IntegerType(a) << MaskTrait::XYZMaskN)
	{}

	lconstfn DefCvt(const lnothrow, typename Trait::IntegerType, Integer)

		lconstfn DefGetter(const lnothrow, typename Trait::AType, A,
			(Integer & MaskTrait::AMask) >> MaskTrait::XYZMaskN)
		lconstfn DefGetter(const lnothrow, typename Trait::BType, B,
			Integer & MaskTrait::XMask)
		lconstfn DefGetter(const lnothrow, typename Trait::GType, G,
			(Integer & MaskTrait::YMask) >> MaskTrait::XMaskN)
		lconstfn DefGetter(const lnothrow, typename Trait::RType, R,
			(Integer & MaskTrait::ZMask) >> MaskTrait::XYMaskN)
};

/*!
\brief 取像素 Alpha 值。
*/
lconstfn PDefH(AlphaType, FetchAlpha, Pixel<> px) lnothrow
ImplRet(px.GetA())

/*!
\brief 取不透明像素。
\relates Pixel
*/
lconstfn PDefH(Pixel<>, FetchOpaque, Pixel<> px) lnothrow
ImplRet({ px.GetB(), px.GetG(), px.GetR(), 0xFF })


lconstfn PDefH(std::uint32_t, FetchPixel,
	stdex::byte r, stdex::byte g, stdex::byte b) lnothrow
	ImplRet(std::uint32_t(r) | std::uint32_t(g) << 8 | std::uint32_t(b) << 16)

#define LL_FetchPixel(r, g, b) FetchPixel(r, g, b)

#define DefColorH_(hex, name) \
	name = (LL_FetchPixel(((hex) >> 16) & 0xFF, \
		((hex) >> 8) & 0xFF, (hex) & 0xFF) << 8 | 0xFF)

//! \brief 系统默认颜色空间。
namespace ColorSpace
{
#define	HexAdd0x(hex) 0x##hex
#define DefColorH(hex_, name) DefColorH_(HexAdd0x(hex_), name)

	/*!
	\brief 默认颜色集。
	\see http://www.w3schools.com/html/html_colornames.asp 。
	*/
	enum ColorSet : uint32
	{
		DefColorH(00FFFF, Aqua),
		DefColorH(000000, Black),
		DefColorH(0000FF, Blue),
		DefColorH(FF00FF, Fuchsia),
		DefColorH(808080, Gray),
		DefColorH(008000, Green),
		DefColorH(00FF00, Lime),
		DefColorH(800000, Maroon),
		DefColorH(000080, Navy),
		DefColorH(808000, Olive),
		DefColorH(800080, Purple),
		DefColorH(FF0000, Red),
		DefColorH(C0C0C0, Silver),
		DefColorH(008080, Teal),
		DefColorH(FFFFFF, White),
		DefColorH(FFFF00, Yellow)
	};

#undef LL_FetchPixel
#undef DefColorH
#undef DefColorH_
#undef HexAdd0x
} // namespace ColorSpace;


  //! \brief 颜色。
class LB_API Color
{
public:
	using ColorSet = ColorSpace::ColorSet;

private:
	/*!
	\brief RGB 分量。
	*/
	stdex::byte r = 0, g = 0, b = 0;
	/*!
	\brief Alpha 分量。
	*/
	stdex::byte a = 0;

public:
	/*!
	\brief 无参数构造：所有分量为 0 的默认颜色。
	*/
	lconstfn
		Color() = default;
	/*!
	\brief 构造：使用本机颜色对象。
	*/
	lconstfn
		Color(Pixel<> px) lnothrow
		: r(px.GetR()), g(px.GetG()), b(px.GetB()), a(px.GetA())
	{}
	/*!
	\brief 构造：使用默认颜色。
	*/
	lconstfn
		Color(ColorSet cs) lnothrow
		: r((cs & 0xFF00) >> 8), g((cs & 0xFF0000) >> 16),
		b((cs & 0xFF000000) >> 24), a(0xFF)

	{}
	/*!
	\brief 构造：使用 RGB 值和 alpha 位。
	*/
	lconstfn
		Color(stdex::byte r_, stdex::byte g_, stdex::byte b_, AlphaType a_ = 0xFF) lnothrow
		: r(r_), g(g_), b(b_), a(a_)
	{}
	/*!
	\brief 构造：使用相同类型转换为单色的 RGB 值和 alpha位。
	\note 避免列表初始化时 narrowing 转换。
	*/
	template<typename _tScalar>
	lconstfn
		Color(_tScalar r_, _tScalar g_, _tScalar b_, AlphaType a_ = 0xFF) lnothrow
		: Color(stdex::byte(r_), stdex::byte(g_), stdex::byte(b_), a_)
	{}

	/*!
	\brief 转换：本机颜色对象。
	*/
	lconstfn
		operator Pixel<>() const lnothrow
	{
		return{ b, g, r, a };
	}

	/*!
	\brief 取 alpha 分量。
	*/
	lconstfn DefGetter(const lnothrow, stdex::byte, A, a)
		/*!
		\brief 取蓝色分量。
		*/
		lconstfn DefGetter(const lnothrow, stdex::byte, B, b)
		/*!
		\brief 取绿色分量。
		*/
		lconstfn DefGetter(const lnothrow, stdex::byte, G, g)
		/*!
		\brief 取红色分量。
		*/
		lconstfn DefGetter(const lnothrow, stdex::byte, R, r)
};

namespace ColorSpace {
	//透明
	lconstfn Color Trans = {0,0,0,0};
}

LEO_DRAW_END

#endif
