/*!	\file functor.hpp
\ingroup LBase
\brief ͨ�÷º�����
\par �޸�ʱ��:
2016-12-29 12:09 +0800
*/

#ifndef LBase_functor_hpp
#define LBase_functor_hpp 1

#include "LBase/ref.hpp"// for enable_if_t, is_detected, constfn_addressof,
//	not_, or_, is_reference_wrapper, and_, std::greater, std::less,
//	std::greater_equal, std::less_equal, addrof_t, indirect_t;
#include <string> // for std::char_traits;
#include <algorithm> // for std::lexicographical_compare;

#if LB_IMPL_MSCPP >= 1900
/*!
\brief \<algorithm\> ���Բ��Ժꡣ
\see WG21 P0096R1 3.5 ��
\since build 1.4
*/
#	ifndef __cpp_lib_robust_nonmodifying_seq_ops
#		define __cpp_lib_robust_nonmodifying_seq_ops 201304
#	endif
#endif

namespace leo {
	//! \since build 1.4
	//@{
	namespace details
	{

		//! \since build 1.4
		template<class _type, typename = void>
		using mem_is_transparent_t = typename _type::is_transparent;

	} // namespace details;

	  //! \note �ڶ��������ڴ��� SFINAE ģ���������Ϊ�����͡�
	  //@{
	  /*!
	  \ingroup unary_type_traits
	  \brief �ж� _type �Ƿ���� is_transparent ��Ա���͡�
	  \since build 1.4
	  */
	template<typename _type, typename _tDummy = void>
	struct has_mem_is_transparent
		: is_detected<details::mem_is_transparent_t, _type, _tDummy>
	{};

	/*!
	\ingroup metafunction
	\brief �Ƴ���������� is_transparent ��Ա���ͱȽ��������ء�
	\sa enable_if_t
	\sa has_mem_is_transparent
	\see WG21 N3657 ��
	\since build 1.4
	*/
	template<typename _fComp, typename _tDummy, typename _type = void>
	using enable_if_transparent_t
		= enable_if_t<has_mem_is_transparent<_fComp, _tDummy>::value, _type>;
	//@}
	//@}

	/*!	\defgroup functors General Functors
	\brief �º�����
	\note ���������������ָ��ͷº�����
	\since build 1.4
	*/
	//@{
	/*!
	\brief ��ȷº�����
	\since build 1.4
	*/
	//@{
	template<typename _type = void>
	struct id
	{
		lconstfn _type
			operator()(const _type& x) const lnothrow
		{
			return x;
		}

		lconstfn
			operator add_ptr_t<_type(_type)>() const lnothrow
		{
			return[](_type x) lnothrow->_type{
				return x;
			};
		}
	};

	template<>
	struct id<void>
	{
		template<typename _type>
		lconstfn _type
			operator()(_type&& x) const lnothrow
		{
			return lforward(x);
		}

		template<typename _type>
		lconstfn
			operator add_ptr_t<_type(_type)>() const lnothrow
		{
			return[](_type x) lnothrow->_type{
				return x;
			};
		}
	};
	//@}

	/*!
	\brief addressof �º�����
	\since build 1.4
	*/
	//@{
	template<typename _type = void>
	struct addressof_op
	{
		lconstfn _type*
			operator()(_type& x) const lnothrow
		{
			return constfn_addressof(x);
		}
	};

	template<>
	struct addressof_op<void>
	{
		template<typename _type>
		lconstfn _type*
			operator()(_type& x) const lnothrow
		{
			return constfn_addressof(x);
		}
	};
	//@}

	/*!
	\brief ��Ա get ������
	\since build 1.4
	*/
	//@{
	template<typename _type = void>
	struct mem_get
	{
		auto
			operator()(const _type& x) const -> decltype(x.get())
		{
			return lforward(x.get());
		}
	};

	template<>
	struct mem_get<void>
	{
		template<typename _type>
		auto
			operator()(_type&& x) const lnoexcept_spec(std::declval<_type&&>().get())
			-> decltype(x.get())
		{
			return lforward(x.get());
		}
	};
	//@}

	/*!
	\brief ��ȹ�ϵ�º�����
	\note �� reference_wrapper ��ص�������ͬ boost::is_equal ��
	\since build 1.4
	*/
	struct is_equal
	{
		//! \since build 594
		template<typename _type1, typename _type2>
		lconstfn limpl(enable_if_t)<not_<or_<is_reference_wrapper<_type1>,
			is_reference_wrapper<_type2 >> >::value, bool>
			operator()(const _type1& x, const _type2& y) const lnoexcept_spec(x == y)
		{
			return x == y;
		}
		//! \since build 1.4
		//@{
		template<typename _type1, typename _type2>
		lconstfn limpl(enable_if_t)<and_<is_reference_wrapper<_type1>,
			not_<is_reference_wrapper<_type2>>>::value, bool>
			operator()(const _type1& x, const _type2& y) const lnothrow
		{
			return addressof(x.get()) == addressof(y);
		}
		template<typename _type1, typename _type2>
		lconstfn limpl(enable_if_t)<and_<not_<is_reference_wrapper<_type1>>,
			is_reference_wrapper<_type2 >> ::value, bool>
			operator()(const _type1& x, const _type2& y) const lnothrow
		{
			return addressof(x) == addressof(y.get());
		}
		template<typename _type1, typename _type2>
		lconstfn limpl(enable_if_t)<and_<is_reference_wrapper<_type1>,
			is_reference_wrapper<_type2 >> ::value, bool>
			operator()(const _type1& x, const _type2& y) const lnothrow
		{
			return addressof(x.get()) == addressof(y.get());
		}
		//@}
	};

