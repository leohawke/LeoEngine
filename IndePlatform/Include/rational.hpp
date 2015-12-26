#ifndef IndePlatform_rational_hpp
#define IndePlatform_rational_hpp

#include "leoint.hpp"
#include "Operators.hpp"
#include "type_op.hpp"

namespace leo
{

	//@{
	//! \brief 判断指定的值是否为零值。
	//@{
	template<typename _type>
	lconstfn limpl(enable_if_t)<is_floating_point<_type>::value, bool>
		is_zero(_type x)
	{
		return std::fpclassify(x) == FP_ZERO;
	}
	template<typename _type,
		limpl(typename = enable_if_t<!is_floating_point<_type>::value>)>
		lconstfn bool
		is_zero(_type x)
	{
		return x == _type(0);
	}
	//@}


	/*!
	\brief 指定误差内舍入。
	\pre 参数为有限值。
	\warning 不检查参数。
	*/
	//@{
	/*
	\note 不检查误差非零。
	\note 使用 ADL \c round 。
	*/
	template<typename _type>
	lconstfn _type
		round_in_nocheck(_type x, _type err)
	{
		using std::round;

		return round(x / err) * err;
	}

	template<typename _type>
	lconstfn _type
		round_in(_type x, _type err)
	{
		return is_zero(err) ? x : round_in_nocheck(x, err);
	}
	//@}


	//! \brief 计算整数次幂。
	template<typename _type>
	lconstfn _type
		pow_int(_type x, size_t n)
	{
		return n != 0 ? (pow_int(x, n / 2) * pow_int(x, n / 2)
			* (n % 2 == 1 ? _type(x) : _type(1))) : _type(1);
	}

	//! \brief 计算平方。
	template<typename _type>
	lconstfn _type
		square(_type x)
	{
		return x * x;
	}

	//! \brief 计算立方。
	template<typename _type>
	lconstfn _type
		cube(_type x)
	{
		return x * x * x;
	}

	//! \brief 计算四次方。
	template<typename _type>
	lconstfn _type
		quad(_type x)
	{
		return square(ystdex::square(x));
	}
	//@}


	/*!
	\ingroup unary_type_traits
	\brief 判断类型是否可满足正规化要求：在 0 和 1 之间存在值进行表示。
	*/
	template<typename _type>
	struct is_normalizable : or_<is_floating_point<_type>>
	{};

#define YB_Impl_Rational_fp_T fixed_point<_tBase, _vInt, _vFrac>
#define YB_Impl_Rational_fp_T1 fixed_point<_tBase1, _vInt1, _vFrac1>
#define YB_Impl_Rational_fp_T2 fixed_point<_tBase2, _vInt2, _vFrac2>
#define YB_Impl_Rational_fp_PList \
	typename _tBase, size_t _vInt, size_t _vFrac
#define YB_Impl_Rational_fp_PList1 \
	typename _tBase1, size_t _vInt1, size_t _vFrac1
#define YB_Impl_Rational_fp_PList2 \
	typename _tBase2, size_t _vInt2, size_t _vFrac2

	/*!
	\brief 通用定点数。

	基于整数算术实现的确定有限精度二进制定点小数类型。可用于替换内建的浮点数类型。
	是否有符号同基本整数类型参数。若有符号，则最高有效位为符号位。
	逻辑布局： [符号位]|整数部分|小数部分 。各个部分的内部为补码表示。
	\tparam _tBase 基本整数类型。
	\tparam _vInt 整数部分（若有符号则不包括符号位，下同）二进制位数。
	\tparam _vFrac 分数部分二进制位数。
	\note 默认保留 6 位二进制小数。
	\note 部分实现参考： http://www.codeproject.com/KB/cpp/fp_math.aspx 。
	\warning 基本整数类型需要具有补码表示。
	\warning 非虚析构。
	\warning 算术运算可能溢出。
	\todo 实现模除和位操作。
	\todo 根据范围禁止算术类型隐式转换。
	*/
	template<typename _tBase = std::int32_t,
		size_t _vInt = std::numeric_limits<_tBase>::digits - 6U,
		size_t _vFrac = std::numeric_limits<_tBase>::digits - _vInt>
	class fixed_point : public operators<YB_Impl_Rational_fp_T>
	{
		static_assert(is_integral<_tBase>::value, "Non-integral type found.");
		static_assert(_vInt < size_t(std::numeric_limits<_tBase>::digits),
			"No sufficient fractional bits found.");
		static_assert(_vInt + _vFrac == size_t(std::numeric_limits<_tBase>::digits),
			"Wrong total bits found.");

