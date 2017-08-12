/*
\par 修改时间:
2016-11-27 00:56 +0800
*/

#ifndef LBase_linttype_hpp
#define LBase_linttype_hpp 1

#include "LBase/iterator_op.hpp"
#include <numeric>
#include <limits>

namespace leo
{
	/*!
	\brief 字节序。
	\todo 使用单独的头文件。
	*/
	enum class byte_order
	{
		unknown = 0,
		neutral = 1,
		little = 2,
		big = 3,
		PDP = 4
	};

	//! \since build CPP17
	//@{
	namespace details
	{
		using byte = stdex::byte;

		struct bit_order_tester
		{
			unsigned char le : 4, : CHAR_BIT - 4;
		};

		union byte_order_tester
		{
			std::uint_least32_t n;
			byte p[4];
		};

	} // namespace details;

	  //! \brief 测试本机字节序。
	lconstfn_relaxed LB_STATELESS byte_order
		native_byte_order()
	{
		lconstexpr const details::byte_order_tester x = { 0x01020304 };

		return x.p[0] == 4 ? byte_order::little : (x.p[0] == 1 ? byte_order::big
			: (x.p[0] == 2 ? byte_order::PDP : byte_order::unknown));
	}

	//! \brief 测试本机位序。
	lconstfn LB_STATELESS bool
		native_little_bit_order()
	{
		return bool(details::bit_order_tester{ 1 }.le & 1);
	}
	//@}

	inline namespace inttype {
		using stdex::byte;
		using stdex::octet;
		using uint8 = std::uint8_t;
		using int8 = std::int8_t;
		using uint16 = std::uint16_t;
		using int16 = std::int16_t;
		using uint32 = std::uint32_t;
		using int32 = std::int32_t;
		using uint64 = std::uint64_t;
		using int64 = std::int64_t;
	}
	
	template<typename _tInt>
	//取整数类型的位宽度
	struct integer_width : integral_constant<size_t, sizeof(_tInt) * CHAR_BIT>
	{};

	template<typename _type, bool>
	//有符号类型
	struct make_signed_c : make_signed<_type>
	{};

	template<typename _type>
	//无符号类型
	struct make_signed_c<_type, false> : make_unsigned<_type>
	{};

	template<size_t _vWidth>
	//取指定宽度的整数类型
	struct make_width_int
	{
		static_assert(_vWidth <= 64, "Width too large found.");

		using fast_type = typename make_width_int<(_vWidth <= 8U ? 8U
			: (_vWidth <= 16U ? 16U : (_vWidth <= 32U ? 32U : 64U)))>::fast_type;
		using unsigned_fast_type = typename make_width_int<(_vWidth <= 8U ? 8U
			: (_vWidth <= 16U ? 16U : (_vWidth <= 32U ? 32U : 64U)))>
			::unsigned_fast_type;
		using least_type = typename make_width_int<(_vWidth <= 8U ? 8U
			: (_vWidth <= 16U ? 16U : (_vWidth <= 32U ? 32U : 64U)))>::least_type;
		using unsigned_least_type = typename make_width_int<(_vWidth <= 8U ? 8U
			: (_vWidth <= 16U ? 16U : (_vWidth <= 32U ? 32U : 64U)))>
			::unsigned_least_type;
	};

	template<>
	struct make_width_int<8U>
	{
		using type = int8;
		using unsigned_type = uint8;
		using fast_type = std::int_fast8_t;
		using unsigned_fast_type = std::uint_fast8_t;
		using least_type = std::int_least8_t;
		using unsigned_least_type = std::uint_least8_t;
	};

	template<>
	struct make_width_int<16U>
	{
		using type = int16;
		using unsigned_type = uint16;
		using fast_type = std::int_fast16_t;
		using unsigned_fast_type = std::uint_fast16_t;
		using least_type = std::int_least16_t;
		using unsigned_least_type = std::uint_least16_t;
	};

