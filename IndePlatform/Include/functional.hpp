#ifndef IndePlatform_functional_hpp
#define IndePlatform_functional_hpp

#include "type_op.hpp"
#include "tuple.hpp"
#include <functional>
#include <string>

namespace leo
{
	template<typename _tFrom, typename _tTo, typename... _tFromParams,
		typename... _tToParams>
	struct is_covariant<_tFrom(_tFromParams...), _tTo(_tToParams...)>
		: is_covariant<_tFrom, _tTo>
	{};


	template<typename _tResFrom, typename _tResTo, typename... _tFromParams,
		typename... _tToParams>
	struct is_contravariant<_tResFrom(_tFromParams...), _tResTo(_tToParams...)>
		: is_contravariant<std::tuple<_tFromParams...>, std::tuple<_tToParams...>>
	{};

	template<typename... _tParams>
	lconstfn size_t
		//参数个数
		sizeof_params(_tParams&&...)
	{
		return sizeof...(_tParams);
	}

	template<size_t _vN>
	//变长参数操作模板
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
	auto varg(_tParams&&... args) -> decltype(variadic_param<_vN>::get(lforward(args)...))
	{
		static_assert(_vN < sizeof...(args),
			"Out-of-range index of variadic argument found.");

		return variadic_param<_vN>::get(lforward(args)...);
	}

	template<typename _fCallable>
		inline void
		seq_apply(_fCallable&&)
	{}
	template<typename _fCallable, typename _type, typename... _tParams>
	inline void
		//顺序调用
		seq_apply(_fCallable&& f, _type&& arg, _tParams&&... args)
	{
		lforward(f)(yforward(arg));
		leo::seq_apply(lforward(f), lforward(args)...);
	}

	template<typename _fCallable, typename... _tParams>
	inline void
		unseq_apply(_fCallable&& f, _tParams&&... args)
	{
		lunseq((void(lforward(f)(lforward(args))), 0)...);
	}

	template<typename>
	//取参数列表元祖
	struct make_parameter_tuple;

	template<typename _fCallable>
	using make_parameter_tuple_t = typename make_parameter_tuple<_fCallable>::type;

	template<typename _tRet, typename... _tParams>
	struct make_parameter_tuple<_tRet(_tParams...)>
	{
		using type = std::tuple<_tParams...>;
	};

	template<typename _tRet, typename... _tParams>
	struct make_parameter_tuple<_tRet(*)(_tParams...)>
	{
		using type = std::tuple<_tParams...>;
	};

	template<typename _tRet, typename... _tParams>
	struct make_parameter_tuple<_tRet(&)(_tParams...)>
	{
		using type = std::tuple<_tParams...>;
	};

	template<typename _tRet, class _tClass, typename... _tParams>
	//成员函数
	struct make_parameter_tuple<_tRet(_tClass::*)(_tParams...)>
	{
		using type = std::tuple<_tParams...>;
	};

	template<typename _tRet, class _tClass, typename... _tParams>
	struct make_parameter_tuple<_tRet(_tClass::*)(_tParams...) const>
	{
		using type = std::tuple<_tParams...>;
	};

	template<typename _tRet, class _tClass, typename... _tParams>
	struct make_parameter_tuple<_tRet(_tClass::*)(_tParams...) volatile>
	{
		using type = std::tuple<_tParams...>;
	};

	template<typename _tRet, class _tClass, typename... _tParams>
	struct make_parameter_tuple<_tRet(_tClass::*)(_tParams...) const volatile>
	{
		using type = std::tuple<_tParams...>;
	};

	template<typename _tRet, typename... _tParams>
	//函数对象
	struct make_parameter_tuple<std::function<_tRet(_tParams...)>>
	{
		using type = std::tuple<_tParams...>;
	};

	template<typename>
	//取返回类型
	struct return_of;

	template<typename _fCallable>
	using return_of_t = typename return_of<_fCallable>::type;

	template<typename _tRet, typename... _tParams>
	struct return_of<_tRet(_tParams...)>
	{
		using type = _tRet;
	};

	template<typename _tRet, typename... _tParams>
	struct return_of<_tRet(*)(_tParams...)>
	{
		using type = _tRet;
	};

	template<typename _tRet, typename... _tParams>
	struct return_of<_tRet(&)(_tParams...)>
	{
		using type = _tRet;
	};

	template<typename _tRet, class _tClass, typename... _tParams>
	struct return_of<_tRet(_tClass::*)(_tParams...)>
	{
		using type = _tRet;
	};

	template<typename _tRet, class _tClass, typename... _tParams>
	struct return_of<_tRet(_tClass::*)(_tParams...) const>
	{
		using type = _tRet;
	};

	template<typename _tRet, class _tClass, typename... _tParams>
	struct return_of<_tRet(_tClass::*)(_tParams...) volatile>
	{
		using type = _tRet;
	};

