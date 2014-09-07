#ifndef IndePlatform_variadic_hpp
#define IndePlatform_variadic_hpp

#include "ldef.h"

namespace leo
{
	template<size_t... _vSeq>
	struct variadic_sequence
	{
		//note Í¬ C++1y std::integer_sequence::size ¡£
		static lconstfn size_t
			size()
		{
			return sizeof...(_vSeq);
		}
	};

	template<class>
	struct sequence_split;

	template<class _tSeq>
	using sequence_split_t = typename sequence_split<_tSeq>::type;

	template<size_t _vHead, size_t... _vTail>
	struct sequence_split<variadic_sequence<_vHead, _vTail...>>
	{
		static lconstexpr size_t value = _vHead;

		using type = variadic_sequence<value>;
		using tail = variadic_sequence<_vTail...>;
	};

	template<class, class>
	struct sequence_cat;

	template<class _tSeq1, class _tSeq2>
	using sequence_cat_t = typename sequence_cat<_tSeq1, _tSeq2>::type;

	template<size_t... _vSeq1, size_t... _vSeq2>
	struct sequence_cat<variadic_sequence<_vSeq1...>, variadic_sequence<_vSeq2...>>
	{
		using type = variadic_sequence<_vSeq1..., _vSeq2...>;
	};

	template<size_t, class>
	struct sequence_element;

	template<size_t _vIdx, class _tSeq>
	using sequence_element_t = typename sequence_element<_vIdx, _tSeq>::type;

	template<size_t _vIdx>
	struct sequence_element<_vIdx, variadic_sequence<>>;

	template<size_t... _vSeq>
	struct sequence_element<0, variadic_sequence<_vSeq...>>
	{
	private:
		using vseq = variadic_sequence<_vSeq...>;

	public:
		static lconstexpr auto value = sequence_split<vseq>::value;

		using type = sequence_split_t<vseq>;
	};

	template<size_t _vIdx, size_t... _vSeq>
	struct sequence_element<_vIdx, variadic_sequence<_vSeq...>>
	{
	private:
		using sub = sequence_element<_vIdx - 1,
			typename sequence_split<variadic_sequence<_vSeq...>>::tail>;

	public:
		static lconstexpr auto value = sub::value;
		using type = typename sub::type;
	};

	template<class, class>
	struct sequence_project;

	template<class _tSeq, class _tIdxSeq>
	using sequence_project_t = typename sequence_project<_tSeq, _tIdxSeq>::type;

	template<size_t... _vSeq, size_t... _vIdxSeq>
	struct sequence_project<variadic_sequence<_vSeq...>,
		variadic_sequence<_vIdxSeq... >>
	{
		using type = variadic_sequence<
			sequence_element<_vIdxSeq, variadic_sequence<_vSeq...>>::value...>;
	};

	template<class>
	struct sequence_reverse;

	template<class _tSeq>
	using sequence_reverse_t = typename sequence_reverse<_tSeq>::type;

	template<>
	struct sequence_reverse<variadic_sequence<>>
	{
		using type = variadic_sequence<>;
	};

	template<size_t... _vSeq>
	struct sequence_reverse<variadic_sequence<_vSeq...>>
	{
	private:
		using vseq = variadic_sequence<_vSeq...>;

	public:
		using type = sequence_cat_t<sequence_reverse_t<typename
			sequence_split<vseq>::tail>, sequence_split_t<vseq >> ;
	};

	template<size_t, class>
	struct sequence_split_n;

	template<size_t _vIdx, class _tSeq>
	using sequence_split_n_t = typename sequence_split_n<_vIdx, _tSeq>::type;

	template<size_t... _vSeq>
	struct sequence_split_n<0, variadic_sequence<_vSeq...>>
	{
		using type = variadic_sequence<>;
		using tail = variadic_sequence<_vSeq...>;
	};

	template<size_t _vHead, size_t... _vSeq>
	struct sequence_split_n<1, variadic_sequence<_vHead, _vSeq...>>
	{
		using type = variadic_sequence<_vHead>;
		using tail = variadic_sequence<_vSeq...>;
	};

	template<size_t _vIdx, size_t... _vSeq>
	struct sequence_split_n<_vIdx, variadic_sequence<_vSeq...>>
	{
	private:
		using half = sequence_split_n<_vIdx / 2, variadic_sequence<_vSeq...>>;
		using last = sequence_split_n<_vIdx - _vIdx / 2, typename half::tail>;

	public:
		using type = sequence_cat_t<typename half::type, typename last::type>;
		using tail = typename last::tail;
	};

	template<class, typename, class>
	struct sequence_fold;

	template<class _fBinary, typename _tState, class _type>
	using sequence_fold_t = typename sequence_fold<_fBinary, _tState, _type>::type;

	template<class _fBinary, class _tState>
	struct sequence_fold<_fBinary, _tState, variadic_sequence<>>
	{
		using type = _tState;

		static lconstexpr auto value = _tState::value;
	};

	template<class _fBinary, class _tState, size_t _vHead>
	struct sequence_fold<_fBinary, _tState, variadic_sequence<_vHead>>
	{
		static lconstexpr auto value = _fBinary()(_tState::value, _vHead);

		using type = variadic_sequence<value>;
	};

	template<class _fBinary, class _tState, size_t... _vSeq>
	struct sequence_fold<_fBinary, _tState, variadic_sequence<_vSeq...>>
	{
	private:
		using parts
			= sequence_split_n<sizeof...(_vSeq) / 2, variadic_sequence<_vSeq...>>;
		using head = typename parts::type;
		using tail = typename parts::tail;

	public:
		static lconstexpr auto value = sequence_fold<_fBinary,
			std::integral_constant<size_t,
			sequence_fold<_fBinary, _tState, head>::value>, tail>::value;

		using type = variadic_sequence<value>;
	};

	template<class _fBinary, size_t _vState, size_t... _vSeq>
	using vseq_fold = sequence_fold<_fBinary,
		std::integral_constant<size_t, _vState>, variadic_sequence<_vSeq... >> ;

	template<class>
	struct make_successor;

	template<class _tSeq>
	using make_successor_t = typename make_successor<_tSeq>::type;

	template<size_t... _vSeq>
	struct make_successor<variadic_sequence<_vSeq...>>
	{
		using type = variadic_sequence<_vSeq..., sizeof...(_vSeq)>;
	};

	template<size_t>
	struct make_natural_sequence;

	template<size_t _vN>
	using make_natural_sequence_t = typename make_natural_sequence<_vN>::type;

	template<size_t _vN>
	struct make_natural_sequence
	{
		using type = make_successor_t<make_natural_sequence_t<_vN - 1>>;
	};

	template<>
	struct make_natural_sequence<0>
	{
		using type = variadic_sequence<>;
	};
}

#endif