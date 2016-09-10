/*!	\file type_traits.hpp
\ingroup LBase
\brief ISO C++ 类型特征扩展。
*/

#ifndef Lbase_type_traits_hpp
#define Lbase_type_traits_hpp 1

#include "LBase/lcompiler.h" // type_traits.std::delval

namespace leo {
	/*!
	\brief 兼容性修真
	\todo 移除
	*/
	using stdex::empty_base;
}

namespace leo {
	/*!	\defgroup template_meta_programing Template Meta Programing
	\brief 模板元编程。
	\note 以下类别中的接口包括类模板和对应的别名模板。
	\since build 1.4
	*/

	/*!	\defgroup meta_types Meta Types
	\ingroup template_meta_programing
	\brief 元类型。
	\since build 1.4
	*/

	/*!	\defgroup meta_operations Meta Operations
	\ingroup template_meta_programing
	\brief 元操作。
	\since build 1.4
	*/

	/*!	\defgroup metafunctions Metafunctions
	\ingroup meta_operations
	\brief 元函数。
	\see http://www.boost.org/doc/libs/1_50_0/libs/mpl/doc/refmanual/metafunction.html 。
	\since build 1.4
	*/

	/*!	\defgroup type_traits_operations Type Traits Operations
	\ingroup metafunctions
	\brief 类型特征操作。
	\since build 1.4
	*/

	/*!	\defgroup unary_type_traits Unary Type Traits
	\ingroup type_traits_operations
	\brief 一元类型特征。
	\see ISO C++11 20.9.1 [meta.rqmts] 。
	\since build 1.4
	*/

	/*!	\defgroup binary_type_traits Binary Type Traits
	\ingroup type_traits_operations
	\brief 二元类型特征。
	\see ISO C++11 20.9.1 [meta.rqmts] 。
	\since build 1.4
	*/

	/*!	\defgroup transformation_traits Transformation Traits
	\ingroup type_traits_operations
	\brief 变换类型特征。
	\see ISO C++11 20.9.1 [meta.rqmts] 。
	\since build 1.4
	*/

	/*!
	\ingroup metafunctions
	\brief 嵌套类型别名。
	\since build 1.4
	*/
	template<class _type>
	using _t = typename _type::type;

#define type_t(type) \
	template<typename _type> \
	using type##_t = _t<type<_type>>

	inline namespace cpp2011
	{
		using std::integral_constant;
		using std::true_type;
		using std::false_type;

		using std::is_void;
		using std::is_integral;
		using std::is_floating_point;
		using std::is_array;
		using std::is_pointer;
		using std::is_lvalue_reference;
		using std::is_rvalue_reference;
		using std::is_member_object_pointer;
		using std::is_member_function_pointer;
		using std::is_enum;
		using std::is_class;
		using std::is_union;
		using std::is_function;

		using std::is_reference;
		using std::is_arithmetic;
		using std::is_fundamental;
		using std::is_object;
		using std::is_scalar;
		using std::is_compound;
		using std::is_member_pointer;

		using std::is_const;
		using std::is_volatile;
		using std::is_trivial;
		//	using std::is_trivially_copyable;
		using std::is_standard_layout;
		using std::is_pod;
		using std::is_literal_type;
		using std::is_empty;
		using std::is_polymorphic;
		using std::is_abstract;

		using std::is_signed;
		using std::is_unsigned;

		using std::is_constructible;
		//@{
		using std::is_default_constructible;
		using std::is_copy_constructible;
		using std::is_move_constructible;

		using std::is_assignable;
		using std::is_copy_assignable;
		using std::is_move_assignable;

		using std::is_destructible;
		//@}

#	if !LB_IMPL_GNUC || LB_IMPL_GNUCPP >= 50000
	//@{
		using std::is_trivially_constructible;
		using std::is_trivially_default_constructible;
		using std::is_trivially_copy_constructible;
		using std::is_trivially_move_constructible;

		using std::is_trivially_assignable;
		using std::is_trivially_copy_assignable;
		using std::is_trivially_move_assignable;
		//@}
#	endif
		using std::is_trivially_destructible;

		//@{
		using std::is_nothrow_constructible;
		using std::is_nothrow_default_constructible;
		using std::is_nothrow_copy_constructible;
		using std::is_nothrow_move_constructible;

		using std::is_nothrow_assignable;
		using std::is_nothrow_copy_assignable;
		using std::is_nothrow_move_assignable;

		using std::is_nothrow_destructible;
		//@}

		using std::has_virtual_destructor;

		using std::alignment_of;
		using std::rank;
		using std::extent;

		using std::is_same;
		using std::is_base_of;
		using std::is_convertible;

		using std::remove_const;
		using std::remove_volatile;
		using std::remove_cv;
		using std::add_const;
		using std::add_volatile;
		using std::add_cv;

		using std::remove_reference;
		using std::add_lvalue_reference;
		using std::add_rvalue_reference;

		using std::make_signed;
		using std::make_unsigned;

		using std::remove_extent;
		using std::remove_all_extents;

		using std::remove_pointer;
		using std::add_pointer;

		using std::aligned_storage;
#	if !LB_IMPL_GNUC || LB_IMPL_GNUCPP >= 50000
		using std::aligned_union;
#	endif
		using std::decay;
		using std::enable_if;
		using std::conditional;
		using std::common_type;
		using std::underlying_type;
		using std::result_of;
		//@}
	}

