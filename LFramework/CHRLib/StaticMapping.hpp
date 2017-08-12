/*!	\file StaticMapping.hpp
\ingroup CHRLib
\brief 静态编码映射。
\version r2436
\author FrankHB <frankhb1989@gmail.com>
\par 创建时间:
2015-12-29 19:47:21 +0800
\par 修改时间:
2016-06-19 leohawke port to vs2015
\par 模块名称:
CHRLib::StaticMapping
*/


#pragma warning(push)
#pragma warning(disable:4244)

#ifndef LBase_CHRLib_StaticMapping_hpp
#define LBase_CHRLib_StaticMapping_hpp 1
#include "LBase/cstdio.h"
#include "LBase/ref.hpp"
#include "LBase/any.h"

#include <LFramework/CHRLib/CharacterMapping.h>
#include <utility>

namespace CHRLib
{

	using leo::remove_reference_t;

template<typename _tIn>
using GuardPair = std::pair<leo::lref<_tIn>, const _tIn>;


template<typename _tIn>
inline bool
CheckIterator(_tIn& i)
{
	static_assert(std::is_constructible<const byte, decltype(*i)>(),
		"Invalid mapping source type found.");
	using leo::is_undereferenceable;

	return !is_undereferenceable(i);
}

//! \brief 以输入迭代器指向内容填充有效输入迭代器指定的字节。
//@{
template<typename _tIn>
inline bool
FillByte(_tIn& i, byte& b)
{
	if(CheckIterator(i))
	{
		const auto r = byte(*i);

		lunseq(++i, b = r);
		return true;
	}
	return {};
}

template<typename _tIn, typename _tState>
inline bool
FillByte(_tIn& i, _tState& st)
{
	if(CheckIterator(i))
	{
		static_assert(!std::is_volatile<remove_reference_t<_tState>>(),
			"Volatile state is not supported.");
		const auto r = byte(*i);

		lunseq(++i, GetSequenceOf(st)[GetIndexOf(st)++] = r);
		return true;
	}
	return {};
}

//@{
template<typename _tIn>
inline bool
FillByte(GuardPair<_tIn>& pr, byte& b)
{
	return pr.first != pr.second ? FillByte(pr.first.get(), b) : false;
}
template<typename _tIn, typename _tState>
inline bool
FillByte(GuardPair<_tIn>& pr, _tState& st)
{
	return pr.first != pr.second ? FillByte(pr.first.get(), st) : false;
}
//@}
//@}


//@{
//! \brief 判断八元组是否组成 UTF-16 代理对起始代码的高 8 位。
lconstfn PDefH(bool, IsSurrogateHead, octet x) lnothrow
	ImplRet(x >= 0xD8 && x < 0xE0)

//! \brief 判断八元组是否组成 UTF-16 有效代理对起始代码的高 8 位。
lconstfn PDefH(bool, IsValidSurrogateHead, octet x) lnothrow
	ImplRet(x >= 0xD8 && x < 0xDE)

//! \brief 判断八元组是否组成 UTF-16 有效代理对终止代码的高 8 位。
lconstfn PDefH(bool, IsValidSurrogateTail, octet x) lnothrow
	ImplRet(x >= 0xDC && x < 0xE0)


//! \brief 提供编码映射公共操作默认实现的基类。
struct UCSMapperBase
{
	static void
	Assign(leo::pseudo_output, ucs4_t) lnothrow
	{}
	template<typename _tObj>
	static void
	Assign(_tObj& uc, ucs4_t c) lnothrow
	{
		lnoexcept_assert("Invalid type found", uc = _tObj(c));

		uc = _tObj(c);
	}
};


/*!
\brief 静态编码操作模板及 Unicode 编码特化。
\note 使用 ADL FillByte 填充字节。
\note 使用 ADL GetIndexOf 和 GetSequenceOf 操作状态。
\sa UCSMapperBase

提供编码转换的映射模板。
假定通过外部指定具体字符类型的中间状态为 Unicode 代码点。
默认使用小端序（即 UCS-2LE 或 UCS-4LE ），对应字符类型 ucs2_t 和 ucs4_t 。
注意保留超出基本多文种平面的代码点需要使用范围足够大的字符类型。
若使用 ucs2_t ，对越界部分不保证能保留正确的结果，取决于中间状态的字符赋值时的转换。
一般接口（静态成员函数模板）如下：
IsInvalid ：判断八元组是否被禁止的接口。
Decode ：解码操作：从指定编码的字符序列映射到中间状态的字符。
Encode ：编码操作：从中间状态的字符逆向映射到指定编码的字符序列。
解码操作允许接受 leo::pseudo_object 忽略输出。
使用模板参数决定状态类型中，状态索引等于已解码的输入字节数；否则不予保证。
*/
//@{
template<Encoding>
class GUCSMapper
{};

/*!
\warning 使用 UCS-2 时， 4 字节代码点可能溢出。
\see 参考规范： RFC 3629 ，见 http://tools.ietf.org/html/rfc3629 。
*/
template<>
struct GUCSMapper<CharSet::UTF_8> : UCSMapperBase
{
	using UCSMapperBase::Assign;

private:
	template<typename _tObj, typename _tSeq>
	static void
	Assign1(_tObj& uc, const _tSeq& seq) lnothrow
	{
		lnoexcept_assert("Invalid type found", Assign(uc, ucs4_t()));

		Assign(uc, (ucs4_t(seq[0] & 0x1CU) >> 2U << 8U)
			| (ucs4_t(seq[0] & 0x03U) << 6) | (ucs4_t(seq[1]) & 0x3FU));
	}