		template<typename _OtherBase, size_t _vOtherInt, size_t _vOtherFrac>
		friend class fixed_point;
		friend class std::numeric_limits<YB_Impl_Rational_fp_T>;

	public:
		using base_type = _tBase;

		//! \brief 整数部分二进制位数。
		static lconstexpr const size_t int_bit_n = _vInt;
		//! \brief 小数部分二进制位数。
		static lconstexpr const size_t frac_bit_n = _vFrac;
		//! \brief 非符号位的二进制位数。
		static lconstexpr const size_t digit_bit_n = int_bit_n + frac_bit_n;

	private:
		base_type value;

	public:
		/*!
		\brief 无参数构造。
		\warning 基本整数类型成员未被初始化，具有未决定值 ，使用可能造成未定义行为。
		*/
		fixed_point() = default;

		lconstfn
			fixed_point(base_type v, raw_tag) lnothrow
			: value(v)
		{}
		template<typename _tInt>
		lconstfn
			fixed_point(_tInt val,
				limpl(enable_if_t<is_integral<_tInt>::value, _tInt*> = {})) lnothrowv
			// XXX: Conversion to 'base_type' might be implementation-defined if
			//	it is signed.
			: value(base_type(val << frac_bit_n))
		{}
		//@{
		template<typename _tFloat>
		explicit lconstfn
			fixed_point(_tFloat val, limpl(enable_if_t<
				is_floating_point<_tFloat>::value, _tFloat*> = {})) lnothrow
			: value(base_type(std::llround(base_element() * val)))
		{}
		template<typename _tFirst, typename _tSecond>
		//@{
		lconstfn
			fixed_point(_tFirst x, _tSecond y, limpl(enable_if_t<is_integral<
				_tFirst>::value && !is_same<_tSecond, raw_tag>::value, _tFirst*> = {}))
			lnothrowv
			: value((x << frac_bit_n) / y)
		{}
		template<typename _tFirst, typename _tSecond>
		explicit lconstfn
			fixed_point(_tFirst x, _tSecond y, limpl(enable_if_t<is_floating_point<
				_tFirst>::value && !is_same<_tSecond, raw_tag>::value, _tFirst*> = {}))
			lnothrowv
			: fixed_point(x / y)
		{}
		//@}
		lconstfn
			fixed_point(const fixed_point&) = default;
		template<size_t _vOtherInt, size_t _vOtherFrac>
		lconstfn
			fixed_point(const fixed_point<base_type, _vOtherInt, _vOtherFrac>& f,
				limpl(enable_if_t<(_vOtherInt < int_bit_n), base_type*> = {})) lnothrow
			: value(f.value >> (int_bit_n - _vOtherInt))
		{}
		template<size_t _vOtherInt, size_t _vOtherFrac>
		lconstfn
			fixed_point(const fixed_point<base_type, _vOtherInt, _vOtherFrac>& f,
				limpl(enable_if_t<(int_bit_n < _vOtherInt), base_type*> = {})) lnothrow
			: value(f.value << (_vOtherInt - int_bit_n))
		{}
		template<typename _tOtherBase, size_t _vOtherInt, size_t _vOtherFrac>
		lconstfn
			fixed_point(const fixed_point<_tOtherBase, _vOtherInt, _vOtherFrac>& f,
				limpl(enable_if_t<(frac_bit_n == _vOtherFrac), base_type*> = {}))
			lnothrow
			: value(f.value)
		{}
		template<typename _tOtherBase, size_t _vOtherInt, size_t _vOtherFrac>
		lconstfn
			fixed_point(const fixed_point<_tOtherBase, _vOtherInt, _vOtherFrac>& f,
				limpl(enable_if_t<(frac_bit_n < _vOtherFrac), base_type*> = {}))
			lnothrow
			: value(f.value >> (_vOtherFrac - frac_bit_n))
		{}
		template<typename _tOtherBase, size_t _vOtherInt, size_t _vOtherFrac>
		lconstfn
			fixed_point(const fixed_point<_tOtherBase, _vOtherInt, _vOtherFrac>& f,
				limpl(enable_if_t<(_vOtherFrac < frac_bit_n), base_type*> = {}))
			lnothrow
			: value(f.value << (frac_bit_n - _vOtherFrac))
		{}
		//@}

