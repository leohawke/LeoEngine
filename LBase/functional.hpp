/*!	\file functional.hpp
\ingroup LBase
\brief 函数和可调用对象。
\par 修改时间:
2017-02-18 17:11 +0800
*/

#ifndef LBase_functional_hpp
#define LBase_functional_hpp 1

#include "LBase/type_op.hpp" // for "tuple.hpp", true_type, std::tuple,
//	is_convertible, vseq::at, bool_constant, index_sequence_for,
//	member_target_type_t, _t, std::tuple_size, vseq::join_n_t, std::swap,
//	common_nonvoid_t, false_type, integral_constant, is_nothrow_swappable,
//	make_index_sequence, exclude_self_t;
#include "LBase/functor.hpp"// for "ref.hpp", <functional>, std::function,
//	__cpp_lib_invoke, less, addressof_op, mem_get;
#include <numeric>// for std::accumulate;

#ifdef LB_IMPL_MSCPP
#pragma warning(push)
#pragma warning(disable:4003)
#endif

namespace leo
{
	//@{
	namespace details
	{

		template<class, class, class>
		struct tuple_element_convertible;

		template<class _type1, class _type2>
		struct tuple_element_convertible<_type1, _type2, index_sequence<>> : true_
		{};

		template<typename... _types1, typename... _types2, size_t... _vSeq,
			size_t _vHead>
			struct tuple_element_convertible<std::tuple<_types1...>, std::tuple<_types2...>,
			index_sequence<_vHead, _vSeq...>>
		{
			static_assert(sizeof...(_types1) == sizeof...(_types2),
				"Mismatched sizes of tuple found.");

		private:
			using t1 = std::tuple<_types1...>;
			using t2 = std::tuple<_types2...>;

		public:
			static lconstexpr const bool value
				= is_convertible<vseq::at<t1, _vHead>, vseq::at<t2, _vHead>>::value
				&& tuple_element_convertible<t1, t2, index_sequence<_vSeq...>>::value;
		};

	} // namespace details;

	  /*!
	  \ingroup binary_type_traits
	  */
	  //@{
	  //! \brief 判断指定类型之间是否协变。
	  //@{
	template<typename _tFrom, typename _tTo>
	struct is_covariant : is_convertible<_tFrom, _tTo>
	{};

	template<typename _tFrom, typename _tTo, typename... _tFromParams,
		typename... _tToParams>
		struct is_covariant<_tFrom(_tFromParams...), _tTo(_tToParams...)>
		: is_covariant<_tFrom, _tTo>
	{};

	template<typename... _tFroms, typename... _tTos>
	struct is_covariant<std::tuple<_tFroms...>, std::tuple<_tTos...>>
		: bool_<details::tuple_element_convertible<std::tuple<_tFroms...>,
		std::tuple<_tTos...>, index_sequence_for<_tTos...>>::value>
	{};

	template<typename _tFrom, typename _tTo, typename... _tFromParams,
		typename... _tToParams>
		struct is_covariant<std::function<_tFrom(_tFromParams...)>,
		std::function<_tTo(_tToParams...)>>
		: is_covariant<_tFrom(_tFromParams...), _tTo(_tToParams...)>
	{};
	//@}


	//! \brief 判断指定类型之间是否逆变。
	//@{
	template<typename _tFrom, typename _tTo>
	struct is_contravariant : is_convertible<_tTo, _tFrom>
	{};

	template<typename _tResFrom, typename _tResTo, typename... _tFromParams,
		typename... _tToParams>
		struct is_contravariant<_tResFrom(_tFromParams...), _tResTo(_tToParams...)>
		: is_contravariant<std::tuple<_tFromParams...>, std::tuple<_tToParams...>>
	{};

	template<typename... _tFroms, typename... _tTos>
	struct is_contravariant<std::tuple<_tFroms...>, std::tuple<_tTos...>>
		: bool_<details::tuple_element_convertible<std::tuple<_tTos...>,
		std::tuple<_tFroms...>, index_sequence_for<_tTos...>>::value>
	{};

	template<typename _tResFrom, typename _tResTo, typename... _tFromParams,
		typename... _tToParams>
		struct is_contravariant<std::function<_tResFrom(_tFromParams...)>,
		std::function<_tResTo(_tToParams...)>>
		: is_contravariant<_tResFrom(_tFromParams...), _tResTo(_tToParams...)>
	{};
	//@}
	//@}


	//@{
	//! \brief 统计函数参数列表中的参数个数。
	template<typename... _tParams>
	lconstfn size_t
		sizeof_params(_tParams&&...) lnothrow
	{
		return sizeof...(_tParams);
	}