	template<typename _tObj, typename _tSeq>
	static void
	Assign2(_tObj& uc, const _tSeq& seq) lnothrow
	{
		lnoexcept_assert("Invalid type found", Assign(uc, ucs4_t()));

		Assign(uc, (((ucs4_t(seq[0]) & 0x0FU) << 4U
			| ucs4_t(seq[1] & 0x3CU) >> 2U) << 8U)
			| (ucs4_t(seq[1] & 0x3U) << 6U) | ucs4_t(seq[2] & 0x3FU));
	}

	static lconstfn bool
	CheckInvalid(octet x, octet y = 0x80) lnothrow
	{
		return IsInvalid(x) || ((x & 0xC0) != y);
	}

public:
	/*!
	\brief 检查 UTF-8 文本序列中非法字节。
	\note 包括： C0 、 C1 、 F5 至 FF 。
	*/
	static lconstfn bool
	IsInvalid(octet x) lnothrow
	{
		return x == 0xC0 || x == 0xC1 || x > 0xF4;
	}

	/*!
	\brief 解码： UTF-8 。
	\since build 599
	*/
	template<typename _tObj, typename _tIn, typename _tState>
	static ConversionResult
	Decode(_tObj&& uc, _tIn&& i, _tState&& st)
		lnoexcept(noexcept(GetSequenceOf(st)) && noexcept(GetIndexOf(st))
		&& noexcept(!FillByte(i, st)))
	{
		const auto seq(GetSequenceOf(st));

		switch(GetIndexOf(st))
		{
		case 0:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
			if((seq[0] & 0x80) == 0)
			{
				Assign(uc, seq[0]);
				break;
			}
			if(LB_UNLIKELY(CheckInvalid(seq[0], 0xC0)))
				return ConversionResult::Invalid;
		case 1:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
			if(LB_UNLIKELY(CheckInvalid(seq[1])))
				return ConversionResult::Invalid;
			if(((seq[0] ^ 0xC0) & 0xE0) == 0)
			{
				Assign1(uc, seq);
				break;
			}
		case 2:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
			if(LB_UNLIKELY(CheckInvalid(seq[2])))
				return ConversionResult::Invalid;
			if(((seq[0] ^ 0xE0) & 0xF0) == 0)
			{
				Assign2(uc, seq);
				break;
			}
		case 3:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
			if(LB_UNLIKELY(CheckInvalid(seq[3])))
				return ConversionResult::Invalid;
			if(LB_LIKELY(((seq[0] ^ 0xF0) & 0xF8) == 0))
			{
				Assign2(uc, seq);
				break;
			}
			return ConversionResult::Unhandled;
		default:
			return ConversionResult::BadState;
		}
		return ConversionResult::OK;
	}
	/*!
	\brief 快速解码： UTF-8 。
	\see https://github.com/miloyip/rapidjson/blob/master/include/rapidjson/encodings.h
	*/
	//@{
	template<typename _tObj, typename _tIn>
	static ConversionResult
	Decode(_tObj&& uc, _tIn&& i, ConversionState& st)
		lnoexcept_spec(!FillByte(i, st))
	{
		byte b(0);

		if(LB_UNLIKELY(!FillByte(i, b)))
			return ConversionResult::BadSource;
		if((b & 0x80U) == 0)
		{
			if(st.Index == 0)
			{
				uc = b;
				return ConversionResult::OK;
			}
			return ConversionResult::Invalid;
		}

		using state_t = std::uint_fast8_t;
		static lconstexpr const state_t t_data_1[]{
			8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			10, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3,
			11, 6, 6, 6, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
		};
		static lconstexpr const state_t t_data_2[]{1, 2, 4, 4};
		auto& code(st.Value.UCS4);
#define CHRLib_Impl_UTF8_Decode_Fill \
	if(LB_UNLIKELY(!FillByte(i, b))) \
		return ConversionResult::BadSource; \
	code = (code << 6U) | (b & 0x3FU)
#define CHRLib_Impl_UTF8_Decode_Tail \
	CHRLib_Impl_UTF8_Decode_Fill; \
	trans(0x70U)
#define CHRLib_Impl_UTF8_Decode_FillTransTail(_mask) \
	CHRLib_Impl_UTF8_Decode_Fill; \
	trans(_mask); \
	CHRLib_Impl_UTF8_Decode_Tail;
		bool res(true);
		auto trans([&](unsigned mask){
			res &= (b & 0xC0U) == 0x80U
				&& (t_data_2[byte(b >> 4U) & 0x3U] & byte(mask)) != 0;
		});
		auto& type(st.Index);

		if(type == 0)
		{
			if((b & 0xC0U) != 0)
			{
				type = state_t(t_data_1[b & 0x3FU]);
				code = (0xFFU >> type) & byte(b);
			}
			else
				return ConversionResult::Invalid;
		}
		switch(type)
		{
		case 6:
			CHRLib_Impl_UTF8_Decode_Fill;
		case 3:
			CHRLib_Impl_UTF8_Decode_Fill;
		case 2:
			CHRLib_Impl_UTF8_Decode_Fill;
			break;
		case 4:
			CHRLib_Impl_UTF8_Decode_FillTransTail(3U);
			break;
		case 10:
			CHRLib_Impl_UTF8_Decode_FillTransTail(4U);
			break;
		case 5:
			CHRLib_Impl_UTF8_Decode_FillTransTail(1U);
			CHRLib_Impl_UTF8_Decode_Tail;
			break;
		case 11:
			CHRLib_Impl_UTF8_Decode_FillTransTail(6U);
			CHRLib_Impl_UTF8_Decode_Tail;
			break;
		case 8:
			return ConversionResult::Invalid;
		default:
			return ConversionResult::BadState;
		}
		uc = code;
		return res ? ConversionResult::OK : ConversionResult::Invalid;
#undef CHRLib_Impl_UTF8_Decode_FillTransTail
#undef CHRLib_Impl_UTF8_Decode_Tail
#undef CHRLib_Impl_UTF8_Decode_Fill
	}
	template<typename _tObj, typename _tIn>
	static ConversionResult
	Decode(_tObj&& uc, _tIn&& i, ConversionState&& st)
		lnoexcept_spec(Decode(uc, lforward(i), st))
	{
		return Decode(uc, lforward(i), st);
	}
	//@}