	/*!
	\brief 包含 ISO C++ 2014 引入的名称的命名空间。
	*/
	inline namespace cpp2014
	{
		/*!
		\ingroup transformation_traits
		\brief ISO C++ 14 兼容类型操作别名。
		*/
		//@{
#if __cpp_lib_transformation_trait_aliases >= 201304 || __cplusplus > 201103L
		using std::remove_const_t;
		using std::remove_volatile_t;
		using std::remove_cv_t;
		using std::add_const_t;
		using std::add_volatile_t;
		using std::add_cv_t;

		using std::remove_reference_t;
		using std::add_lvalue_reference_t;
		using std::add_rvalue_reference_t;

		using std::make_signed_t;
		using std::make_unsigned_t;

		using std::remove_extent_t;
		using std::remove_all_extents_t;

		using std::remove_pointer_t;
		using std::add_pointer_t;

		using std::aligned_storage_t;
		using std::aligned_union_t;
		using std::decay_t;
		using std::enable_if_t;
		using std::conditional_t;
		using std::common_type_t;
		using std::underlying_type_t;
		using std::result_of_t;

#endif
		//@}

	} // inline namespace cpp2014;

#if __cpp_lib_bool_constant >= 201505
	using std::bool_constant;
#else
	template<bool _b>
	using bool_constant = integral_constant<bool, _b>;
#endif

	/*!
	\ingroup metafunctions
	\brief 逻辑操作元函数。
	\note 和 libstdc++ 实现以及 Boost.MPL 兼容。
	*/
	//@{
	template<typename...>
	struct and_;

	template<>
	struct and_<> : true_type
	{};

	template<typename _b1>
	struct and_<_b1> : _b1
	{};

	template<typename _b1, typename _b2>
	struct and_<_b1, _b2> : conditional_t<_b1::value, _b2, _b1>
	{};

	template<typename _b1, typename _b2, typename _b3, typename... _bn>
	struct and_<_b1, _b2, _b3, _bn...>
		: conditional_t<_b1::value, and_<_b2, _b3, _bn...>, _b1>
	{};


	template<typename...>
	struct or_;

	template<>
	struct or_<> : false_type
	{

	};

	template<typename _b1>
	struct or_<_b1> : _b1
	{
	};

	template<typename _b1, typename _b2>
	struct or_<_b1, _b2> : conditional_t<_b1::value, _b1, _b2>
	{
	};

	template<typename _b1, typename _b2, typename _b3, typename... _bn>
	struct or_<_b1, _b2, _b3, _bn...>
		: conditional_t<_b1::value, _b1, or_<_b2, _b3, _bn...>>
	{
	};