	//@{
	//! \brief 变长参数操作模板。
	//@{
	template<size_t _vN>
	struct variadic_param
	{
		template<typename _type, typename... _tParams>
		static lconstfn auto
			get(_type&&, _tParams&&... args) lnothrow
			-> decltype(variadic_param<_vN - 1>::get(lforward(args)...))
		{
			static_assert(sizeof...(args) == _vN,
				"Wrong variadic arguments number found.");

			return variadic_param<_vN - 1>::get(lforward(args)...);
		}
	};

	template<>
	struct variadic_param<0U>
	{
		template<typename _type>
		static lconstfn auto
			get(_type&& arg) lnothrow -> decltype(lforward(arg))
		{
			return lforward(arg);
		}
	};
	//@}


	/*!
	\brief 取指定位置的变长参数。
	\tparam _vN 表示参数位置的非负数，从左开始计数，第一个参数为 0 。
	*/
	template<size_t _vN, typename... _tParams>
	lconstfn auto
		varg(_tParams&&... args) lnothrow
		-> decltype(variadic_param<_vN>::get(lforward(args)...))
	{
		static_assert(_vN < sizeof...(args),
			"Out-of-range index of variadic argument found.");

		return variadic_param<_vN>::get(lforward(args)...);
	}
	//@}


	//! \see 关于调用参数类型： ISO C++11 30.3.1.2 [thread.thread.constr] 。
	//@{
	//! \brief 顺序链式调用。
	//@{
	template<typename _func>
	inline void
		chain_apply(_func&& f) lnothrow
	{
		return lforward(f);
	}
	template<typename _func, typename _type, typename... _tParams>
	inline void
		chain_apply(_func&& f, _type&& arg, _tParams&&... args)
		lnoexcept_spec(leo::chain_apply(
			lforward(lforward(f)(lforward(arg))), lforward(args)...))
	{
		return leo::chain_apply(lforward(lforward(f)(lforward(arg))),
			lforward(args)...);
	}
	//@}

	//! \brief 顺序递归调用。
	//@{
	template<typename _func>
	inline void
		seq_apply(_func&&) lnothrow
	{}
	template<typename _func, typename _type, typename... _tParams>
	inline void
		seq_apply(_func&& f, _type&& arg, _tParams&&... args)
		lnoexcept_spec(limpl(lunseq(0, (void(lforward(f)(lforward(args))), 0)...)))
	{
		lforward(f)(lforward(arg));
		leo::seq_apply(lforward(f), lforward(args)...);
	}
	//@}

	//! \brief 无序调用。
	template<typename _func, typename... _tParams>
	inline void
		unseq_apply(_func&& f, _tParams&&... args)
		lnoexcept_spec(limpl(lunseq((void(lforward(f)(lforward(args))), 0)...)))
	{
		lunseq((void(lforward(f)(lforward(args))), 0)...);
	}
	//@}
	//@}

	// TODO: Blocked. Wait for upcoming ISO C++17 for %__cplusplus.
#if __cpp_lib_invoke >= 201411
	using std::invoke;
#else
	namespace details
	{

		template<typename _type, typename _tCallable>
		struct is_callable_target
			: is_base_of<member_target_type_t<_tCallable>, decay_t<_type>>
		{};

		template<typename _fCallable, typename _type>
		struct is_callable_case1 : and_<is_member_function_pointer<_fCallable>,
			is_callable_target<_type, _fCallable>>
		{};

		template<typename _fCallable, typename _type>
		struct is_callable_case2 : and_<is_member_function_pointer<_fCallable>,
			not_<is_callable_target<_type, _fCallable>>>
		{};

		template<typename _fCallable, typename _type>
		struct is_callable_case3 : and_<is_member_object_pointer<_fCallable>,
			is_callable_target<_type, _fCallable>>
		{};

		template<typename _fCallable, typename _type>
		struct is_callable_case4 : and_<is_member_object_pointer<_fCallable>,
			not_<is_callable_target<_type, _fCallable>>>
		{};

		template<typename _fCallable, typename _type, typename... _tParams>
		lconstfn auto
			invoke_impl(_fCallable&& f, _type&& obj, _tParams&&... args)
			-> enable_if_t<is_callable_case1<decay_t<_fCallable>, _type>::value,
			decltype((lforward(obj).*f)(lforward(args)...))>
		{
			return lconstraint(f), (lforward(obj).*f)(lforward(args)...);
		}
		template<typename _fCallable, typename _type, typename... _tParams>
		lconstfn auto
			invoke_impl(_fCallable&& f, _type&& obj, _tParams&&... args)
			-> enable_if_t<is_callable_case2<decay_t<_fCallable>, _type>::value,
			decltype(((*lforward(obj)).*f)(lforward(args)...))>
		{
			return lconstraint(f), ((*lforward(obj)).*f)(lforward(args)...);
		}
		template<typename _fCallable, typename _type>
		lconstfn auto
			invoke_impl(_fCallable&& f, _type&& obj)
			-> enable_if_t<is_callable_case3<decay_t<_fCallable>, _type>::value,
			decltype(lforward(obj).*f)>
		{
			return lconstraint(f), lforward(obj).*f;
		}
		template<typename _fCallable, typename _type>
		lconstfn auto
			invoke_impl(_fCallable&& f, _type&& obj)
			-> enable_if_t<is_callable_case4<decay_t<_fCallable>, _type>::value,
			decltype((*lforward(obj)).*f)>
		{
			return lconstraint(f), (*lforward(obj)).*f;
}
		template<typename _func, typename... _tParams>
		lconstfn auto
			invoke_impl(_func&& f, _tParams&&... args)
			-> enable_if_t<!is_member_pointer<decay_t<_func>>::value,
			decltype(lforward(f)(lforward(args)...))>
		{
			return lforward(f)(lforward(args)...);
		}

} // namespace details;