	template<typename _tRet, class _tClass, typename... _tParams>
	struct return_of<_tRet(_tClass::*)(_tParams...) const volatile>
	{
		using type = _tRet;
	};

	template<size_t _vIdx, typename _fCallable>
	//取指定索引的参数类型
	struct parameter_of
	{
		using type = tuple_element_t<_vIdx,
			typename make_parameter_tuple<_fCallable>::type>;
	};

	template<size_t _vIdx, typename _fCallable>
	using parameter_of_t = typename parameter_of<_vIdx, _fCallable>::type;

	template<typename _fCallable>
	//取参数列表大小
	struct paramlist_size : integral_constant<size_t, std::tuple_size<typename
		make_parameter_tuple<_fCallable>::type>::value>
	{};

	template<typename, class>
	//调用投影:向原调用传递序列指定的位置的参数.
	struct call_projection;

	template<typename _tRet, typename... _tParams, size_t... _vSeq>
	struct call_projection<_tRet(_tParams...), variadic_sequence<_vSeq...>>
	{
		template<typename _fCallable>
		static _tRet
			call(_fCallable&& f, std::tuple<_tParams...>&& args, remove_reference_t<
			decltype(lforward(f)(lforward(std::get<_vSeq>(std::move(args)))...))>*
			= {})
		{
			lforward(f)(lforward(std::get<_vSeq>(std::move(args)))...);
		}
	};

	template<typename _tRet, typename... _tParams, size_t... _vSeq>
	struct call_projection<std::function<_tRet(_tParams...)>,
		variadic_sequence<_vSeq... >> : private
		call_projection<_tRet(_tParams...), variadic_sequence<_vSeq...>>
	{
		using
		call_projection<_tRet(_tParams...), variadic_sequence<_vSeq...>>::call;
	};

	namespace details
	{

		template<typename _fCallable, size_t _vLen = paramlist_size<_fCallable>::value>
		struct expand_proxy : private call_projection<_fCallable,
			make_natural_sequence_t<_vLen >> , private expand_proxy<_fCallable, _vLen - 1>
		{
			using call_projection<_fCallable, make_natural_sequence_t<_vLen>>::call;
			using expand_proxy<_fCallable, _vLen - 1>::call;
		};

		template<typename _fCallable>
		struct expand_proxy<_fCallable, 0>
			: private call_projection<_fCallable, variadic_sequence<>>
		{
			using call_projection<_fCallable, variadic_sequence<>>::call;
		};

	}//namespace details

	template<typename _fHandler, typename _fCallable>
	//接受冗余参数的可调用对象
	struct expanded_caller
	{
		static_assert(is_object<_fCallable>::value,
			"Callable object type is needed.");

		_fCallable Caller;

		template<typename _fCaller,
			limpl(typename = exclude_self_ctor_t<expanded_caller, _fCaller>)>
			expanded_caller(_fCaller&& f)
			: Caller(lforward(f))
		{}

		template<typename... _tParams>
		auto
			operator()(_tParams&&... args)
			-> decltype(details::expand_proxy<_fHandler>::call(Caller,
			std::forward_as_tuple(lforward(args)...)))
		{
			return details::expand_proxy<_fHandler>::call(Caller,
				std::forward_as_tuple(lforward(args)...));
		}
	};

	//散列扩展接口
	//ref http://www.boost.org/doc/libs/1_54_0/doc/html/hash/reference.html#boost.hash_combine
	//重复计算散列(1UL<<31)/((1+std::sqrt(5)/4) == 0X9E3779B9
	template<typename _type>
	inline void
		hash_combine(size_t& seed, const _type& val)
	{
		seed ^= std::hash<_type>()(val)+0x9E3779B9 + (seed << 6) + (seed >> 2);
	}

	template<typename _type>
	lconstfn size_t
		hash_combine_seq(size_t seed, const _type& val)
	{
		return leo::hash_combine(seed, val), seed;
	}
	template<typename _type, typename... _tParams>
	lconstfn size_t
		hash_combine_seq(size_t seed, const _type& x, const _tParams&... args)
	{
		return leo::hash_combine_seq(leo::hash_combine_seq(seed, x), args...);
	}

	template<typename _tIn>
	inline size_t
		hash_range(_tIn first, _tIn last)
	{
		size_t seed(0);

		for (; first != last; ++first)
			hash_combine(seed, *first);
		return seed;
	}
	template<typename _tIn>
	inline size_t
		hash_range(size_t& seed, _tIn first, _tIn last)
	{
		for (; first != last; ++first)
			hash_combine(seed, *first);
		return seed;
	}

	template<typename...>
	struct combined_hash;

	template<typename _type>
	struct combined_hash<_type> : std::hash<_type>
	{};

	namespace details
	{

