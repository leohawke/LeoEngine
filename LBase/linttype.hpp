/*
\par 修改时间:
2016-11-27 00:56 +0800
*/

#ifndef LBase_linttype_hpp
#define LBase_linttype_hpp 1

#include "ldef.h"
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

	using std::integral_constant;
	using std::make_signed;
	using std::make_unsigned;
	
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
}

#endif