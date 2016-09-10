/*!	\file functional.hpp
\ingroup LBase
\brief �����Ϳɵ��ö���
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
	//! \since build 1.4
	//@{
	namespace details
	{

		template<class, class, class>
		struct tuple_element_convertible;

		template<class _type1, class _type2>
		struct tuple_element_convertible<_type1, _type2, index_sequence<>>
			: true_type
		{};

		template<typename... _types1, typename... _types2, size_t... _vSeq,
			size_t _vHead>
		struct tuple_element_convertible<std::tuple<_types1...>, std::tuple<_types2...>,
			index_sequence<_vHead, _vSeq... >>
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
	\since build 1.4
	*/
	//@{
	//! \brief �ж�ָ������֮���Ƿ�Э�䡣
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
		: bool_constant<details::tuple_element_convertible<std::tuple<_tFroms...>,
		std::tuple<_tTos...>, index_sequence_for<_tTos... >> ::value>
	{};

	//! \since build 1.4
	template<typename _tFrom, typename _tTo, typename... _tFromParams,
		typename... _tToParams>
	struct is_covariant<std::function<_tFrom(_tFromParams...)>,
		std::function<_tTo(_tToParams...) >>
		: is_covariant<_tFrom(_tFromParams...), _tTo(_tToParams...)>
	{};
	//@}


	//! \brief �ж�ָ������֮���Ƿ���䡣
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
		: bool_constant<details::tuple_element_convertible<std::tuple<_tTos...>,
		std::tuple<_tFroms...>, index_sequence_for<_tTos... >> ::value>
	{};

	//! \since build 1.4
	template<typename _tResFrom, typename _tResTo, typename... _tFromParams,
		typename... _tToParams>
	struct is_contravariant<std::function<_tResFrom(_tFromParams...)>,
		std::function<_tResTo(_tToParams...) >>
		: is_contravariant<_tResFrom(_tFromParams...), _tResTo(_tToParams...)>
	{};
	//@}
	//@}

	template<typename... _tParams>
	lconstfn size_t
		//! \brief ͳ�ƺ��������б��еĲ���������
		sizeof_params(_tParams&&...)
	{
		return sizeof...(_tParams);
	}

	template<size_t _vN>
	//! \brief�䳤��������ģ��
	struct variadic_param
	{
		template<typename _type, typename... _tParams>
		lconstfn static auto get(_type&&, _tParams&&... args) -> decltype(variadic_param<_vN - 1>::get(lforward(args)...))
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
		lconstfn static auto get(_type&& arg) -> decltype(lforward(arg))
		{
			return lforward(arg);
		}
	};

	template<size_t _vN, typename... _tParams>
	lconstexpr
		//\brief ȡָ��λ�õı䳤������
		//\tparam _vN ��ʾ����λ�õķǸ���������ʼ��������һ������Ϊ 0 ��
		auto varg(_tParams&&... args) -> decltype(variadic_param<_vN>::get(lforward(args)...))
	{
		static_assert(_vN < sizeof...(args),
			"Out-of-range index of variadic argument found.");

		return variadic_param<_vN>::get(lforward(args)...);
	}

	//! \see ���ڵ��ò������ͣ� ISO C++11 30.3.1.2 [thread.thread.constr] ��
	//@{
	//! \brief ˳����ʽ���á�
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
		lnoexcept_spec(chain_apply(
			lforward(lforward(f)(lforward(arg))), lforward(args)...))
	{
		return chain_apply(lforward(lforward(f)(lforward(arg))),
			lforward(args)...);
	}
	//@}
	template<typename _fCallable>
		inline void
		seq_apply(_fCallable&&)
	{}
	template<typename _fCallable, typename _type, typename... _tParams>
	inline void
		//˳�����
		seq_apply(_fCallable&& f, _type&& arg, _tParams&&... args)
	{
		lforward(f)(lforward(arg));
		leo::seq_apply(lforward(f), lforward(args)...);
	}

	template<typename _fCallable, typename... _tParams>
	inline void
		unseq_apply(_fCallable&& f, _tParams&&... args)
	{
		lunseq((void(lforward(f)(lforward(args))), 0)...);
	}
	//@}
	//@}