  /*!
  \brief 调用可调用对象。
  \sa http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4169.html
  \see WG21 N4527 20.9.2[func.require] ， WG21 N4527 20.9.3[func.invoke] 。
  \see LWG 2013 。
  \see CWG 1581 。
  \see https://llvm.org/bugs/show_bug.cgi?id=23141 。
  \todo 支持引用包装
  */
	template<typename _fCallable, typename... _tParams>
	limpl(lconstfn) result_of_t<_fCallable && (_tParams&&...)>
		invoke(_fCallable&& f, _tParams&&... args)
	{
		return details::invoke_impl(lforward(f), lforward(args)...);
	}
#endif

	namespace details
	{

		template<typename _fCallable, typename... _tParams>
		lconstfn pseudo_output
			invoke_nonvoid_impl(true_, _fCallable&& f, _tParams&&... args)
		{
			return leo::invoke(lforward(f), lforward(args)...), pseudo_output();
		}
		template<typename _fCallable, typename... _tParams>
		inline invoke_result_t<_fCallable && (_tParams&&...)>
			invoke_nonvoid_impl(false_, _fCallable&& f, _tParams&&... args)
		{
			return leo::invoke(lforward(f), lforward(args)...);
		}

	} // namespace details;

	  /*!
	  \brief 调用可调用对象，保证返回值非空。
	  */
	template<typename _fCallable, typename... _tParams>
	limpl(lconstfn) nonvoid_result_t<invoke_result_t<_fCallable && (_tParams&&...)>>
		invoke_nonvoid(_fCallable&& f, _tParams&&... args)
	{
		return details::invoke_nonvoid_impl(is_void<invoke_result_t<
			_fCallable && (_tParams&&...)>>(), lforward(f), lforward(args)...);
	}

	/*!
	\brief 使用 invoke 调用非空值或取默认值。
	\sa call_value_or
	\sa invoke
	*/
	//@{
	template<typename _type, typename _func>
	lconstfn auto
		invoke_value_or(_func f, _type&& p)
		-> decay_t<decltype(invoke(f, *lforward(p)))>
	{
		return p ? invoke(f, *lforward(p))
			: decay_t<decltype(invoke(f, *lforward(p)))>();
	}
	template<typename _tOther, typename _type, typename _func>
	lconstfn auto
		invoke_value_or(_func f, _type&& p, _tOther&& other)
		->limpl(decltype(p ? invoke(f, *lforward(p)) : lforward(other)))
	{
		return p ? invoke(f, *lforward(p)) : lforward(other);
	}
	template<typename _tOther, typename _type, typename _func,
		typename _tSentinal = stdex::nullptr_t>
		lconstfn auto
		invoke_value_or(_func f, _type&& p, _tOther&& other, _tSentinal&& last)
		->limpl(decltype(!bool(p == lforward(last))
			? invoke(f, *lforward(p)) : lforward(other)))
	{
		return !bool(p == lforward(last)) ? invoke(f, *lforward(p))
			: lforward(other);
	}
	//@}

	/*!
	\ingroup metafunctions
	\brief 取参数列表元组。
	*/
	//@{
	template<typename>
	struct make_parameter_tuple;

	template<typename _fCallable>
	using make_parameter_tuple_t = _t<make_parameter_tuple<_fCallable>>;

	template<typename _fCallable>
	struct make_parameter_tuple<_fCallable&> : make_parameter_tuple<_fCallable>
	{};

	template<typename _fCallable>
	struct make_parameter_tuple<_fCallable&&> : make_parameter_tuple<_fCallable>
	{};

#define LB_Impl_Functional_ptuple_spec(_exp, _p, _q) \
	template<typename _tRet, _exp typename... _tParams> \
	struct make_parameter_tuple<_tRet _p (_tParams...) _q> \
	{ \
		using type = std::tuple<_tParams...>; \
	};