	template<typename _tOut>
	static size_t
	Encode(_tOut d, ucs4_t s) lnothrow
	{
		lnoexcept_assert("Invalid type found.", *d = char());
		using leo::is_undereferenceable;
		size_t l(0);

		lconstraint(!is_undereferenceable(d));
		if(s < 0x80U)
		{
			*d = s;
			return 1;
		}
		if(s < 0x800U)
			l = 2;
		else
		{
			*d = char(0xE0U | s >> 12U);
			++d;
			l = 3;
		}
		*d = char(0x80U | (s >> 6U & 0x3FU));
		*++d = char(0x80U | (s & 0x3FU));
		return l;
	}
};

//! \see 参考规范： RFC 2781 ，见 http://tools.ietf.org/html/rfc2781 。
template<>
struct GUCSMapper<CharSet::UTF_16BE> : UCSMapperBase
{
	using UCSMapperBase::Assign;

	/*!
	\brief 解码： UTF-16BE 。
	\since build 599
	*/
	template<typename _tObj, typename _tIn, typename _tState>
	static ConversionResult
	Decode(_tObj&& uc, _tIn&& i, _tState&& st)
		lnoexcept(noexcept(GetSequenceOf(st))
		&& noexcept(GetIndexOf(st)) && noexcept(!FillByte(i, st)))
	{
		const auto seq(GetSequenceOf(st));

		switch(GetIndexOf(st))
		{
		case 0:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
		case 1:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
			if(LB_UNLIKELY(CHRLib::IsSurrogateHead(seq[0])))
			{
				if(LB_UNLIKELY(!CHRLib::IsValidSurrogateHead(seq[0])))
					return ConversionResult::Invalid;
			}
			else
			{
				Assign(uc, ucs4_t(seq[0]) << 8U | ucs4_t(seq[1]));
				break;
			}
		case 2:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
		case 3:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
			if(LB_UNLIKELY(!CHRLib::IsValidSurrogateTail(seq[2])))
				return ConversionResult::Invalid;
			Assign(uc, ucs4_t(seq[0] & 0x03U) << 18U | ucs4_t(seq[1]) << 10U
				| ucs4_t(seq[2] & 0x03U) << 8U | ucs4_t(seq[3]));
			break;
		default:
			return ConversionResult::BadState;
		}
		return ConversionResult::OK;
	}
};

//! \see 参考规范： RFC 2781 ，见 http://tools.ietf.org/html/rfc2781 。
template<>
struct GUCSMapper<CharSet::UTF_16LE> : UCSMapperBase
{
	using UCSMapperBase::Assign;