		fixed_point&
			operator=(const fixed_point&) = default;

		bool
			operator<(const fixed_point& f) const lnothrow
		{
			return value < f.value;
		}

		bool
			operator==(const fixed_point& f) const lnothrow
		{
			return value == f.value;
		}

		bool
			operator!() const lnothrow
		{
			return value == 0;
		}

		fixed_point
			operator-() const lnothrow
		{
			fixed_point result;

			result.value = -value;
			return result;
		}

		//@{
		fixed_point&
			operator++() lnothrowv
		{
			value += base_element();
			return *this;
		}

		fixed_point&
			operator--() lnothrowv
		{
			value -= base_element();
			return *this;
		}

		fixed_point&
			operator+=(const fixed_point& f) lnothrowv
		{
			value += f.value;
			return *this;
		}

		fixed_point&
			operator-=(const fixed_point& f) lnothrowv
		{
			value -= f.value;
			return *this;
		}

		fixed_point&
			operator*=(const fixed_point& f) lnothrowv
		{
			value = mul<frac_bit_n + is_signed<base_type>::value>(value,
				f.value, integral_constant<bool,
				is_signed<typename make_widen_int<base_type>::type>::value>());
			return *this;
		}

		fixed_point&
			operator/=(const fixed_point& f) lnothrowv
		{
			using widen_type = typename make_widen_int<base_type>::type;

			value = base_type((widen_type(value) << widen_type(frac_bit_n))
				/ f.value);
			return *this;
		}
		//@}

		fixed_point&
			operator>>=(size_t s) lnothrow
		{
			value >>= s;
			return *this;
		}

		fixed_point&
			operator<<=(size_t s) lnothrowv
		{
			value <<= s;
			return *this;
		}

		//@{
		template<typename _type,
			limpl(typename = enable_if_t<is_integral<_type>::value, _type>)>
			explicit lconstfn
			operator _type() const lnothrow
		{
			return _type(value >> base_type(frac_bit_n));
		}
		template<typename _type, limpl(typename _type2 = _type,
			typename = enable_if_t<is_floating_point<_type2>::value, _type>)>
			lconstfn
			operator _type() const lnothrow
		{
			return _type(value) / base_element();
		}
		//@}

		//! \since build 595
		//@{
		lconstfn base_type
			get() const lnothrow
		{
			return value;
		}

	private:
		template<size_t _vShiftBits>
		static lconstfn base_type
			mul(base_type x, base_type y, true_type) lnothrowv
		{
			return mul_signed<_vShiftBits>(
				typename make_widen_int<base_type>::type(x * y));
		}
		template<size_t _vShiftBits>
		static lconstfn base_type
			mul(base_type x, base_type y, false_type) lnothrowv
		{
			// NOTE: Only fit for unsigned type, due to there exists
			//	implementation-defined behavior in conversion and right shifting on
			//	operands of signed types.
			return base_type((typename make_widen_int<base_type>::type(x) * y)
				>> _vShiftBits);
		}

		template<size_t _vShiftBits>
		static lconstfn base_type
			mul_signed(typename make_widen_int<base_type>::type tmp) lnothrowv
		{
			return base_type(tmp < 0 ? -(-tmp >> _vShiftBits) : tmp >> _vShiftBits);
		}
		//@}

	public:
		/*!
		\brief 取 \c base_type 表达的单位元。

		取值等于 1 的元素，使用 \c base_type 表达。
		*/
		static lconstfn base_type
			base_element() lnothrow
		{
			return base_type(1) << frac_bit_n;
		}

