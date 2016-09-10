#ifndef LBase_CHRLib_CharacterMapping_h
#define LBase_CHRLib_CharacterMapping_h 1

#include "LBase/lmacro.h"
#include "LBase/CHRLib/Encoding.h"
#include "LBase/cassert.h"

namespace CHRLib
{
	using CharSet::Encoding;
	

	/*!
	\brief 默认字符编码。
	*/
	lconstexpr const Encoding CS_Default = CharSet::UTF_8;


	/*!
	\ingroup metafunctions
	\brief 取指定字符类型对应的默认定长编码。
	*/
	//@{
	template<typename _tChar>
	struct MapFixedEncoding : std::integral_constant<Encoding, CS_Default>
	{};

	template<>
	struct MapFixedEncoding<ucs2_t>
		: std::integral_constant<Encoding, CharSet::csUnicode>
	{};

	template<>
	struct MapFixedEncoding<ucs4_t>
		: std::integral_constant<Encoding, CharSet::csUCS4>
	{};
	//@}


	/*!
	\brief 取 c_ptr 指向的大端序双字节字符。
	\pre 断言： \c c_ptr 。
	*/
	inline PDefH(ucs2_t, FetchBiCharBE, const char* c_ptr)
		ImplRet(lconstraint(c_ptr), ucs2_t((*c_ptr << CHAR_BIT) | c_ptr[1]))

		/*!
		\brief 取 c_ptr 指向的小端序双字节字符。
		\pre 断言： \c c_ptr 。
		*/
		inline PDefH(ucs2_t, FetchBiCharLE, const char* c_ptr)
		ImplRet(lconstraint(c_ptr), ucs2_t((c_ptr[1] << CHAR_BIT) | *c_ptr))


		/*!
		\brief 编码转换结果。
		*/
	enum class ConversionResult
	{
		OK = 0, //!< 转换成功。
		BadState, //!< 转换状态错误。
		BadSource, //!< 源数据不可达（如越界）。
		Invalid, //!< 数据校验失败（如不构成代码点的字节序列）。
		Unhandled //!< 未处理（超过被处理的界限）。
	};


	/*!
	\brief 编码转换状态。
	*/
	struct ConversionState
	{
		/*!
		\brief 当前状态索引。
		*/
		std::uint_fast8_t Index;
		union
		{
			ucsint_t Wide;
			ucs4_t UCS4;
			/*!
			\brief 字节序列：宽字符的字节表示。
			*/
			byte Sequence[sizeof(ucsint_t)];
		} Value;

		lconstfn
			ConversionState(std::size_t n = 0)
			: Index(std::uint_fast8_t(n)), Value()
		{}
	};

	//! \relates ConversionState
	//@{
	lconstfn PDefH(std::uint_fast8_t&, GetIndexOf, ConversionState& st)
		ImplRet(st.Index)
		lconstfn PDefH(ucsint_t&, GetWideOf, ConversionState& st)
		ImplRet(st.Value.Wide)
		lconstfn PDefH(byte*, GetSequenceOf, ConversionState& st)
		ImplRet(st.Value.Sequence)
		//@}


		/*!
		\brief 取一般状态类型索引。
		*/
		template<typename _type>
	lconstfn _type&
		GetIndexOf(_type& st)
	{
		return st;
	}


	/*!
	\brief 取指定固定编码的固定字符宽度。
	\return 未定义编码或变长编码返回 0 ，否则为指定编码中每个字符占用的字节数。
	\note UTF-16 视为 UCS-2 。
	*/
	LB_API std::size_t
		FetchFixedCharWidth(Encoding);

	/*!
	\brief 取指定编码的最大字符宽度。
	\return 未定义编码返回 0 ，否则为指定编码中每个字符最大可能占用的字节数。
	\note UTF-16 视为 UCS-2 。
	*/
	LB_API std::size_t
		FetchMaxCharWidth(Encoding);

	/*!
	\brief 取指定变长编码的最大字符宽度。
	\return 未定义编码或固定编码返回 0 ，否则为指定编码中每个字符最大可能占用的字节数。
	\note UTF-16 视为 UCS-2 。
	*/
	LB_API std::size_t
		FetchMaxVariantCharWidth(Encoding);
}

#endif