#if __cpp_lib_invoke >= 201411
	using std::invoke;
#else
#endif

	namespace details
	{
		template<typename _fCallable, typename... _tParams>
		lconstfn pseudo_output
			invoke_nonvoid_impl(true_type, _fCallable&& f, _tParams&&... args)
		{
			return invoke(lforward(f), lforward(args)...), pseudo_output();
		}
		template<typename _fCallable, typename... _tParams>
		inline result_of_t<_fCallable && (_tParams&&...)>
			invoke_nonvoid_impl(false_type, _fCallable&& f, _tParams&&... args)
		{
			return invoke(lforward(f), lforward(args)...);
		}

	} // namespace details;

	  /*!
	  \brief ���ÿɵ��ö��󣬱�֤����ֵ�ǿա�
	  \since build 1.4
	  */
	template<typename _fCallable, typename... _tParams>
	limpl(lconstfn) nonvoid_result_t<result_of_t<_fCallable && (_tParams&&...)>>
		invoke_nonvoid(_fCallable&& f, _tParams&&... args)
	{
		return details::invoke_nonvoid_impl(is_void<result_of_t<
			_fCallable && (_tParams&&...) >> (), lforward(f), lforward(args)...);
	}

	template<typename>
	//ȡ�����б�Ԫ��
	struct make_parameter_tuple;

	template<typename _fCallable>
	using make_parameter_tuple_t = _t<make_parameter_tuple<_fCallable>>;

	//! \since build 1.4
	template<typename _fCallable>
	struct make_parameter_tuple<_fCallable&> : make_parameter_tuple<_fCallable>
	{};

	//! \since build 1.4
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
		//! \since build 1.4
		//@{
		LB_Impl_Functional_ptuple_spec_mf(const)
		LB_Impl_Functional_ptuple_spec_mf(volatile)
		LB_Impl_Functional_ptuple_spec_mf(const volatile)
		//@}

#undef LB_Impl_Functional_ptuple_spec_mf

#undef LB_Impl_Functional_ptuple_spec

	template<typename _tRet, typename... _tParams>
	//��������
	struct make_parameter_tuple<std::function<_tRet(_tParams...)>>
	{
		using type = std::tuple<_tParams...>;
	};

	template<typename>
	//ȡ��������
	struct return_of;

	template<typename _fCallable>
	using return_of_t = _t<return_of<_fCallable>>;

	//! \since build 1.4
	template<typename _fCallable>
	struct return_of<_fCallable&> : return_of<_fCallable>
	{};

	//! \since build 1.4
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
		//! \since build 675
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
		//! \since build 675
		LB_Impl_Functional_ret_spec_mfq((, ...))

#undef LB_Impl_Functional_ret_spec_mfq
#undef LB_Impl_Functional_ret_spec_mf