		/*!
		\brief 取单位元。

		取值等于 1 的元素。
		*/
		static lconstfn fixed_point
			identity() lnothrow
		{
			return fixed_point(base_element());
		}

		friend lconstfn fixed_point
			fabs(fixed_point x) lnothrow
		{
			return x.value < 0 ? -x : x;
		}

		friend lconstfn fixed_point
			ceil(fixed_point x) lnothrow
		{
			return fixed_point(
				(x.value + base_element() - 1) & ~(base_element() - 1), raw_tag());
		}

		friend lconstfn fixed_point
			floor(fixed_point x) lnothrow
		{
			return fixed_point(x.value & ~(base_element() - 1), raw_tag());
		}

		friend lconstfn fixed_point
			round(fixed_point x) lnothrow
		{
			return fixed_point((x.value + (base_element() >> 1))
				& ~(base_element() - 1), raw_tag());
		}
	};


	//! \relates fixed_point
	//@{
#define YB_Impl_Rational_fp_TmplHead_2 \
	template<YB_Impl_Rational_fp_PList1, YB_Impl_Rational_fp_PList2>
#define YB_Impl_Rational_fp_TmplHead_2_l \
	template<YB_Impl_Rational_fp_PList, typename _type>
#define YB_Impl_Rational_fp_TmplHead_2_r \
	template<typename _type, YB_Impl_Rational_fp_PList>
#define YB_Impl_Rational_fp_TmplBody_Impl_2(_op) \
	{ \
		using result_type \
			= common_type_t<decay_t<decltype(x)>, decay_t<decltype(y)>>; \
	\
		return result_type(x) _op result_type(y); \
	}
#define YB_Impl_Rational_fp_TmplSig_2(_op) \
	operator _op(const YB_Impl_Rational_fp_T1& x, \
		const YB_Impl_Rational_fp_T2& y)
#define YB_Impl_Rational_fp_TmplSig_2_l(_op) \
	operator _op(const YB_Impl_Rational_fp_T& x, const _type& y)
#define YB_Impl_Rational_fp_TmplSig_2_r(_op) \
	operator _op(const _type& x, const YB_Impl_Rational_fp_T& y)

	/*!
	\brief 不同模板参数的二元算术操作符。
	*/
	//@{
#define YB_Impl_Rational_fp_arithmetic2(_op) \
	YB_Impl_Rational_fp_TmplHead_2 \
	lconstfn common_type_t<YB_Impl_Rational_fp_T1, YB_Impl_Rational_fp_T2> \
	YB_Impl_Rational_fp_TmplSig_2(_op) \
	YB_Impl_Rational_fp_TmplBody_Impl_2(_op) \
	\
	YB_Impl_Rational_fp_TmplHead_2_l \
	lconstfn enable_if_t<std::is_floating_point<_type>::value, \
		common_type_t<YB_Impl_Rational_fp_T, _type>> \
	YB_Impl_Rational_fp_TmplSig_2_l(_op) \
	YB_Impl_Rational_fp_TmplBody_Impl_2(_op) \
	\
	YB_Impl_Rational_fp_TmplHead_2_r \
	lconstfn enable_if_t<std::is_floating_point<_type>::value, \
		common_type_t<_type, YB_Impl_Rational_fp_T>> \
	YB_Impl_Rational_fp_TmplSig_2_r(_op) \
	YB_Impl_Rational_fp_TmplBody_Impl_2(_op)

	YB_Impl_Rational_fp_arithmetic2(+)
		YB_Impl_Rational_fp_arithmetic2(-)
		YB_Impl_Rational_fp_arithmetic2(*)
		YB_Impl_Rational_fp_arithmetic2(/ )

#undef YB_Impl_Rational_fp_ar2
		//@}

		/*!
		\brief 不同模板参数的二元关系操作符。
		*/
		//@{
#define YB_Impl_Rational_fp_rational2(_op) \
	YB_Impl_Rational_fp_TmplHead_2 \
	lconstfn bool \
	YB_Impl_Rational_fp_TmplSig_2(_op) \
	YB_Impl_Rational_fp_TmplBody_Impl_2(_op) \
	\
	YB_Impl_Rational_fp_TmplHead_2_l \
	lconstfn bool \
	YB_Impl_Rational_fp_TmplSig_2_l(_op) \
	YB_Impl_Rational_fp_TmplBody_Impl_2(_op) \
	\
	YB_Impl_Rational_fp_TmplHead_2_r \
	lconstfn bool \
	YB_Impl_Rational_fp_TmplSig_2_r(_op) \
	YB_Impl_Rational_fp_TmplBody_Impl_2(_op)