	// NOTE: There is 'constexpr' in WG21 N3936; but it is in ISO/IEC 14882:2014.
#define LB_Impl_Functor_Ops_Primary(_n, _tRet, _using_stmt, _expr, ...) \
	template<typename _type = void> \
	struct _n \
	{ \
		using first_argument_type = _type; \
		_using_stmt \
		using result_type = _tRet; \
		\
		lconstfn _tRet \
		operator()(__VA_ARGS__) const \
		{ \
			return _expr; \
		} \
	};

#define LB_Impl_Functor_Ops_Spec(_n, _tparams, _params, _expr) \
	template<> \
	struct _n<void> \
	{ \
		using is_transparent = limpl(void); \
		\
		template<_tparams> \
		lconstfn auto \
		operator()(_params) const -> decltype(_expr) \
		{ \
			return _expr; \
		} \
	};

#define LB_Impl_Functor_Ops_Spec_Ptr(_n) \
	template<typename _type> \
	struct _n<_type*> \
	{ \
		using first_argument_type = _type*; \
		using second_argument_type = _type*; \
		using result_type = bool; \
		\
		lconstfn bool \
		operator()(_type* x, _type* y) const \
		{ \
			return std::_n<_type*>()(x, y); \
		} \
	};

#define LB_Impl_Functor_Ops_using(_tRet) using second_argument_type = _tRet;

#define LB_Impl_Functor_Ops1(_n, _op, _tRet) \
	LB_Impl_Functor_Ops_Primary(_n, _tRet, , _op x, const _type& x) \
	\
	LB_Impl_Functor_Ops_Spec(_n, typename _type, _type&& x, _op lforward(x))

#define LB_Impl_Functor_Ops2(_n, _op, _tRet) \
	LB_Impl_Functor_Ops_Primary(_n, _tRet, \
		LB_Impl_Functor_Ops_using(_tRet), x _op y, const _type& x, \
		const _type& y) \
	\
	LB_Impl_Functor_Ops_Spec(_n, typename _type1 LPP_Comma typename \
		_type2, _type1&& x LPP_Comma _type2&& y, lforward(x) _op lforward(y))

#define LB_Impl_Functor_Binary(_n, _op) \
	LB_Impl_Functor_Ops2(_n, _op, _type)

#define LB_Impl_Functor_Unary(_n, _op) \
	LB_Impl_Functor_Ops1(_n, _op, _type)

	//! \since build 1.4
	inline namespace cpp2014
	{

#if __cpp_lib_transparent_operators >= 201210 || __cplusplus >= 201402L
		using std::plus;
		using std::minus;
		using std::multiplies;
		using std::divides;
		using std::modulus;
		using std::negate;

		using std::equal_to;
		using std::not_equal_to;
		using std::greater;
		//! \since build 1.4
		using std::less;
		using std::greater_equal;
		using std::less_equal;

		using std::logical_and;
		using std::logical_or;
		using std::logical_not;

		using std::bit_and;
		using std::bit_or;
		using std::bit_xor;
		using std::bit_not;
#else
		/*!
		\note ͬ ISO WG21 N4296 ��Ӧ��׼��º�����
		\since build 1.4
		*/
		//@{
		//! \brief �ӷ��º�����
		LB_Impl_Functor_Binary(plus, +)

			//! \brief �����º�����
			LB_Impl_Functor_Binary(minus, -)

			//! \brief �˷��º�����
			LB_Impl_Functor_Binary(multiplies, *)

			/*!
			\brief �����º�����
			\since build 1.4
			*/
			LB_Impl_Functor_Binary(divides, / )

			//! \brief ģ����º�����
			LB_Impl_Functor_Binary(modulus, %)

			/*!
			\brief ȡ���º�����
			\since build 1.4
			*/
			LB_Impl_Functor_Unary(negate, -)

#define LB_Impl_Functor_bool(_n, _op) \
	LB_Impl_Functor_Ops2(_n, _op, bool)

#define LB_Impl_Functor_bool_Ordered(_n, _op) \
	LB_Impl_Functor_bool(_n, _op) \
	\
	LB_Impl_Functor_Ops_Spec_Ptr(_n)

			//! \brief ���ڹ�ϵ�º�����
			LB_Impl_Functor_bool(equal_to, == )

			//! \brief �����ڹ�ϵ�º�����
			LB_Impl_Functor_bool(not_equal_to, != )

			//! \brief ���ڹ�ϵ�º�����
			LB_Impl_Functor_bool_Ordered(greater, >)

			//! \brief С�ڹ�ϵ�º�����
			LB_Impl_Functor_bool_Ordered(less, <)

			//! \brief ���ڵ��ڹ�ϵ�º�����
			LB_Impl_Functor_bool_Ordered(greater_equal, >= )

			//! \brief С�ڵ��ڹ�ϵ�º�����
			LB_Impl_Functor_bool_Ordered(less_equal, <= )

			//! \since build 656
			//@{
			//! \brief �߼���º�����
			LB_Impl_Functor_bool(logical_and, &&)

			//! \brief �߼���º�����
			LB_Impl_Functor_bool(logical_or, || )

			//! \brief �߼��Ƿº�����
			LB_Impl_Functor_Ops1(logical_not, !, bool)

			//! \brief λ��º�����
			LB_Impl_Functor_Binary(bit_and, &)

			//! \brief λ��º�����
			LB_Impl_Functor_Binary(bit_or, | )

			//! \brief λ���º�����
			LB_Impl_Functor_Binary(bit_xor, ^)

			//! \brief λȡ���º�����
			// NOTE: Available in %std since ISO C++14.
			LB_Impl_Functor_Unary(bit_not, ~)
			//@}
			//@}
#endif

	} // inline namespace cpp2014;