	template<>
	struct make_width_int<32U>
	{
		using type = int32;
		using unsigned_type = uint32;
		using fast_type = std::int_fast32_t;
		using unsigned_fast_type = std::uint_fast32_t;
		using least_type = std::int_least32_t;
		using unsigned_least_type = std::uint_least32_t;
	};

	template<>
	struct make_width_int<64U>
	{
		using type = int64;
		using unsigned_type = uint64;
		using fast_type = std::int_fast64_t;
		using unsigned_fast_type = std::uint_fast64_t;
		using least_type = std::int_least64_t;
		using unsigned_least_type = std::uint_least64_t;
	};

	/*!
	\ingroup metafunctions
	\brief 位加倍扩展。
	\note 可用于定点数乘除法中间类型。
	\todo 使用扩展整数类型保持 64 位类型精度。
	*/
	//@{
	template<typename _type, bool _bSigned = is_signed<_type>::value>
	struct make_widen_int
	{
	private:
		using width = integer_width<_type>;

	public:
		using type = _t<make_signed_c<_t<make_width_int<(width::value << 1) <= 64
			? width::value << 1 : width::value>>, _bSigned>>;
	};

	/*!
	\brief 公共整数类型。
	\note 同 common_type 但如果可能，按需自动扩展整数位宽避免缩小数值范围。
	*/
	//@{
	template<typename... _types>
	struct common_int_type : common_type<_types...>
	{};

	template<typename _type1, typename _type2, typename... _types>
	struct common_int_type<_type1, _type2, _types...>
	{
	private:
		using common_t = common_type_t<_type1, _type2>;

	public:
		using type = typename common_int_type<cond_t<
			and_<is_unsigned<common_t>, or_<is_signed<_type1>, is_signed<_type2>>>,
			_t<make_widen_int<common_t, true>>, common_t>, _types...>::type;
	};
	//@}

	template<typename _type>
	struct modular_arithmetic
	{
		static lconstexpr _type value = is_unsigned<_type>::value
			? std::numeric_limits<_type>::max() : _type(0);
	};

	template<typename _type1, typename _type2>
	struct have_same_modulo : integral_constant<bool, uintmax_t(modular_arithmetic<
		_type1>::value) != 0 && uintmax_t(modular_arithmetic<_type1>::value)
		== uintmax_t(modular_arithmetic<_type2>::value)>
	{};

	template<size_t _vWidth, typename _tIn>
	typename make_width_int<_vWidth>::unsigned_type
		pack_uint(_tIn first, _tIn last) lnothrowv
	{
		static_assert(_vWidth != 0 && _vWidth % std::numeric_limits<byte>::digits
			== 0, "Invalid integer width found.");
		using utype = typename make_width_int<_vWidth>::unsigned_type;

		lconstraint(!is_undereferenceable(first));
		return std::accumulate(first, last, utype(), [](utype x, byte y) {
			return utype(x << std::numeric_limits<byte>::digits | y);
		});
	}

	template<size_t _vWidth, typename _tOut>
	void
		unpack_uint(typename make_width_int<_vWidth>::unsigned_type value,
			_tOut result) lnothrow
	{
		static_assert(_vWidth != 0 && _vWidth % std::numeric_limits<byte>::digits
			== 0, "Invalid integer width found.");
		auto n(_vWidth);

		while (!(_vWidth < (n -= std::numeric_limits<byte>::digits)))
		{
			lconstraint(!is_undereferenceable(result));
			*result = byte(value >> n);
			++result;
		}
	}

	template<size_t _vWidth>
	inline LB_NONNULL(1) typename make_width_int<_vWidth>::unsigned_type
		read_uint_le(const byte* buf) lnothrowv
	{
		lconstraint(buf);
		return leo::pack_uint<_vWidth>(leo::make_reverse_iterator(
			buf + _vWidth / std::numeric_limits<byte>::digits),
			leo::make_reverse_iterator(buf));
	}
}

#endif