	template<typename _b>
	struct not_ : bool_constant<!_b::value>
	{};
	//@}

	namespace details
	{

		template<typename _type>
		struct is_referenceable_function : false_type
		{};

		template<typename _tRes, typename... _tParams>
		struct is_referenceable_function<_tRes(_tParams...)> : true_type
		{};

		template<typename _tRes, typename... _tParams>
		struct is_referenceable_function<_tRes(_tParams..., ...)> : true_type
		{};

	} // namespace details;

	  /*!
	  \ingroup unary_type_traits
	  \brief 判断指定类型是否为可引用类型。
	  \see ISO C++11 17.3.20 [defns.referenceable] 。
	  */
	template<typename _type>
	struct is_referenceable : or_<is_object<_type>, is_reference<_type>,
		details::is_referenceable_function<_type >>
	{};

	struct any_constructible;
	struct nonesuch;

	/*!
	\brief 引入 std::swap 实现为 ADL 提供重载的命名空间。
	\since build 1.4
	*/
	namespace dep_swap
	{

		using std::swap;

		//! \since build 1.4
		nonesuch
			swap(any_constructible, any_constructible);

		template<typename _type, typename _type2>
		struct limpl(helper)
		{
			static lconstexpr const bool value = !is_same<decltype(swap(std::declval<
				_type>(), std::declval<_type2>())), nonesuch>::value;

			//! \since build 1.4
			helper()
				lnoexcept_spec(swap(std::declval<_type>(), std::declval<_type2>()))
			{}
		};

	} // namespace dep_swap;

	  /*!
	  \ingroup type_traits_operations
	  \see ISO C++11 [swappable.requirements] 。
	  \since build 1.4
	  */
	  //@{
	  //! \brief 判断是否可以调用 \c swap 。
	  //@{
	  //! \ingroup binary_type_traits
	template<typename _type, typename _type2>
	struct is_swappable_with
		: bool_constant<limpl(dep_swap::helper<_type, _type2>::value)>
	{};


	//! \ingroup unary_type_traits
	template<typename _type>
	struct is_swappable
		: and_<is_referenceable<_type>, is_swappable_with<_type&, _type&>>
	{};
	//@}


	//! \brief 判断是否可以无抛出地调用 \c swap 。
	//@{
	template<typename _type, typename _type2>
	struct is_nothrow_swappable_with
		: is_nothrow_default_constructible<limpl(dep_swap::helper<_type, _type2>)>
	{};


	template<typename _type>
	struct is_nothrow_swappable
		: and_<is_referenceable<_type>, is_nothrow_swappable_with<_type&, _type&>>
	{};
	//@}
	//@}

	//! \ingroup metafunctions
	//@{
	//! \brief 表达式 SFINAE 别名模板。
	//@{
	//! \since build 1.4
	template<typename _type>
	struct always
	{
	private:
		template<typename...>
		struct impl
		{
			using type = _type;
		};

	public:
		template<typename... _types>
		using apply = impl<_types...>;
	};

	/*!
	\brief void_t 的一般化，允许指定别名类型。
	\see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59204 。
	\see http://wg21.cmeerw.net/cwg/issue1558 。
	\since build 1.4
	*/
#if LB_IMPL_GNUCPP >= 50000
	template<typename _type, typename...>
	using well_formed_t = _type;
#else
	template<typename _type, typename... _types>
	using well_formed_t = typename always<_type>::template apply<_types...>::type;
#endif

	/*!
	\see WG21 N3911 。
	\see WG21 N4296 20.10.2[meta.type.synop] 。
	\since build 1.4
	*/
	// TODO: Blocked. Wait for upcoming ISO C++17 for %__cplusplus.
#if __cpp_lib_void_t >= 201411
	using std::void_t;
#else
	template<typename... _types>
	using void_t = well_formed_t<void, _types...>;
#endif

	/*!
	\brief 类似 void_t 的元函数，具有较低匹配优先级，避免偏特化歧义；但使用条件表达式。
	\sa void_t
	\note 无定义。
	\since build 1.4
	*/
	template<bool _bCond>
	struct when;