	LB_Impl_Functional_ptuple_spec(, , )
		LB_Impl_Functional_ptuple_spec(, (*), )

#define LB_Impl_Functional_ptuple_spec_mf(_q) \
	LB_Impl_Functional_ptuple_spec(class _tClass LPP_Comma, (_tClass::*), _q)

		LB_Impl_Functional_ptuple_spec_mf()
		//@{
		LB_Impl_Functional_ptuple_spec_mf(const)
		LB_Impl_Functional_ptuple_spec_mf(volatile)
		LB_Impl_Functional_ptuple_spec_mf(const volatile)
		//@}

#undef LB_Impl_Functional_ptuple_spec_mf

#undef LB_Impl_Functional_ptuple_spec

		template<typename _tRet, typename... _tParams>
	struct make_parameter_tuple<std::function<_tRet(_tParams...)>>
	{
		using type = std::tuple<_tParams...>;
	};
	//@}


	/*!
	\ingroup metafunctions
	\brief 取返回类型。
	*/
	//@{
	template<typename>
	struct return_of;

	template<typename _fCallable>
	using return_of_t = _t<return_of<_fCallable>>;

	template<typename _fCallable>
	struct return_of<_fCallable&> : return_of<_fCallable>
	{};

	template<typename _fCallable>
	struct return_of<_fCallable&&> : return_of<_fCallable>
	{};

#define LB_Impl_Functional_ret_spec(_exp, _p, _e, _q) \
	template<typename _tRet, _exp typename... _tParams> \
	struct return_of<_tRet _p (_tParams... LPP_Args _e) _q> \
	{ \
		using type = _tRet; \
	};

#define LB_Impl_Functional_ret_spec_f(_e) \
	LB_Impl_Functional_ret_spec(, , _e, ) \
	LB_Impl_Functional_ret_spec(, (*), _e, )

	LB_Impl_Functional_ret_spec_f()
		LB_Impl_Functional_ret_spec_f((, ...))

#undef LB_Impl_Functional_ret_spec_f

#define LB_Impl_Functional_ret_spec_mf(_e, _q) \
	LB_Impl_Functional_ret_spec(class _tClass LPP_Comma, (_tClass::*), \
		_e, _q)

#define LB_Impl_Functional_ret_spec_mfq(_e) \
	LB_Impl_Functional_ret_spec_mf(_e, ) \
	LB_Impl_Functional_ret_spec_mf(_e, const) \
	LB_Impl_Functional_ret_spec_mf(_e, volatile) \
	LB_Impl_Functional_ret_spec_mf(_e, const volatile)


		LB_Impl_Functional_ret_spec_mfq()
		LB_Impl_Functional_ret_spec_mfq((, ...))

#undef LB_Impl_Functional_ret_spec_mfq
#undef LB_Impl_Functional_ret_spec_mf

#undef LB_Impl_Functional_ret_spec

		template<typename _tRet, typename... _tParams>
	struct return_of<std::function<_tRet(_tParams...)>>
	{
		using type = _tRet;
	};
	//@}


	/*!
	\ingroup metafunctions
	\brief 取指定索引的参数类型。
	*/
	//@{
	template<size_t _vIdx, typename _fCallable>
	struct parameter_of
	{
		using type = tuple_element_t<_vIdx,
			_t<make_parameter_tuple<_fCallable>>>;
	};

	template<size_t _vIdx, typename _fCallable>
	using parameter_of_t = _t<parameter_of<_vIdx, _fCallable>>;
	//@}


	/*!
	\ingroup metafunctions
	\brief 取参数列表大小。
	*/
	template<typename _fCallable>
	struct paramlist_size : size_t_<std::tuple_size<typename
		make_parameter_tuple<_fCallable>::type>::value>
	{};


	/*!
	\ingroup metafunctions
	*/
	//@{
	//! \brief 取指定返回类型和元组指定参数类型的函数类型。
	//@{
	template<typename, class>
	struct make_function_type;

	template<typename _tRet, class _tTuple>
	using make_function_type_t = _t<make_function_type<_tRet, _tTuple>>;

	template<typename _tRet, typename... _tParams>
	struct make_function_type<_tRet, std::tuple<_tParams...>>
	{
		using type = _tRet(_tParams...);
	};
	//@}


	/*!
	\brief 启用备用重载。
	*/
	template<template<typename...> class _gOp, typename _func, typename... _tParams>
	using enable_fallback_t = enable_if_t<!is_detected<_gOp, _tParams&&...>::value,
		decltype(std::declval<_func>()(std::declval<_tParams&&>()...))>;


	//! \brief 取指定维数和指定参数类型的多元映射扩展恒等函数类型。
	template<typename _type, size_t _vN = 1, typename _tParam = _type>
	using id_func_t
		= make_function_type_t<_type, vseq::join_n_t<_vN, std::tuple<_tParam>>>;