		YB_Impl_Rational_fp_rational2(== )
		YB_Impl_Rational_fp_rational2(!= )
		YB_Impl_Rational_fp_rational2(<)
		YB_Impl_Rational_fp_rational2(<= )
		YB_Impl_Rational_fp_rational2(>)
		YB_Impl_Rational_fp_rational2(>= )

#undef YB_Impl_Rational_fp_rational2
		//@}

#undef YB_Impl_Rational_fp_TmplSig_2_r
#undef YB_Impl_Rational_fp_TmplSig_2_l
#undef YB_Impl_Rational_fp_TmplSig_2
#undef YB_Impl_Rational_fp_TmplBody_Impl_2
#undef YB_Impl_Rational_fp_TmplHead_2_r
#undef YB_Impl_Rational_fp_TmplHead_2_l
#undef YB_Impl_Rational_fp_TmplHead_2

	template<YB_Impl_Rational_fp_PList>
	lconstfn YB_Impl_Rational_fp_T
		abs(YB_Impl_Rational_fp_T x) lnothrow
	{
		return fabs(x);
	}
	//@}


	/*!
	\brief modular_arithmetic 的 fixed_point 特化类型。
	\note 使用保留公共整数类型和整数位数策略选取公共类型。
	*/
	template<YB_Impl_Rational_fp_PList>
	struct modular_arithmetic<YB_Impl_Rational_fp_T>
		: modular_arithmetic<typename YB_Impl_Rational_fp_T::base_type>
	{};


	/*!
	\brief is_normalizable 的 fixed_point 特化类型。
	*/
	template<YB_Impl_Rational_fp_PList>
	struct is_normalizable<YB_Impl_Rational_fp_T> : true_type
	{};

	/*!
	\brief 取包括指定有效二进制位和至少指定位整数的定点数类型。
	*/
#if LB_IMPL_MSCPP || LB_IMPL_CLANGPP
	template<size_t _vFrac, size_t _vInt = 1, bool _bSigned = false>
#else
	template<size_t _vFrac, size_t _vInt = 1, bool _bSigned = {}>
#endif
	using make_fixed_t = fixed_point<conditional_t<_bSigned,
		typename make_width_int<_vFrac + _vInt>::least_type,
		typename make_width_int<_vFrac + _vInt>::unsigned_least_type>, _vFrac>;



}

namespace std {

	/*!
	\brief std::common_type 的 leo::fixed_point 特化类型。
	\note 使用保留公共整数类型和整数位数策略选取公共类型。
	*/
	template<YB_Impl_Rational_fp_PList1, YB_Impl_Rational_fp_PList2>
	struct common_type<leo::YB_Impl_Rational_fp_T1,
		leo::YB_Impl_Rational_fp_T2>
	{
	private:
		using common_base_type = common_type_t<_tBase1, _tBase2>;

		static lconstexpr const size_t int_size = _vInt1 < _vInt2 ? _vInt2 : _vInt1;

	public:
		using type = leo::fixed_point<common_base_type, int_size,
			std::numeric_limits<common_base_type>::digits - int_size>;
	};

	/*!
	\brief std::common_type 的 ystdex::fixed_point 和其它类型的特化类型。
	\todo 支持范围不小于定点数的可以转换为 std::double_t 的类型为公共类型。

	当其它类型是浮点数时即为公共类型，
	否则 leo::fixed_point 的实例为公共类型。
	*/
	//@{
	template<YB_Impl_Rational_fp_PList, typename _type>
	struct common_type<leo::YB_Impl_Rational_fp_T, _type>
	{
	private:
		using fixed = leo::YB_Impl_Rational_fp_T;

