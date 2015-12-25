#ifndef IndePlatform_type_op_hpp
#define IndePlatform_type_op_hpp 1

#include "ldef.h" // type_traits.std::delval


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
#	if !YB_IMPL_GNUC || YB_IMPL_GNUCPP >= 50000
	using std::aligned_union_t;
#	endif
	using std::decay_t;
	using std::enable_if_t;
	using std::conditional_t;
	using std::common_type_t;
	using std::underlying_type_t;
	using std::result_of_t;
#else
	//@{
	template<typename _type>
	using remove_const_t = typename remove_const<_type>::type;

	template<typename _type>
	using remove_volatile_t = typename remove_volatile<_type>::type;

	template<typename _type>
	using remove_cv_t = typename remove_cv<_type>::type;

	template<typename _type>
	using add_const_t = typename add_const<_type>::type;

	template<typename _type>
	using add_volatile_t = typename add_volatile<_type>::type;

	template<typename _type>
	using add_cv_t = typename add_cv<_type>::type;


	template<typename _type>
	using remove_reference_t = typename remove_reference<_type>::type;

	template<typename _type>
	using add_lvalue_reference_t = typename add_lvalue_reference<_type>::type;

	template<typename _type>
	using add_rvalue_reference_t = typename add_rvalue_reference<_type>::type;


	template<typename _type>
	using make_signed_t = typename make_signed<_type>::type;

	template<typename _type>
	using make_unsigned_t = typename make_unsigned<_type>::type;


	template<typename _type>
	using remove_extent_t = typename remove_extent<_type>::type;

	template<typename _type>
	using remove_all_extents_t = typename remove_all_extents<_type>::type;


	template<typename _type>
	using remove_pointer_t = typename remove_pointer<_type>::type;

	template<typename _type>
	using add_pointer_t = typename add_pointer<_type>::type;


	template<size_t _vLen,
		size_t _vAlign = lalignof(typename aligned_storage<_vLen>::type)>
		using aligned_storage_t = typename aligned_storage<_vLen, _vAlign>::type;
	//@}

	//@{
#	if !LB_IMPL_GNUC || LB_IMPL_GNUCPP >= 50000
	template<size_t _vLen, typename... _types>
	using aligned_union_t = typename aligned_union<_vLen, _types...>::type;
#	endif

	template<typename _type>
	using decay_t = typename decay<_type>::type;

	template<bool _bCond, typename _type = void>
	using enable_if_t = typename enable_if<_bCond, _type>::type;

	template<bool _bCond, typename _type, typename _type2>
	using conditional_t = typename conditional<_bCond, _type, _type2>::type;

	template<typename... _types>
	using common_type_t = typename common_type<_types...>::type;

	template<typename _type>
	using underlying_type_t = typename underlying_type<_type>::type;

	template<typename _type>
	using result_of_t = typename result_of<_type>::type;
	//@}
#endif
	//@}

} // inline namespace cpp2014;

//兼容性保留
//todo remove it
namespace leo {

	using stdex::nullptr_t;

	template<typename _type>
	struct is_returnable : integral_constant < bool, !is_array<_type>::value
		&& !is_abstract<_type>::value && !is_function<_type>::value >
	{};

	template<typename type>
	struct id_decayable
		:integral_constant < bool, !is_same<decay_t<type>, type>::value >
	{};

	template<typename _type>
	struct is_class_pointer : integral_constant < bool, is_pointer<_type>::value
		&& is_class<remove_pointer_t<_type>>::value >
	{};

	template<typename _type>
	struct is_lvalue_class_reference : integral_constant < bool, !is_lvalue_reference<
		_type>::value && is_class<remove_reference_t<_type>>::value >
	{};

	template<typename _type>
	struct is_rvalue_class_reference : integral_constant < bool, !is_lvalue_reference<
		_type>::value && is_class<typename remove_reference<_type>::type>::value >
	{};

	template<typename _type>
	struct is_pod_struct : integral_constant < bool,
		is_pod<_type>::value && is_class<_type>::value >
	{};

	template<typename _type>
	struct is_pod_union : integral_constant < bool,
		is_pod<_type>::value && is_union<_type>::value >
	{};

	template<typename _tFrom, typename _tTo>
	struct is_covariant : is_convertible < _tFrom, _tTo >
	{};

	template<typename _tFrom, typename _tTo>
	struct is_contravariant : is_convertible < _tTo, _tFrom >
	{};

	namespace details
	{
#define LB_HAS_MEMBER(_n) \
	template<class _type> \
	struct has_mem_##_n \
				{ \
	private: \
		template<typename _type2> \
		static std::true_type \
		test(stdex::empty_base<typename _type2::_n>*); \
		template<typename _type2> \
		static std::false_type \
		test(...); \
	\
	public: \
		static lconstexpr bool value = decltype(test<_type>(nullptr))::value; \
				};
		LB_HAS_MEMBER(value)

#define LB_TYPE_OP_TEST_2(_n, _expr) \
	template<typename _type1, typename _type2> \
	struct _n \
							{ \
	private: \
		template<typename _type> \
		static std::true_type \
		test(enable_if_t<(_expr), int>); \
		template<typename> \
		static std::false_type \
		test(...); \
	\
	public: \
		static lconstexpr bool value = decltype(test<_type1>(0))::value; \
							};