	//! \brief 取指定维数和 const 左值引用参数类型的多元映射扩展恒等函数类型。
	template<typename _type, size_t _vN = 1>
	using id_func_clr_t = id_func_t<_type, _vN, const _type&>;

	//! \brief 取指定维数和 const 右值引用参数类型的多元映射扩展恒等函数类型。
	template<typename _type, size_t _vN = 1>
	using id_func_rr_t = id_func_t<_type, _vN, _type&&>;
	//@}


	/*!
	\brief 复合调用 std::bind 和 std::placeholders::_1 。
	\note ISO C++ 要求 std::placeholders::_1 被实现支持。
	*/
	//@{
	template<typename _func, typename... _tParams>
	inline auto
		bind1(_func&& f, _tParams&&... args) -> decltype(
			std::bind(lforward(f), std::placeholders::_1, lforward(args)...))
	{
		return std::bind(lforward(f), std::placeholders::_1, lforward(args)...);
	}
	template<typename _tRes, typename _func, typename... _tParams>
	inline auto
		bind1(_func&& f, _tParams&&... args) -> decltype(
			std::bind<_tRes>(lforward(f), std::placeholders::_1, lforward(args)...))
	{
		return std::bind<_tRes>(lforward(f), std::placeholders::_1, lforward(args)...);
	}
	//@}

	/*!
	\brief 复合调用 leo::bind1 和 std::placeholders::_2 以实现值的设置。
	\note 从右到左逐个应用参数。
	\note ISO C++ 要求 std::placeholders::_2 被实现支持。
	*/
	template<typename _func, typename _func2, typename... _tParams>
	inline auto
		bind_forward(_func&& f, _func2&& f2, _tParams&&... args)
		-> decltype(leo::bind1(lforward(f), std::bind(lforward(f2),
			std::placeholders::_2, lforward(args)...)))
	{
		return leo::bind1(lforward(f), std::bind(lforward(f2),
			std::placeholders::_2, lforward(args)...));
	}


	//@{
	//! \brief 复合函数。
	template<typename _func, typename _func2>
	struct composed
	{
		_func f;
		_func2 g;

		/*!
		\note 每个函数只在函数调用表达式中出现一次。
		*/
		template<typename... _tParams>
		lconstfn auto
			operator()(_tParams&&... args) const lnoexcept_spec(f(g(lforward(args)...)))
			-> decltype(f(g(lforward(args)...)))
		{
			return f(g(lforward(args)...));
		}
	};

	/*!
	\brief 函数复合。
	\note 最后一个参数最先被调用，可以为多元函数；其它被复合的函数需要保证有一个参数。
	\relates composed
	\return 复合的可调用对象。
	*/
	//@{
	template<typename _func, typename _func2>
	lconstfn composed<_func, _func2>
		compose(_func f, _func2 g)
	{
		return composed<_func, _func2>{f, g};
	}
	template<typename _func, typename _func2, typename _func3, typename... _funcs>
	lconstfn auto
		compose(_func f, _func2 g, _func3 h, _funcs... args)
		-> decltype(leo::compose(leo::compose(f, g), h, args...))
	{
		return leo::compose(leo::compose(f, g), h, args...);
	}
	//@}
	//@}


	//@{
	//! \brief 多元分发的复合函数。
	template<typename _func, typename _func2>
	struct composed_n
	{
		_func f;
		_func2 g;

		//! \note 第二函数会被分发：多次出现在函数调用表达式中。
		template<typename... _tParams>
		lconstfn auto
			operator()(_tParams&&... args) const lnoexcept_spec(f(g(lforward(args))...))
			-> decltype(f(g(lforward(args))...))
		{
			return f(g(lforward(args))...);
		}
	};

	/*!
	\brief 单一分派的多元函数复合。
	\note 第一个参数最后被调用，可以为多元函数；其它被复合的函数需要保证有一个参数。
	\relates composed_n
	\return 单一分派的多元复合的可调用对象。
	*/
	//@{
	template<typename _func, typename _func2>
	lconstfn composed_n<_func, _func2>
		compose_n(_func f, _func2 g)
	{
		return composed_n<_func, _func2>{f, g};
	}
	template<typename _func, typename _func2, typename _func3, typename... _funcs>
	lconstfn auto
		compose_n(_func f, _func2 g, _func3 h, _funcs... args)
		-> decltype(leo::compose_n(leo::compose_n(f, g), h, args...))
	{
		return leo::compose_n(leo::compose_n(f, g), h, args...);
	}
	//@}


	//! \brief 多元复合函数。
	template<typename _func, typename... _funcs>
	struct generalized_composed
	{
		_func f;
		std::tuple<_funcs...> g;

		template<typename... _tParams>
		lconstfn auto
			operator()(_tParams&&... args) const lnoexcept_spec(limpl(call(
				index_sequence_for<_tParams...>(), lforward(args)...))) -> decltype(
					limpl(call(index_sequence_for<_tParams...>(), lforward(args)...)))
		{
			return call(index_sequence_for<_tParams...>(), lforward(args)...);
		}