	/*!
	\brief 类似 void_t 的元函数，但具有较低匹配优先级，避免偏特化歧义。
	\sa void_t
	\since build 1.4
	*/
	template<typename... _types>
	using when_valid = well_formed_t<when<true>, _types...>;

	/*!
	\brief 类似 enable_if_t 的元函数，但具有较低匹配优先级，避免偏特化歧义。
	\sa enable_if_t
	\sa when
	\since build 1.4
	*/
	template<bool _bCond>
	using enable_when = enable_if_t<_bCond, when<true>>;
	//@}
	//@}

	/*!
	\brief 接受任意参数类型构造的类型。
	\since build 1.4
	*/
	struct any_constructible
	{
		any_constructible(...);
	};


	/*!
	\brief 不接受任意参数类型构造的类型。
	\see WG21 N4502 6.6 。
	\since build 1.4
	*/
	struct nonesuch
	{
		nonesuch() = delete;
		nonesuch(const nonesuch&) = delete;
		~nonesuch() = delete;

		void
			operator=(const nonesuch&) = delete;
	};

	/*!
	\ingroup metafunctions
	\since build 1.4
	\see WG21 N4502 。
	*/
	//@{
	namespace details
	{

		template<typename _tDefault, typename, template<typename...> class, typename...>
		struct detector
		{
			using value_t = false_type;
			using type = _tDefault;
		};

		template<typename _tDefault, template<typename...> class _gOp,
			typename... _tParams>
		struct detector<_tDefault, void_t<_gOp<_tParams...>>, _gOp, _tParams...>
		{
			using value_t = true_type;
			using type = _gOp<_tParams...>;
		};

	} // namespace details;

	template<template<typename...> class _gOp, typename... _tParams>
	using is_detected
		= typename details::detector<nonesuch, void, _gOp, _tParams...>::value_t;

	template<template<typename...> class _gOp, typename... _tParams>
	using detected_t = _t<details::detector<nonesuch, void, _gOp, _tParams...>>;

	template<typename _tDefault, template<typename...> class _gOp,
		typename... _tParams>
		using detected_or = details::detector<_tDefault, void, _gOp, _tParams...>;

	template<typename _tDefault, template<typename...> class _gOp,
		typename... _tParams>
		using detected_or_t = _t<detected_or<_tDefault, _gOp, _tParams...>>;

	template<typename _tExpected, template<typename...> class _gOp,
		typename... _tParams>
		using is_detected_exact = is_same<_tExpected, detected_t<_gOp, _tParams...>>;

	template<typename _tTo, template<typename...> class _gOp, typename... _tParams>
	using is_detected_convertible
		= is_convertible<detected_t<_gOp, _tParams...>, _tTo>;
	//@}

	//! \ingroup metafunctions
	//@{
	/*!
	\brief 条件判断。
	\sa conditional_t
	\since build 1.4
	*/
	template<typename _tCond, typename _tThen = true_type,
		typename _tElse = false_type>
		using cond_t = conditional_t<_tCond::value, _tThen, _tElse>;

	/*!
	\brief 移除选择类类型的特定重载避免构造模板和复制/转移特殊成员函数冲突。
	\pre 第一参数为类类型。
	\sa enable_if_t
	\since build 1.4
	*/
	template<class _tClass, typename _tParam, typename _type = void>
	using exclude_self_t
		= enable_if_t<!is_same<_tClass&, decay_t<_tParam>&>::value, _type>;
	//@}

	/*!
	\ingroup unary_type_traits
	\tparam _type 需要判断特征的类型参数。
	*/
	//@{
	//! \note 以下参数可能是 cv 修饰的类型，结果和去除 cv 修饰符的类型一致。
	//@{
	/*!
	\brief 判断是否为未知大小的数组类型。
	\since build 1.4
	*/
	template<typename _type>
	struct is_array_of_unknown_bound
		: bool_constant<is_array<_type>::value && extent<_type>::value == 0>
	{};