		template<bool, class, class>
		struct combined_hash_tuple;

		template<bool _bNoExcept, typename _type, size_t... _vSeq>
		struct combined_hash_tuple<_bNoExcept, _type, variadic_sequence<_vSeq...>>
		{
			static lconstfn size_t
				call(const _type& tp) lnoexcept(_bNoExcept)
			{
				return leo::hash_combine_seq(0, std::get<_vSeq>(tp)...);
			}
		};

	} // namespace details;

	template<typename... _types>
	struct combined_hash<std::tuple<_types...>>
	{
	public:
		using type = std::tuple<_types...>;

#if LB_HAS_NOEXCEPT
	private:
		//! \brief 判断使用 noexcept 并避免 constexpr 失败。
		static lconstexpr bool is_noexcept_v = noexcept(
			leo::hash_combine_seq(0, std::declval<const _types&>()...));
#else
	private:
		static lconstexpr bool is_noexcept_v = false;
#endif

	public:
		lconstfn size_t
			operator()(const type& tp) const lnoexcept(is_noexcept_v)
		{
			return details::combined_hash_tuple<is_noexcept_v, type,
				make_natural_sequence_t<sizeof...(_types) >> ::call(tp);
		}
	};

	template<typename _type1, typename _type2>
	struct combined_hash<std::pair<_type1, _type2>>
		: combined_hash<std::tuple<_type1, _type2>>
	{};

	//仿函数
	struct is_equal
	{
		template<typename _type1, typename _type2>
		lconstfn bool
			operator()(const _type1& x, const _type2& y) const
		{
			return x == y;
		}
	};

	template<typename _type>
	struct ref_eq
	{
		lconstfn bool
			operator()(const _type& _x, const _type& _y) const
		{
			return &_x == &_y;
		}
	};

	struct plus
	{
		template<typename _type>
		lconstfn auto
			operator()(const _type& x, const _type& y) const
			-> decltype(x + y)
		{
			return x + y;
		}
	};

	struct multiply
	{
		template<typename _type>
		lconstfn auto
			operator()(const _type& x, const _type& y) const
			-> decltype(x * y)
		{
			return x * y;
		}
	};

	template<bool, typename _tScalar>
	//自增仿函数
	struct xcrease_t
	{
		inline _tScalar&
			operator()(_tScalar& _x)
		{
			return ++_x;
		}
	};
	template<typename _tScalar>
	//自减仿函数
	struct xcrease_t<false, _tScalar>
	{
		inline _tScalar&
			operator()(_tScalar& _x)
		{
			return --_x;
		}
	};

	template<bool, typename _tScalar1, typename _tScalar2>
	//加法复合赋值运算仿函数
	struct delta_assignment
	{
		lconstfn _tScalar1&
			operator()(_tScalar1& x, _tScalar2 y) const
		{
			return x += y;
		}
	};
	template<typename _tScalar1, typename _tScalar2>
	//减法复合赋值运算仿函数
	struct delta_assignment<false, _tScalar1, _tScalar2>
	{
		lconstfn _tScalar1&
			operator()(_tScalar1& x, _tScalar2 y) const
		{
			return x -= y;
		}
	};

	template<bool _bIsPositive, typename _tScalar>
	lconstfn _tScalar&
		xcrease(_tScalar& _x)
	{
		return xcrease_t<_bIsPositive, _tScalar>()(_x);
	}

	template<bool _bIsPositive, typename _tScalar1, typename _tScalar2>
	lconstfn _tScalar1&
		delta_assign(_tScalar1& _x, _tScalar2& _y)
	{
		return delta_assignment<_bIsPositive, _tScalar1, _tScalar2>()(_x, _y);
	}

	template<typename _type>
	struct deref_op
	{
		lconstfn _type*
			operator()(_type& _x) const
		{
			return &_x;
		}
	};

	template<typename _type>
	struct const_deref_op
	{
		inline const _type*
			operator()(const _type& _x) const
		{
			return &_x;
		}
	};

	template<typename _type, typename _tPointer = _type*,
	class _fCompare = std::less<_type >>
	//比较指针指向对象
	//若参数有空指针 => false
	struct deref_comp
	{
		bool
			operator()(const _tPointer& _x, const _tPointer& _y) const
		{
			return bool(_x) && bool(_y) && _fCompare()(*_x, *_y);
		}
	};

	template<typename _tChar, class _fCompare = std::less<_tChar>>
	//是否满足字典严格偏序关系
	//若参数有空指针 => false
	struct deref_str_comp
	{
		bool
			operator()(const _tChar* x, const _tChar* y) const
		{
			using traits_type = std::char_traits<_tChar>;

			return x && y && std::lexicographical_compare(x, x + traits_type
				::length(x), y, y + traits_type::length(y), _fCompare());
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
}
#endif