	/*!
	\brief 解码： UTF-16LE 。
	*/
	template<typename _tObj, typename _tIn, typename _tState>
	static ConversionResult
	Decode(_tObj&& uc, _tIn&& i, _tState&& st)
		lnoexcept(noexcept(GetSequenceOf(st))
		&& noexcept(GetIndexOf(st)) && noexcept(!FillByte(i, st)))
	{
		const auto seq(GetSequenceOf(st));

		switch(GetIndexOf(st))
		{
		case 0:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
		case 1:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
			if(LB_UNLIKELY(CHRLib::IsSurrogateHead(seq[1])))
			{
				if(LB_UNLIKELY(!CHRLib::IsValidSurrogateHead(seq[1])))
					return ConversionResult::Invalid;
			}
			else
			{
				Assign(uc, ucs4_t(seq[0]) | ucs4_t(seq[1]) << 8U);
				break;
			}
		case 2:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
		case 3:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
			if(LB_UNLIKELY(!CHRLib::IsValidSurrogateTail(seq[3])))
				return ConversionResult::Invalid;
			Assign(uc, ucs4_t(seq[0] << 10U) | ucs4_t(seq[1] & 0x03U) << 18U
				| ucs4_t(seq[2]) | ucs4_t(seq[3] & 0x03U) << 8U);
			break;
		default:
			return ConversionResult::BadState;
		}
		return ConversionResult::OK;
	}
};

//! \see 参考规范： UAX #19 ，见 http://www.unicode.org/unicode/reports/tr19/ 。
template<>
struct GUCSMapper<CharSet::UTF_32BE> : UCSMapperBase
{
	using UCSMapperBase::Assign;

	/*!
	\brief 解码： UTF-32BE 。
	\since build 599
	*/
	template<typename _tObj, typename _tIn, typename _tState>
	static ConversionResult
	Decode(_tObj&& uc, _tIn&& i, _tState&& st)
		lnoexcept(noexcept(GetSequenceOf(st))
		&& noexcept(GetIndexOf(st)) && noexcept(!FillByte(i, st)))
	{
		const auto seq(GetSequenceOf(st));

		switch(GetIndexOf(st))
		{
		case 0:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
		case 1:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
		case 2:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
		case 3:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
			Assign(uc, ucs4_t(seq[0]) << 24U | ucs4_t(seq[1]) << 16U
				| ucs4_t(seq[2]) << 8U | ucs4_t(seq[3]));
			break;
		default:
			return ConversionResult::BadState;
		}
		return ConversionResult::OK;
	}
};

//! \see 参考规范： UAX #19 ，见 http://www.unicode.org/unicode/reports/tr19/ 。
template<>
struct GUCSMapper<CharSet::UTF_32LE> : UCSMapperBase
{
	using UCSMapperBase::Assign;

	/*!
	\brief 解码： UTF-32LE 。
	*/
	template<typename _tObj, typename _tIn, typename _tState>
	static ConversionResult
	Decode(_tObj&& uc, _tIn&& i, _tState&& st)
		lnoexcept(noexcept(GetSequenceOf(st))
		&& noexcept(GetIndexOf(st)) && noexcept(!FillByte(i, st)))
	{
		const auto seq(GetSequenceOf(st));

		switch(GetIndexOf(st))
		{
		case 0:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
		case 1:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
		case 2:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
		case 3:
			if(LB_UNLIKELY(!FillByte(i, st)))
				return ConversionResult::BadSource;
			Assign(uc, ucs4_t(seq[2]) | ucs4_t(seq[3]) << 8U
				| ucs4_t(seq[0]) << 16U | ucs4_t(seq[1]) << 24U);
			break;
		default:
			return ConversionResult::BadState;
		}
		return ConversionResult::OK;
	}
};
//@}
//@}

} // namespace CHRLib;

#pragma warning(pop)

#endif