	private:
		template<size_t... _vSeq, typename... _tParams>
		lconstfn auto
			call(index_sequence<_vSeq...>, _tParams&&... args) const
			lnoexcept_spec(f(std::get<_vSeq>(g)(lforward(args))...))
			-> decltype(f(std::get<_vSeq>(g)(lforward(args))...))
		{
			return f(std::get<_vSeq>(g)(lforward(args))...);
		}
	};

	/*!
	\brief 多元函数复合。
	\relates generalized_composed
	\return 以多元函数复合的可调用对象。
	*/
	template<typename _func, typename... _funcs>
	lconstfn generalized_composed<_func, std::tuple<_funcs...>>
		generalized_compose(_func f, _funcs... args)
	{
		return generalized_composed<_func,
			std::tuple<_funcs...>>{f, make_tuple(args...)};
	}
	//@}


	/*!
	\brief 调用一次的函数包装模板。
	\pre 静态断言：函数对象和结果转移以及默认状态构造和状态交换不抛出异常。
	\todo 优化 std::function 等可空类型的实现。
	\todo 复用静态断言。
	\todo 简化转移实现。
	*/
	//@{
	template<typename _func, typename _tRes = void, typename _tState = bool>
	struct one_shot
	{
		static_assert(is_nothrow_move_constructible<_func>::value,
			"Invalid target type found.");
		static_assert(is_nothrow_move_constructible<_tRes>::value,
			"Invalid result type found.");
		static_assert(is_nothrow_default_constructible<_tState>::value,
			"Invalid state type found.");
		static_assert(is_nothrow_swappable<_tState>::value ,
			"Invalid state type found.");

		_func func;
		mutable _tRes result;
		mutable _tState fresh{};

		one_shot(_func f, _tRes r = {}, _tState s = {})
			: func(f), result(r), fresh(s)
		{}
		one_shot(one_shot&& f) lnothrow
			: func(std::move(f.func)), result(std::move(f.result))
		{
			using std::swap;

			swap(fresh, f.fresh);
		}

		template<typename... _tParams>
		lconstfn_relaxed auto
			operator()(_tParams&&... args) const
			lnoexcept(noexcept(func(lforward(args)...)))
			-> decltype(func(lforward(args)...))
		{
			if (fresh)
			{
				result = func(lforward(args)...);
				fresh = {};
			}
			return std::forward<_tRes>(result);
		}
	};

	template<typename _func, typename _tState>
	struct one_shot<_func, void, _tState>
	{
		static_assert(is_nothrow_move_constructible<_func>::value,
			"Invalid target type found.");
		static_assert(is_nothrow_default_constructible<_tState>::value,
			"Invalid state type found.");
		static_assert(is_nothrow_swappable<_tState>::value,
			"Invalid state type found.");

		_func func;
		mutable _tState fresh{};

		one_shot(_func f, _tState s = {})
			: func(f), fresh(s)
		{}
		one_shot(one_shot&& f) lnothrow
			: func(std::move(f.func))
		{
			using std::swap;

			swap(fresh, f.fresh);
		}

		template<typename... _tParams>
		lconstfn_relaxed void
			operator()(_tParams&&... args) const
			lnoexcept(noexcept(func(lforward(args)...)))
		{
			if (fresh)
			{
				func(lforward(args)...);
				fresh = {};
			}
		}
	};

	template<typename _func, typename _tRes>
	struct one_shot<_func, _tRes, void>
	{
		static_assert(is_nothrow_move_constructible<_func>::value,
			"Invalid target type found.");
		static_assert(is_nothrow_move_constructible<_tRes>::value,
			"Invalid result type found.");


		mutable _func func;
		mutable _tRes result;

		one_shot(_func f, _tRes r = {})
			: func(f), result(r)
		{}

		template<typename... _tParams>
		lconstfn_relaxed auto
			operator()(_tParams&&... args) const
			lnoexcept(noexcept(func(lforward(args)...)))
			-> decltype(func(lforward(args)...) && noexcept(func = {}))
		{
			if (func)
			{
				result = func(lforward(args)...);
				func = {};
			}
			return std::forward<_tRes>(result);
		}
	};

	template<typename _func>
	struct one_shot<_func, void, void>
	{
		static_assert(is_nothrow_move_constructible<_func>::value,
			"Invalid target type found.");

		mutable _func func;

		one_shot(_func f)
			: func(f)
		{}

		template<typename... _tParams>
		lconstfn_relaxed void
			operator()(_tParams&&... args) const
			lnoexcept(noexcept(func(lforward(args)...) && noexcept(func = {})))
		{
			if (func)
			{
				func(lforward(args)...);
				func = {};
			}
		}
	};
	//@}


