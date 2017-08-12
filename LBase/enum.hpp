/*! \file enum.hpp
\ingroup LBase
\brief 枚举相关操作。
*/

#ifndef LBase_enum_hpp
#define LBase_enum_hpp 1

#include "tuple.hpp" // for false_type, underlying_type_t, _t, and_, or_,
//	is_enum, common_type_t, vseq::find, std::tuple;

namespace leo {
	//@{
	//! \ingroup unary_type_traits
	template<typename>
	struct is_enum_union :false_type
	{};

	//! \ingroup transformation_traits
	//@{
	template<typename _type>
	struct wrapped_enum_traits {
		using type = underlying_type_t<_type>;
	};

	type_t(wrapped_enum_traits);
	//@}

	template<typename... _types>
	class enum_union
	{
		static_assert(and_<or_<is_enum<_types>, is_enum_union<_types>>...>::value,
			"Invalid types found.");

	public:
		using underlying_type = common_type_t<wrapped_enum_traits_t<_types>...>;

	private:
		underlying_type value;

	public:
		lconstfn
			enum_union() = default;
		explicit lconstfn
			enum_union(underlying_type i) lnothrow
			: value(i)
		{}
		template<typename _type,
			limpl(typename = vseq::find<std::tuple<_types...>, _type>())>
			lconstfn
			enum_union(_type e) lnothrow
			: value(static_cast<underlying_type>(e))
		{}
		lconstfn
			enum_union(const enum_union&) = default;

		enum_union&
			operator=(const enum_union&) = default;

		explicit lconstfn
			operator underlying_type() const lnothrow
		{
			return value;
		}
	};

	//! \relates enum_union
	//@{
	template<typename... _types>
	struct is_enum_union<enum_union<_types...>> : true_type
	{};

	template<typename... _types>
	struct wrapped_enum_traits<enum_union<_types...>>
	{
		using type = typename enum_union<_types...>::underlying_type;
	};

	template<typename _type>
	lconstfn limpl(enable_if_t)<is_enum_union<_type>::value,
		wrapped_enum_traits_t<_type >>
		underlying(_type val) lnothrow
	{
		return wrapped_enum_traits_t<_type>(val);
	}
	//@}
	//@}
	
}
#endif