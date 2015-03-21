#ifndef IndePlatform_leoint_hpp
#define IndePlatform_leoint_hpp

#include "type_op.hpp"
#include <limits>

namespace leo
{
	//类型暴漏

	using uint8 = std::uint8_t;
	using int8 = std::int8_t;
	using uint16 = std::uint16_t;
	using int16 = std::int16_t;
	using uint32 = std::uint32_t;
	using int32 = std::int32_t;
	using uint64 = std::uint64_t;
	using int64 = std::int64_t;

	namespace win
	{
		using uint = unsigned int;
		using dword = unsigned long;
		using BOOL = int;
		using HRESULT = long;
	}
	template<typename _tInt>
	//取整数类型的位宽度
	struct integer_width : integral_constant<size_t, sizeof(_tInt) * CHAR_BIT>
	{};

	template<typename _type, bool>
	//有符号类型
	struct make_signed_c
	{
		using type = make_signed_t<_type>;
	};

	template<typename _type>
	//无符号类型
	struct make_signed_c<_type, false>
	{
		using type = make_unsigned_t<_type>;
	};

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
}

#endif