#undef LB_Impl_Functional_ret_spec

	//! \since build 1.4
	template<typename _tRet, typename... _tParams>
	struct return_of<std::function<_tRet(_tParams...)>>
	{
		using type = _tRet;
	};
	//@}

	template<size_t _vIdx, typename _fCallable>
	//ȡָ�������Ĳ�������
	struct parameter_of
	{
		using type = tuple_element_t<_vIdx,
			_t<make_parameter_tuple<_fCallable>>>;
	};

	template<size_t _vIdx, typename _fCallable>
	using parameter_of_t = _t<parameter_of<_vIdx, _fCallable>>;

	template<typename _fCallable>
	//ȡ�����б���С
	struct paramlist_size : integral_constant<size_t, std::tuple_size<typename
		make_parameter_tuple<_fCallable>::type>::value>
	{};

	/*!
	\ingroup metafunctions
	\since build 1.4
	*/
	//@{
	//! \brief ȡָ���������ͺ�Ԫ��ָ���������͵ĺ������͡�
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
	\brief ���ñ������ء�
	\since build 1.4
	*/
	template<template<typename...> class _gOp, typename _func, typename... _tParams>
	using enable_fallback_t = enable_if_t<!is_detected<_gOp, _tParams&&...>::value,
		decltype(std::declval<_func>()(std::declval<_tParams&&>()...))>;


	//! \brief ȡָ��ά����ָ���������͵Ķ�Ԫӳ����չ��Ⱥ������͡�
	template<typename _type, size_t _vN = 1, typename _tParam = _type>
	using id_func_t
		= make_function_type_t<_type, vseq::join_n_t<_vN, std::tuple<_tParam>>>;

	//! \brief ȡָ��ά���� const ��ֵ���ò������͵Ķ�Ԫӳ����չ��Ⱥ������͡�
	template<typename _type, size_t _vN = 1>
	using id_func_clr_t = id_func_t<_type, _vN, const _type&>;

	//! \brief ȡָ��ά���� const ��ֵ���ò������͵Ķ�Ԫӳ����չ��Ⱥ������͡�
	template<typename _type, size_t _vN = 1>
	using id_func_rr_t = id_func_t<_type, _vN, _type&&>;
	//@}

	/*!
	\brief ���ϵ��� std::bind �� std::placeholders::_1 ��
	\note ISO C++ Ҫ�� std::placeholders::_1 ��ʵ��֧�֡�
	*/
	//@{
	//! \since build 1.4
	template<typename _func, typename... _tParams>
	inline decltype(auto)	
		bind1(_func&& f, _tParams&&... args)
	{
		return std::bind(lforward(f), std::placeholders::_1, lforward(args)...);
	}
	//! \since build 1.4
	template<typename _tRes, typename _func, typename... _tParams>
	inline decltype(auto)
		bind1(_func&& f, _tParams&&... args)
	{
		return std::bind<_tRes>(lforward(f), std::placeholders::_1, lforward(args)...);
	}
	//@}

	/*!
	\brief ���ϵ��� ystdex::bind1 �� std::placeholders::_2 ��ʵ��ֵ�����á�
	\note ���ҵ������Ӧ�ò�����
	\note ISO C++ Ҫ�� std::placeholders::_2 ��ʵ��֧�֡�
	\since build 1.4
	*/
	template<typename _func, typename _func2, typename... _tParams>
	inline decltype(auto)
		bind_forward(_func&& f, _func2&& f2, _tParams&&... args)
	{
		return bind1(lforward(f), std::bind(lforward(f2),
			std::placeholders::_2, lforward(args)...));
	}

	//! \since build 1.4
	//@{
	//! \brief ���Ϻ�����
	template<typename _func1, typename _func2>
	struct composed
	{
		_func1 f;
		_func2 g;

		template<typename... _tParams>
		lconstfn auto
			operator()(_tParams&&... args) const -> decltype(f(g(lforward(args))...))
		{
			return f(g(lforward(args))...);
		}
	};

	/*!
	\brief �������ϡ�
	\note ��һ��������󱻵��ã�����Ϊ��Ԫ���������������ϵĺ�����Ҫ��֤��һ��������
	\relates composed
	\return ���ϵĿɵ��ö���
	*/
	template<typename _func1, typename _func2>
	composed<_func1, _func2>
		compose(_func1 f, _func2 g)
	{
		return composed<_func1, _func2>{f, g};
	}
	template<typename _func1, typename _func2, typename... _funcs>
	lconstfn auto
		compose(_func1 f, _func2 g, _funcs... args)
		-> decltype(compose(compose(f, g), args...))
	{
		return compose(compose(f, g), args...);
	}
	//@}


	/*!
	\brief ����һ�εĺ�����װģ�塣
	\pre ��̬���ԣ���������ͽ��ת���Լ�Ĭ��״̬�����״̬�������׳��쳣��
	\since build 1.4
	\todo �Ż� std::function �ȿɿ����͵�ʵ�֡�
	\todo ���þ�̬���ԡ�
	\todo ��ת��ʵ�֡�
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
		static_assert(is_nothrow_swappable<_tState>::value,
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
	\brief get ��ԱС�ڷº�����
	\since build 1.4
	*/
	template<typename _type>
	using get_less
		= composed<less<_type*>, composed<addressof_op<_type>, mem_get<>>>;


	//! \since build 1.4
	namespace details
	{

		template<typename _type, typename _fCallable, typename... _tParams>
		_type
			call_for_value(std::true_type, _type&& val, _fCallable&& f, _tParams&&... args)
		{
			invoke(lforward(f), lforward(args)...);
			return lforward(val);
		}

		template<typename _type, typename _fCallable, typename... _tParams>
		auto
			call_for_value(std::false_type, _type&&, _fCallable&& f, _tParams&&... args)
			-> result_of_t<_fCallable && (_tParams&&...)>
		{
			return invoke(lforward(f), lforward(args)...);
		}

	} // unnamed namespace;

	  /*!
	  \brief ���õڶ���������ָ���ĺ������������ؿ�������ʹ�õ�һ��������ֵΪ����ֵ��
	  \since build 1.4
	  */
	template<typename _type, typename _fCallable, typename... _tParams>
	auto
		call_for_value(_type&& val, _fCallable&& f, _tParams&&... args)
		-> common_nonvoid_t<result_of_t<_fCallable && (_tParams&&...)>, _type>
	{
		return details::call_for_value(bool_constant<
			is_void<result_of_t<_fCallable && (_tParams&&...)>>::value>(),
			lforward(val), lforward(f), lforward(args)...);
	}

	template<typename, class>
	//����ͶӰ:��ԭ���ô�������ָ����λ�õĲ���.
	struct call_projection;

	template<typename _tRet, typename... _tParams, size_t... _vSeq>
	struct call_projection<_tRet(_tParams...), index_sequence<_vSeq...>>
	{
		template<typename _fCallable>
		static _tRet
			call(_fCallable&& f, std::tuple<_tParams...>&& args, limpl(decay_t<
				decltype(lforward(f)(std::get<_vSeq>(std::move(args))...))>* = {}))
		{
			lforward(f)(lforward(std::get<_vSeq>(std::move(args)))...);
		}

		//@{
		template<typename _func>
		static auto
			call(_func&& f, _tParams&&... args)
			-> decltype(call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(args)...)))
		{
			return call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(args)...));
		}

		template<typename _fCallable>
		static _tRet
			invoke(_fCallable&& f, std::tuple<_tParams...>&& args,
				limpl(decay_t<decltype(invoke(lforward(f),
					std::get<_vSeq>(std::move(args))...))>* = {}))
		{
			return invoke(lforward(f), std::get<_vSeq>(std::move(args))...);
		}
		template<typename _func>
		static auto
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
		index_sequence<_vSeq... >> : private
		call_projection<_tRet(_tParams...), index_sequence<_vSeq...>>
	{
		using
		call_projection<_tRet(_tParams...), index_sequence<_vSeq...>>::call;
		using
			call_projection<_tRet(_tParams...), index_sequence<_vSeq...>>::invoke;
	};

	/*!
	\note ����Ҫ��ʽָ���������͡�
	\since build 1.4
	*/
	template<typename... _tParams, size_t... _vSeq>
	struct call_projection<std::tuple<_tParams...>, index_sequence<_vSeq...>>
	{
		template<typename _func>
		static auto
			call(_func&& f, std::tuple<_tParams...>&& args)
			-> decltype(lforward(f)(std::get<_vSeq>(std::move(args))...))
		{
			return lforward(f)(std::get<_vSeq>(std::move(args))...);
		}

		//! \since build 1.4
		//@{
		template<typename _func>
		static auto
			call(_func&& f, _tParams&&... args)
			-> decltype(call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(std::move(args))...)))
		{
			return call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(std::move(args))...));
		}

		template<typename _fCallable>
		static auto
			invoke(_fCallable&& f, std::tuple<_tParams...>&& args)
			-> decltype(invoke(lforward(f), std::get<_vSeq>(args)...))
		{
			return invoke(lforward(f), std::get<_vSeq>(args)...);
		}
		template<typename _func>
		static auto
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
	\brief Ӧ�ú�������Ͳ���Ԫ�顣
	\tparam _func �����������������͡�
	\tparam _tTuple Ԫ�鼰���������͡�
	\see WG21 N3936 20.5.1[intseq.general] ��
	\see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4023.html#tuple.apply ��
	\since build 1.4
	*/
	template<typename _func, class _tTuple>
	inline auto
		apply(_func&& f, _tTuple&& args)
		->limpl(decltype(call_projection<_tTuple, make_index_sequence<
			std::tuple_size<decay_t<_tTuple>>::value >> ::call(lforward(f),
				lforward(args))))
	{
		return call_projection<_tTuple, make_index_sequence<std::tuple_size<
			decay_t<_tTuple >> ::value >> ::call(lforward(f), lforward(args));
	}

	//! \since build 1.4
	//@{
	template<typename _fCallable, size_t _vLen = paramlist_size<_fCallable>::value>
	struct expand_proxy : private call_projection<_fCallable,
		make_index_sequence<_vLen >> , private expand_proxy<_fCallable, _vLen - 1>
	{
		using call_projection<_fCallable, make_index_sequence<_vLen>>::call;
		/*!
		\note Ϊ�������壬��ֱ��ʹ�� using ������
		\since build 657
		*/
		template<typename... _tParams>
		static auto
			call(_tParams&&... args)
			-> decltype(expand_proxy<_fCallable, _vLen - 1>::call(lforward(args)...))
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
	\brief ѭ���ظ����ã�����ֱ��ʹ�� do-while ����Ա������������������ı�����
	\tparam _fCond �ж�������
	\tparam _fCallable �ɵ��ö������͡�
	\tparam _tParams �������͡�
	\note �������ܵ��ý����û�в�����
	\since build 1.4
	\sa object_result_t
	*/
	template<typename _fCond, typename _fCallable, typename... _tParams>
	result_of_t<_fCallable && (_tParams&&...)>
		retry_on_cond(_fCond cond, _fCallable&& f, _tParams&&... args)
	{
		using res_t = result_of_t<_fCallable && (_tParams&&...)>;
		using obj_t = object_result_t<res_t>;
		obj_t res;

		do
			res = invoke_nonvoid(lforward(f), lforward(args)...);
		while (expand_proxy<bool(obj_t&)>::call(cond, res));
		return res_t(res);
	}


	/*!
	\brief ������������Ŀɵ��ö���
	\since build 1.4
	\todo ֧�� ref-qualifier ��
	*/
	template<typename _fHandler, typename _fCallable>
	struct expanded_caller
	{
		//! \since build 1.4
		static_assert(is_object<_fCallable>::value, "Callable object type is needed.");

		//! \since build 1.4
		_fCallable caller;

		//! \since build 1.4
		template<typename _fCaller,
			limpl(typename = exclude_self_t<expanded_caller, _fCaller>)>
			expanded_caller(_fCaller&& f)
			: caller(lforward(f))
		{}

		//! \since build 1.4
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
	\brief ���������������Ŀɵ��ö���
	\relates expanded_caller
	\since build 1.4
	*/
	template<typename _fHandler, typename _fCallable>
	lconstfn expanded_caller<_fHandler, decay_t<_fCallable>>
		make_expanded(_fCallable&& f)
	{
		return expanded_caller<_fHandler, decay_t<_fCallable>>(lforward(f));
	}

	/*!
	\brief �ϲ�ֵ���С�
	\note ����ͬ Boost.Signal2 �� \c boost::last_value ���Է� \c void ����ʹ��Ĭ��ֵ��
	\since build 1.4
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

}

#ifdef LB_IMPL_MSCPP
#pragma warning(pop)
#endif
#endif