	/*!
	\ingroup functors
	\brief get 成员小于仿函数。
	*/
	template<typename _type>
	using get_less
		= composed_n<less<_type*>, composed<addressof_op<_type>, mem_get<>>>;


	namespace details
	{

		template<typename _type, typename _fCallable, typename... _tParams>
		_type
			call_for_value(true_, _type&& val, _fCallable&& f, _tParams&&... args)
		{
			leo::invoke(lforward(f), lforward(args)...);
			return lforward(val);
		}

		template<typename _type, typename _fCallable, typename... _tParams>
		auto
			call_for_value(false_, _type&&, _fCallable&& f, _tParams&&... args)
			-> invoke_result_t<_fCallable && (_tParams&&...)>
		{
			return leo::invoke(lforward(f), lforward(args)...);
		}

	} // unnamed namespace;

	  /*!
	  \brief 调用第二个参数起指定的函数对象，若返回空类型则使用第一个参数的值为返回值。
	  */
	template<typename _type, typename _fCallable, typename... _tParams>
	auto
		call_for_value(_type&& val, _fCallable&& f, _tParams&&... args)
		-> common_nonvoid_t<invoke_result_t<_fCallable && (_tParams&&...)>, _type>
	{
		return details::call_for_value(is_void<invoke_result_t<_fCallable && (
			_tParams&&...)>>(), lforward(val), lforward(f), lforward(args)...);
	}


	/*!
	\brief 调用投影：向原调用传递序列指定的位置的参数。
	*/
	//@{
	template<class, class>
	struct call_projection;