			LB_TYPE_OP_TEST_2(have_equality_operator, (is_convertible<decltype(std::declval<
				_type>() == std::declval<_type2>()), bool>::value))


			LB_TYPE_OP_TEST_2(has_subscription, !is_void < decltype(std::declval<_type>()[
				std::declval<_type2>()]) > ::value)

			template<class _type>
				struct have_nonempty_virtual_base
				{
					static_assert(std::is_class<_type>::value,
						"Non-class type found @ leo::has_nonempty_virtual_base;");

				private:
					struct A : _type
					{
						~A()
						{}
					};
					struct B : _type
					{
						~B()
						{}
					};
					struct C : A, B
					{
						//! \since build 461
						~C()
						{}
					};

				public:
					static lconstexpr bool value = sizeof(C) < sizeof(A) + sizeof(B);
				};

				template<class _type1, class _type2>
				struct have_common_nonempty_virtual_base
				{
					static_assert(std::is_class<_type1>::value
						&& std::is_class<_type2>::value,
						"Non-class type found @ leo::has_common_nonempty_virtual_base;");

				private:
					struct A : virtual _type1
					{
						//! \since build 461
						~A()
						{}
					};

					struct B : virtual _type2
					{
						~B()
						{}
					};
					struct C : A, B
					{
						~C()
						{}
					};

				public:
					static lconstexpr bool value = sizeof(C) < sizeof(A) + sizeof(B);
				};

	}// namespace details



	template<class _type>
	struct has_mem_value : std::integral_constant<bool,
		details::has_mem_value<remove_cv_t<_type>>::value>
	{};


	template<typename _type1, typename _type2>
	struct has_subscription : details::has_subscription < _type1, _type2 >
	{};

	template<typename _type1, typename _type2>
	struct has_equality_operator : integral_constant < bool,
		details::have_equality_operator<_type1, _type2>::value >
	{};

	template<class _type>
	struct has_nonempty_virtual_base : integral_constant < bool,
		details::have_nonempty_virtual_base<_type>::value >
	{};

	template<class _type1, class _type2>
	struct has_common_nonempty_virtual_base : integral_constant<bool,
		details::have_common_nonempty_virtual_base<_type1, _type2>::value>
	{};

	/*!
	\ingroup metafunctions
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
	struct remove_rcv
	{
		using type = remove_cv_t<remove_reference_t<_type>>;
	};

	template<typename _type>
	using remove_rcv_t = typename remove_rcv<_type>::type;

	template<typename _type>
	struct remove_rp
	{
		using type = remove_pointer_t<remove_reference_t<_type>>;
	};

	template<typename _type>
	struct remove_rpcv
	{
		using type = remove_cv_t<typename remove_rp<_type>::type>;
	};

	template<typename _type>
	struct array_decay
	{
		using type = conditional_t<is_array<_type>::value, decay_t<_type>, _type>;
	};

	template<typename _type>
	struct qualified_decay
	{
	private:
		using value_type = remove_reference_t<_type>;

	public:
		using type = conditional_t<is_function<value_type>::value
			|| is_array<value_type>::value, decay_t<_type>, _type>;
	};

	template<typename _type>
	struct array_ref_decay
	{
		using type = typename array_decay<_type>::type;
	};

	template<typename _type>
	struct array_ref_decay<_type&>
	{
		using type = typename array_decay<_type>::type;
		using reference = type&;
	};

	template<typename _type>
	struct array_ref_decay<_type&&>
	{
		using type = typename array_decay<_type>::type;
		using reference = type&&;
	};

	template<class _tClass, typename _tParam, typename _type = int>
	using exclude_self_ctor_t
		= enable_if_t<!is_same<_tClass&, remove_rcv_t<_tParam>&>::value, _type>;

	template<size_t _vN>
	struct n_tag
	{
		using type = n_tag<_vN - 1>;
	};

	template<>
	struct n_tag<0>
	{
		using type = void;
	};

	using first_tag = n_tag<0>;

	using second_tag = n_tag<1>;
}


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
{};

template<typename _b1>
struct or_<_b1> : _b1
{};

template<typename _b1, typename _b2>
struct or_<_b1, _b2> : conditional_t<_b1::value, _b1, _b2>
{};

template<typename _b1, typename _b2, typename _b3, typename... _bn>
struct or_<_b1, _b2, _b3, _bn...>
	: conditional_t<_b1::value, _b1, or_<_b2, _b3, _bn...>>
{};


template<typename _b>
struct not_ : integral_constant<bool, !_b::value>
{};
#endif