	/*!
	\brief 判断指定类型是否为对象或 void 类型。
	\since build 1/4
	*/
	template<typename _type>
	struct is_object_or_void : or_<is_object<_type>, is_void<_type>>
	{};


	/*!
	\brief 判断指定类型是否可作为返回类型。
	\note 即排除数组类型、抽象类类型和函数类型的所有类型。
	\see ISO C++11 8.3.5/8 和 ISO C++11 10.4/3 。
	\since build 1.4
	*/
	template<typename _type>
	struct is_returnable
		: not_<or_<is_array<_type>, is_abstract<_type>, is_function<_type>>>
	{};


	/*!
	\brief 判断指定类型是否是类类型。
	\since build 1..4
	*/
	template<typename _type>
	struct is_class_type
		: or_<is_class<_type>, is_union<_type>>
	{};


	/*!
	\brief 判断类型是否为非 const 对象类型。
	\since build 1.3
	*/
	template<typename _type>
	struct is_nonconst_object
		: and_<is_object<_type>, is_same<add_const_t<_type>, _type>>
	{};


	//! \since build 1.3
	//@{
	//! \brief 判断指定类型是否是指向对象类型的指针。
	template<typename _type>
	struct is_pointer_to_object
		: and_<is_pointer<_type>, is_object<remove_pointer_t<_type>>>
	{};


	//! \brief 判断指定类型是否是指向对象类型的指针。
	template<typename _type>
	struct is_object_pointer
		: and_<is_pointer<_type>, is_object_or_void<remove_pointer_t<_type>>>
	{};


	//! \brief 判断指定类型是否是指向函数类型的指针。
	template<typename _type>
	struct is_function_pointer
		: and_<is_pointer<_type>, is_function<remove_pointer_t<_type>>>
	{};
	//@}


	//! \since build 1.3
	//@{
	//! \brief 判断指定类型是否是指向类类型的指针。
	template<typename _type>
	struct is_class_pointer
		: and_<is_pointer<_type>, is_class<remove_pointer_t<_type>>>
	{};


	//! \brief 判断指定类型是否是类类型左值引用。
	template<typename _type>
	struct is_lvalue_class_reference
		: and_<is_lvalue_reference<_type>, is_class<remove_reference_t<_type>>>
	{};


	//! \brief 判断指定类型是否是类类型右值引用。
	template<typename _type>
	struct is_rvalue_class_reference
		: and_<is_rvalue_reference<_type>, is_class<remove_reference_t<_type>>>
	{};


	/*!
	\pre remove_all_extents<_type> 是完整类型或（可能 cv 修饰的） \c void 。
	\see ISO C++11 9/10 。
	*/
	//@{
	//! \brief 判断指定类型是否是 POD struct 。
	template<typename _type>
	struct is_pod_struct : and_<is_pod<_type>, is_class<_type>>
	{};


	//! \brief 判断指定类型是否是 POD union 。
	template<typename _type>
	struct is_pod_union : and_<is_pod<_type>, is_union<_type>>
	{};
	//@}
	//@}
	//@}

	/*!
	\brief 判断指定类型是否为 const 或 volatile 类型。
	\since build 1.4
	*/
	template<typename _type>
	struct is_cv : or_<is_const<_type>, is_volatile<_type>>
	{
#ifdef LB_IMPL_MSCPP
		lconstfn is_cv() lnoexcept = default;
#endif
	};


	/*!
	\brief 判断指定类型是否可分解为一个参数为类型的模板和类型参数。
	\since build 1.4
	*/
	//@{
	template<typename>
	struct is_decomposable : false_type
	{};

	template<template<typename...> class _gOp, typename... _types>
	struct is_decomposable<_gOp<_types...>> : true_type
	{};
	//@}


	/*!
	\brief 判断指定类型是否已退化。
	\since build 1.4
	*/
	template<typename _type>
	struct is_decayed : or_<is_same<decay_t<_type>, _type>>
	{};