	template<typename _tRet, typename... _tParams, size_t... _vSeq>
	struct call_projection<_tRet(_tParams...), index_sequence<_vSeq...>>
	{
		template<typename _func>
		static lconstfn auto
			call(_func&& f, std::tuple<_tParams...>&& args, limpl(decay_t<
				decltype(lforward(f)(std::get<_vSeq>(std::move(args))...))>* = {}))
			-> decltype(lforward(f)(std::get<_vSeq>(std::move(args))...))
		{
			return lforward(f)(std::get<_vSeq>(lforward(args))...);
		}
		//@{
		template<typename _func>
		static lconstfn auto
			call(_func&& f, _tParams&&... args)
			-> decltype(call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(args)...)))
		{
			return call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(args)...));
		}

		template<typename _fCallable>
		static lconstfn auto
			invoke(_fCallable&& f, std::tuple<_tParams...>&& args,
				limpl(decay_t<decltype(leo::invoke(lforward(f),
					std::get<_vSeq>(std::move(args))...))>* = {})) -> decltype(
						leo::invoke(lforward(f), std::get<_vSeq>(lforward(args))...))
		{
			return leo::invoke(lforward(f), std::get<_vSeq>(lforward(args))...);
		}
		template<typename _func>
		static lconstfn auto
			invoke(_func&& f, _tParams&&... args)
			-> decltype(call_projection::invoke(lforward(f),
				std::forward_as_tuple(lforward(args)...)))
		{
			return call_projection::invoke(lforward(f),
				std::forward_as_tuple(lforward(args)...));
		}
		//@}
	};

	template<typename _tRet, typename... _tParams, size_t... _vSeq>
	struct call_projection<std::function<_tRet(_tParams...)>,
		index_sequence<_vSeq...>> : private
		call_projection<_tRet(_tParams...), index_sequence<_vSeq...>>
	{
		using call_projection<_tRet(_tParams...), index_sequence<_vSeq...>>::call;
		using
			call_projection<_tRet(_tParams...), index_sequence<_vSeq...>>::invoke;
	};

	/*!
	\note 不需要显式指定返回类型。
	*/
	template<typename... _tParams, size_t... _vSeq>
	struct call_projection<std::tuple<_tParams...>, index_sequence<_vSeq...>>
	{
		template<typename _func>
		static lconstfn auto
			call(_func&& f, std::tuple<_tParams...>&& args)
			-> decltype(lforward(f)(std::get<_vSeq>(std::move(args))...))
		{
			return lforward(f)(std::get<_vSeq>(lforward(args))...);
		}

		//@{
		template<typename _func>
		static lconstfn auto
			call(_func&& f, _tParams&&... args)
			-> decltype(call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(std::move(args))...)))
		{
			return call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(lforward(args))...));
		}

		template<typename _fCallable>
		static lconstfn auto
			invoke(_fCallable&& f, std::tuple<_tParams...>&& args)
			-> decltype(leo::invoke(lforward(f), std::get<_vSeq>(args)...))
		{
			return leo::invoke(lforward(f), std::get<_vSeq>(args)...);
		}
		template<typename _func>
		static lconstfn auto
			invoke(_func&& f, _tParams&&... args)
			-> decltype(call_projection::invoke(lforward(f),
				std::forward_as_tuple(lforward(args)...)))
		{
			return call_projection::invoke(lforward(f),
				std::forward_as_tuple(lforward(args)...));
		}
		//@}
	};
	//@}


	/*!
	\brief 应用函数对象和参数元组。
	\tparam _func 函数对象及其引用类型。
	\tparam _tTuple 元组及其引用类型。
	\see WG21 N4606 20.5.2.5[tuple.apply]/1 。
	\see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4023.html#tuple.apply 。
	*/
	template<typename _func, class _tTuple>
	lconstfn auto
		apply(_func&& f, _tTuple&& args)
		->limpl(decltype(call_projection<_tTuple, make_index_sequence<
			std::tuple_size<decay_t<_tTuple>>::value>>::call(lforward(f),
				lforward(args))))
	{
		return call_projection<_tTuple, make_index_sequence<std::tuple_size<
			decay_t<_tTuple>>::value>>::call(lforward(f), lforward(args));
	}


	//@{
	template<typename _fCallable, size_t _vLen = paramlist_size<_fCallable>::value>
	struct expand_proxy : private call_projection<_fCallable,
		make_index_sequence<_vLen>>, private expand_proxy<_fCallable, _vLen - 1>
	{
		/*!
		\see CWG 1393 。
		\see EWG 102 。
		*/
		using call_projection<_fCallable, make_index_sequence<_vLen>>::call;
		/*!
		\note 为避免歧义，不直接使用 using 声明。
		*/
		template<typename... _tParams>
		static auto
			call(_tParams&&... args) -> decltype(
				expand_proxy<_fCallable, _vLen - 1>::call(lforward(args)...))
		{
			return expand_proxy<_fCallable, _vLen - 1>::call(lforward(args)...);
		}
	};

	template<typename _fCallable>
	struct expand_proxy<_fCallable, 0>
		: private call_projection<_fCallable, index_sequence<>>
	{
		using call_projection<_fCallable, index_sequence<>>::call;
	};
	//@}


	/*!
	\brief 循环重复调用：代替直接使用 do-while 语句以避免过多引入作用域外的变量。
	\tparam _fCond 判断条件。
	\tparam _fCallable 可调用对象类型。
	\tparam _tParams 参数类型。
	\note 条件接受调用结果或没有参数。
	\sa object_result_t
	*/
	template<typename _fCond, typename _fCallable, typename... _tParams>
	invoke_result_t<_fCallable && (_tParams&&...)>
		retry_on_cond(_fCond cond, _fCallable&& f, _tParams&&... args)
	{
		using res_t = invoke_result_t<_fCallable && (_tParams&&...)>;
		using obj_t = object_result_t<res_t>;
		obj_t res;

		do
			res = leo::invoke_nonvoid(lforward(f), lforward(args)...);
		while (expand_proxy<bool(obj_t&)>::call(cond, res));
		return res_t(res);
	}


	/*!
	\brief 接受冗余参数的可调用对象。
	\todo 支持 ref-qualifier 。
	*/
	template<typename _fHandler, typename _fCallable>
	struct expanded_caller
	{
		static_assert(is_object<_fCallable>::value, "Callable object type is needed.");

		_fCallable caller;

		template<typename _fCaller,
			limpl(typename = exclude_self_t<expanded_caller, _fCaller>)>
			expanded_caller(_fCaller&& f)
			: caller(lforward(f))
		{}

		template<typename... _tParams>
		auto
			operator()(_tParams&&... args) const -> decltype(
				expand_proxy<_fHandler>::call(caller, lforward(args)...))
		{
			return expand_proxy<_fHandler>::call(caller, lforward(args)...);
		}
	};

	/*!
	\ingroup helper_functions
	\brief 构造接受冗余参数的可调用对象。
	\relates expanded_caller
	*/
	template<typename _fHandler, typename _fCallable>
	lconstfn expanded_caller<_fHandler, decay_t<_fCallable>>
		make_expanded(_fCallable&& f)
	{
		return expanded_caller<_fHandler, decay_t<_fCallable>>(lforward(f));
	}


	/*!
	\brief 合并值序列。
	\note 语义同 Boost.Signal2 的 \c boost::last_value 但对非 \c void 类型使用默认值。
	*/
	//@{
	template<typename _type>
	struct default_last_value
	{
		template<typename _tIn>
		_type
			operator()(_tIn first, _tIn last, const _type& val = {}) const
		{
			return std::accumulate(first, last, val,
				[](_type&, decltype(*first) v) {
				return lforward(v);
			});
		}
	};

	template<>
	struct default_last_value<void>
	{
		template<typename _tIn>
		void
			operator()(_tIn first, _tIn last) const
		{
			for (; first != last; ++first)
				*first;
		}
	};
	//@}

} // namespace leo;


#ifdef LB_IMPL_MSCPP
#pragma warning(pop)
#endif
#endif