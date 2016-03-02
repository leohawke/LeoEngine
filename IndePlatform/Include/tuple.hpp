#ifndef IndePlatform_tuple_hpp
#define IndePlatform_tuple_hpp

#include "ldef.h"
#include "type_op.hpp"
#include "variadic.hpp"
#include <tuple>

namespace leo
{
	template<size_t _vIdx, typename _type>
	using tuple_element_t = typename std::tuple_element<_vIdx, _type>::type;

	namespace details
	{

		template<class, class, class>
		struct tuple_element_convertible;

		template<class _type1, class _type2>
		struct tuple_element_convertible<_type1, _type2, variadic_sequence<>>
			: true_type
		{};

		template<typename... _types1, typename... _types2, size_t... _vSeq,
			size_t _vHead>
		struct tuple_element_convertible<std::tuple<_types1...>, std::tuple<_types2...>,
			variadic_sequence<_vHead, _vSeq... >>
		{
			static_assert(sizeof...(_types1) == sizeof...(_types2),
				"Mismatched sizes of tuple found.");
		private:
			using t1 = std::tuple<_types1...>;
			using t2 = std::tuple<_types2...>;

		public:
			static lconstexpr bool value = std::is_convertible<
				sequence_element_t<_vHead, t1>, sequence_element_t<_vHead, t2 >> ::value
				&& tuple_element_convertible<t1, t2, variadic_sequence<_vSeq...>>
				::value;
		};

	} // namespace details;

	template<typename... _tFroms, typename... _tTos>
	struct is_covariant<std::tuple<_tFroms...>, std::tuple<_tTos...>>
		: integral_constant<bool, details::tuple_element_convertible<
		std::tuple<_tFroms...>, std::tuple<_tTos...>,
		make_natural_sequence_t<sizeof...(_tTos) >> ::value>
	{};


	template<typename... _tFroms, typename... _tTos>
	struct is_contravariant<std::tuple<_tFroms...>, std::tuple<_tTos...>>
		: integral_constant<bool, details::tuple_element_convertible<
		std::tuple<_tTos...>, std::tuple<_tFroms...>,
		make_natural_sequence_t<sizeof...(_tTos) >> ::value>
	{};

	template<typename _tHead, typename... _tTail>
	struct sequence_split<std::tuple<_tHead, _tTail...>>
	{
		using type = _tHead;
		using tail = std::tuple<_tTail...>;
	};


	template<typename... _types1, typename... _types2>
	struct sequence_cat<std::tuple<_types1...>, std::tuple<_types2...>>
	{
		using type = std::tuple<_types1..., _types2...>;
	};


	template<size_t _vIdx, typename... _types>
	struct sequence_element<_vIdx, std::tuple<_types...>>
		: std::tuple_element<_vIdx, std::tuple<_types...>>
	{};

	template<typename... _types, size_t... _vIdxSeq>
	struct sequence_project<std::tuple<_types...>, variadic_sequence<_vIdxSeq...>>
	{
	private:
		using tuple_type = std::tuple<_types...>;

	public:
#if LB_IMPL_MSCPP
		using type = std::tuple<typename
			std::tuple_element<_vIdxSeq, tuple_type>::type...>;
#else
		using type = std::tuple<tuple_element_t<_vIdxSeq, tuple_type>...>;
#endif
	};

	template<typename... _types>
	struct sequence_reverse<std::tuple<_types...>>
	{
	private:
		using tuple_type = std::tuple<_types...>;

	public:
		using type = sequence_project_t<tuple_type, sequence_reverse_t<
			make_natural_sequence_t<std::tuple_size<tuple_type>::value >> >;
	};

	template<typename... _types>
	struct sequence_split_n<0, std::tuple<_types...>>
	{
		using type = std::tuple<>;
		using tail = std::tuple<_types...>;
	};

	template<typename _type, typename... _types>
	struct sequence_split_n<1, std::tuple<_type, _types...>>
	{
		using type = std::tuple<_type>;
		using tail = std::tuple<_types...>;
	};

	template<size_t _vIdx, typename... _types>
	struct sequence_split_n<_vIdx, std::tuple<_types...>>
	{
	private:
		using half = sequence_split_n<_vIdx / 2, std::tuple<_types...>>;
		using last = sequence_split_n<_vIdx - _vIdx / 2, typename half::tail>;

	public:
		using type = sequence_cat_t<typename half::type, typename last::type>;
		using tail = typename last::tail;
	};

	template<class _fBinary, typename _tState>
	struct sequence_fold<_fBinary, _tState, std::tuple<>>
	{
		using type = _tState;
	};

	template<class _fBinary, typename _tState, typename _type>
	struct sequence_fold<_fBinary, _tState, std::tuple<_type>>
	{
		using type = typename _fBinary::template apply<_tState, _type>::type;
	};

	template<class _fBinary, typename _tState, typename... _types>
	struct sequence_fold<_fBinary, _tState, std::tuple<_types...>>
	{
	private:
		using parts
			= sequence_split_n_t<sizeof...(_types) / 2, std::tuple<_types...>>;
		using head = typename parts::type;
		using tail = typename parts::tail;

	public:
		using type = sequence_fold_t<_fBinary,
			sequence_fold_t<_fBinary, _tState, head>, tail>;
	};

#if LB_IMPL_CPP <= 201103L

	template<size_t... I>
	using index_sequence = variadic_sequence<I...>;

	template<size_t>
	struct make_index_sequence;

	template<size_t _vN>
	using make_index_sequence_t = typename make_index_sequence<_vN>::type;

	template<size_t _vN>
	struct make_index_sequence
	{
		using type = make_successor_t<make_index_sequence_t<_vN - 1>>;
	};

	template<>
	struct make_index_sequence<0>
	{
		using type = variadic_sequence<>;
	};

	template<typename... _types>
	using index_sequence_for = make_index_sequence<sizeof...(_types)>;
#endif

	template<class... Args>
	struct type_list
	{
		template<std::size_t N>
		using type = typename std::tuple_element < N, std::tuple<Args...>>::type;
	};

	namespace details
	{
		template<std::size_t N,class C, class R, class... Args>
		decltype(auto) paras_f(R (C::*)(Args...)) {
			using type = typename type_list<Args...>::type<N>;
			return type();
		}

		template<std::size_t N, class C, class R, class... Args>
		decltype(auto) paras_f(R(C::*)(Args...) const) {
			using type = typename type_list<Args...>::type<N>;
			return type();
		}

		template<std::size_t N, class C, class R, class... Args>
		decltype(auto) paras_f(R(__stdcall C::*)(Args...)) {
			using type = typename type_list<Args...>::type<N>;
			return type();
		}

		template<std::size_t N, class P, bool>
		struct paras_index;


		template<std::size_t N, class P>
		struct paras_index<N, P, true>
		{
			using target_type = decltype(&P::operator());
			using type = decltype(paras_f<N>(std::declval<decltype(&P::operator())>()));
		};

		template<std::size_t N, class P>
		struct paras_index<N, P, false>
		{
			using target_type = P;
			using type = decltype(paras_f<N>(target_type()));
		};
	}


	template<std::size_t N, class P>
	struct paras_index : details::paras_index<N,std::decay_t<P>,std::is_class<std::remove_reference_t<P>>::value> {

	};

}

#endif