	/*!
	\pre remove_all_extents<_type> 是完整类型、（可能 cv 修饰的） \c void ，
	或未知大小的数组。
	\since build 1.4
	*/
	//@{
	template<typename _type>
	struct is_copyable
		: and_<is_copy_constructible<_type>, is_copy_assignable<_type>>
	{};


	template<typename _type>
	struct is_moveable
		: and_<is_move_constructible<_type>, is_move_assignable<_type>>
	{};


	template<typename _type>
	struct is_nothrow_copyable : and_<is_nothrow_copy_constructible<_type>,
		is_nothrow_copy_assignable<_type >>
	{};


	template<typename _type>
	struct is_nothrow_moveable : and_<is_nothrow_move_constructible<_type>,
		is_nothrow_move_assignable<_type >>
	{};


#	if !LB_IMPL_GNUC || LB_IMPL_GNUCPP >= 50000
	template<typename _type>
	struct is_trivially_copyable : and_<is_trivially_copy_constructible<_type>,
		is_trivially_copy_assignable<_type >>
	{};


	template<typename _type>
	struct is_trivially_moveable : and_<is_trivially_move_constructible<_type>,
		is_trivially_move_assignable<_type >>
	{};
#	endif
	//@}
	//@}

	/*!
	\ingroup binary_type_traits
	\brief 判断指定类型之间是否可转换。
	\since build 1.4
	*/
	template<typename _type1, typename _type2>
	struct is_interoperable
		: or_<is_convertible<_type1, _type2>, is_convertible<_type2, _type1>>
	{};


	/*!
	\ingroup metafunctions
	\since build 1.4
	*/
	//@{
	//! \brief 判断第一个参数和之后的参数指定的类型相同。
	template<typename _type, typename... _types>
	struct are_same : and_<is_same<_type, _types>...>
	{};

	//! \brief 判断第一个参数在之后参数指定的类型中出现。
	template<typename _type, typename... _types>
	struct is_in_types : or_<is_same<_type, _types...>>
	{};
	//@}

	//!ingroup transformation_traits
	//@{
	/*!
	\brief 恒等元函数。
	\note 功能可以使用 ISO C++ 11 的 std::common_type 的单一参数实例替代。
	\note ISO C++ LWG 2141 建议更改 std::common 的实现，无法替代。
	\note 这里的实现不依赖 std::common_type 。
	\note 同 boost::mpl::identity 。
	\note Microsoft Visual C++ 2013 使用 LWG 2141 建议的实现。
	\see http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2141
	\see http://www.boost.org/doc/libs/1_55_0/libs/mpl/doc/refmanual/identity.html
	\see http://msdn.microsoft.com/en-us/library/vstudio/bb531344%28v=vs.120%29.aspx
	\see http://lists.cs.uiuc.edu/pipermail/cfe-commits/Week-of-Mon-20131007/090403.html
	*/

	template<typename _type>
	struct identity
	{
		using type = _type;
	};

	template<typename _type>
	using identity_t = _t<identity<_type>>;
	//@}

	//! \since build 1.4
	//@{
	template<typename _type>
	using addrof_t = decltype(&std::declval<_type>());

	template<typename _type>
	using indirect_t = decltype(*std::declval<_type>());
	//@}

	//! \since build 1.4
	template<typename _type>
	using indirect_element_t = remove_reference_t<indirect_t<_type>>;

	/*!
	\sa enable_if_t
	*/
	//@{
	template<typename _tFrom, typename _tTo, typename _type = void>
	using enable_if_convertible_t
		= enable_if_t<is_convertible<_tFrom, _tTo>::value, _type>;

	template<typename _tFrom,typename _tTo,typename _type = void>
	using enable_if_incovertible_t
		= enable_if_t<!is_convertible<_tFrom, _tTo>::value, _type>;

	template<typename _type1, typename _type2, typename _type = void>
	using enable_if_interoperable_t
		= enable_if_t<is_interoperable<_type1, _type2>::value, _type>;

	template<typename _type1, typename _type2, typename _type = void>
	using enable_if_same_t
		= enable_if_t<is_same<_type1, _type2>::value, _type>;
	//@}
	//@}
}

#endif