	  /*!
	  \note LBase ��չ��
	  */
	  //@{
	  //! \brief һԪ & ������
	LB_Impl_Functor_Ops1(addrof, &, addrof_t<const _type&>)

	//! \brief һԪ * ������
	LB_Impl_Functor_Ops1(indirect, *, indirect_t<const _type&>)


	//! \brief ���õȼ۹�ϵ�º�����
	//@{
	LB_Impl_Functor_Ops_Primary(ref_eq, bool, LB_Impl_Functor_Ops_using(_type), \
		leo::constfn_addressof(x) == leo::constfn_addressof(y), \
		const _type& x, const _type& y)

	LB_Impl_Functor_Ops_Spec(ref_eq, typename _type1 LPP_Comma typename \
		_type2, _type1&& x LPP_Comma _type2&& y, \
		leo::constfn_addressof(lforward(x)) \
		== leo::constfn_addressof(lforward(y)))
	//@}
	//@}

#undef LB_Impl_Functor_bool_Ordered
#undef LB_Impl_Functor_bool
#undef LB_Impl_Functor_Unary
#undef LB_Impl_Functor_Binary
#undef LB_Impl_Functor_Ops2
#undef LB_Impl_Functor_Ops1
#undef LB_Impl_Functor_Ops_using
#undef LB_Impl_Functor_Ops_Spec
#undef LB_Impl_Functor_Ops_Primary

		//! \brief ѡ������/�Լ�����º�����
		//@{
		template<bool, typename _tScalar>
	struct xcrease_t
	{
		//! \since build 1.4
		inline _tScalar&
			operator()(_tScalar& _x) const
		{
			return ++_x;
		}
	};
	template<typename _tScalar>
	struct xcrease_t<false, _tScalar>
	{
		//! \since build 1.4
		inline _tScalar&
			operator()(_tScalar& _x) const
		{
			return --_x;
		}
	};
	//@}

	/*!
	\brief ѡ��ӷ�/�������ϸ�ֵ����º�����
	\since build 1.4
	*/
	//@{
	template<bool, typename _tScalar1, typename _tScalar2>
	struct delta_assignment
	{
		lconstfn _tScalar1&
			operator()(_tScalar1& x, _tScalar2 y) const
		{
			return x += y;
		}
	};
	template<typename _tScalar1, typename _tScalar2>
	struct delta_assignment<false, _tScalar1, _tScalar2>
	{
		lconstfn _tScalar1&
			operator()(_tScalar1& x, _tScalar2 y) const
		{
			return x -= y;
		}
	};
	//@}

	//! \brief ѡ������/�Լ����㡣
	template<bool _bIsPositive, typename _tScalar>
	lconstfn _tScalar&
		xcrease(_tScalar& _x)
	{
		return xcrease_t<_bIsPositive, _tScalar>()(_x);
	}

	/*!
	\brief ѡ��ӷ�/�������ϸ�ֵ���㡣
	\since build 1.4
	*/
	template<bool _bIsPositive, typename _tScalar1, typename _tScalar2>
	lconstfn _tScalar1&
		delta_assign(_tScalar1& _x, _tScalar2& _y)
	{
		return delta_assignment<_bIsPositive, _tScalar1, _tScalar2>()(_x, _y);
	}
	//@}

}

#endif