	public:
		using type = conditional_t<is_floating_point<_type>::value
#if 0
			|| !(std::double_t(std::numeric_limits<fixed>::min())
				< std::double_t(std::numeric_limits<_type>::min())
				|| std::double_t(std::numeric_limits<_type>::max())
				< std::double_t(std::numeric_limits<fixed>::max()))
#endif
			, _type, fixed>;
	};

	template<typename _type, YB_Impl_Rational_fp_PList>
	struct common_type<_type, leo::YB_Impl_Rational_fp_T>
	{
		using type = common_type_t<leo::YB_Impl_Rational_fp_T, _type>;
	};
	//@}


	/*!
	\brief leo::fixed_point 散列支持。
	*/
	//@{
	template<typename>
	struct hash;

	template<YB_Impl_Rational_fp_PList>
	struct hash<leo::YB_Impl_Rational_fp_T>
	{
		size_t
			operator()(const leo::YB_Impl_Rational_fp_T& k) const
			lnoexcept_spec(limpl(hash<_tBase>{}(k.get())))
		{
			return hash<_tBase>{}(k.get());
		}
	};
	//@}

	/*!
	\brief std::numeric_traits 的 leo::fixed_point 特化类型。
	*/
	template<YB_Impl_Rational_fp_PList>
	class numeric_limits<leo::YB_Impl_Rational_fp_T>
	{
	private:
		using fp_type = leo::YB_Impl_Rational_fp_T;
		using base_type = typename fp_type::base_type;

	public:
		static lconstexpr const bool is_specialized = true;

		static lconstfn fp_type
			min() lnothrow
		{
			return fp_type(std::numeric_limits<base_type>::min(),
				stdex::raw_tag());
		}

		static lconstfn fp_type
			max() lnothrow
		{
			return fp_type(std::numeric_limits<base_type>::max(),
				stdex::raw_tag());
		}

		static lconstfn fp_type
			lowest() lnothrow
		{
			return min();
		}

		static lconstexpr const int digits = _vInt;
		static lconstexpr const int digits10 = digits * 643L / 2136;
		static lconstexpr const int max_digits10 = 0;
		static lconstexpr const bool is_signed = numeric_limits<base_type>::is_signed;
		static lconstexpr const bool is_integer = {};
		static lconstexpr const bool is_exact = true;
		static lconstexpr const int radix = 2;

		static lconstfn fp_type
			epsilon() lnothrow
		{
			return fp_type(1, typename stdex::raw_tag());
		}

		static lconstfn fp_type
			round_error() lnothrow
		{
			return 0.5;
		}

		static lconstexpr const int min_exponent = 0;
		static lconstexpr const int min_exponent10 = 0;
		static lconstexpr const int max_exponent = 0;
		static lconstexpr const int max_exponent10 = 0;

		static lconstexpr const bool has_infinity = {};
		static lconstexpr const bool has_quiet_NaN = {};
		static lconstexpr const bool has_signaling_NaN = has_quiet_NaN;
		static lconstexpr const float_denorm_style has_denorm = denorm_absent;
		static lconstexpr const bool has_denorm_loss = {};

		static lconstfn fp_type
			infinity() lnothrow
		{
			return 0;
		}

		static lconstfn fp_type
			quiet_NaN() lnothrow
		{
			return 0;
		}

		static lconstfn fp_type
			signaling_NaN() lnothrow
		{
			return 0;
		}

		static lconstfn fp_type
			denorm_min() lnothrow
		{
			return 0;
		}

		static lconstexpr const bool is_iec559 = {};
		static lconstexpr const bool is_bounded = true;
		static lconstexpr const bool is_modulo
			= numeric_limits<base_type>::is_modulo;

		static lconstexpr const bool traps = numeric_limits<base_type>::traps;
		static lconstexpr const bool tinyness_before = {};
		static lconstexpr const float_round_style round_style = round_toward_zero;
	};
#undef YB_Impl_Rational_fp_PList2
#undef YB_Impl_Rational_fp_PList1
#undef YB_Impl_Rational_fp_PList
#undef YB_Impl_Rational_fp_T2
#undef YB_Impl_Rational_fp_T1
#undef YB_Impl_Rational_fp_T
}


#endif
