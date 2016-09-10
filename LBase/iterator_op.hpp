/*!	\file iterator_op.hpp
\ingroup LBase
\brief 迭代器操作。
*/

#ifndef LBase_iterator_op_hpp
#define LBase_iterator_op_hpp 1

#include "LBase/range.hpp" // for std::next, std::prev, std::make_move_iterator,
//	std::reverse_iterator, std::make_pair, begin, end;
#include "LBase/cassert.h" // for lconstraint;
#include "LBase/deref_op.hpp" // for is_undereferenceable;
#include "LBase/operators.hpp" // for input_iteratable, output_iteratable,
//	forward_iteratable, bidirectional_iteratable, random_access_iteratable;

namespace leo
{
	/*!	\defgroup iterator_operations Iterator Operations
	\ingroup iterators
	\brief 迭代器操作。
	\since build 1.4
	*/
	//@{
	/*!
	\brief 迭代器指向的值满足条件时取邻接迭代器，否则取原值。
	\param i 指定的迭代器。
	\pre 迭代器可解引用，蕴含断言：<tt>!is_undereferenceable(i)</tt> 。
	*/
	//@{
	template<typename _tIn, typename _fPred>
	_tIn
		next_if(_tIn i, _fPred f,
			typename std::iterator_traits<_tIn>::difference_type n = 1)
	{
		lconstraint(!is_undereferenceable(i));
		return f(*i) ? std::next(i, n) : i;
	}
	template<typename _tIn, typename _type>
	_tIn
		next_if_eq(_tIn i, const _type& val,
			typename std::iterator_traits<_tIn>::difference_type n = 1)
	{
		lconstraint(!is_undereferenceable(i));
		return *i == val ? std::next(i, n) : i;
	}

	/*!
	\brief 迭代器指向的值满足条件时取反向邻接迭代器，否则取原值。
	\pre 迭代器可解引用。
	*/
	//@{
	template<typename _tBi, typename _fPred>
	_tBi
		prev_if(_tBi i, _fPred f,
			typename std::iterator_traits<_tBi>::difference_type n = 1)
	{
		return f(*i) ? std::prev(i, n) : i;
	}
	template<typename _tBi, typename _type>
	_tBi
		prev_if_eq(_tBi i, const _type& val,
			typename std::iterator_traits<_tBi>::difference_type n = 1)
	{
		return *i == val ? std::prev(i, n) : i;
	}
	//@}
	//@}

	//! \ingroup helper_functions
	//@{
	/*!
	\brief 构造反向迭代器。
	\see WG21 N3936 24.5.1.3.21[reverse.iter.make] 。
	\see http://wg21.cmeerw.net/lwg/issue2285 。
	\since build 1.4
	*/
#if __cpp_lib_make_reverse_iterator >= 201402 || __cplusplus >= 201402L
	using std::make_reverse_iterator;
#else
	template<typename _tIter>
	std::reverse_iterator<_tIter>
		make_reverse_iterator(_tIter i)
	{
		return std::reverse_iterator<_tIter>(i);
	}
#endif

	/*!
	\brief 构造转移迭代器对。
	*/
	template<typename _tIterator1, typename _tIterator2>
	inline auto
		make_move_iterator_pair(_tIterator1 it1, _tIterator2 it2) -> decltype(
			std::make_pair(std::make_move_iterator(it1), std::make_move_iterator(it2)))
	{
		return std::make_pair(std::make_move_iterator(it1),
			std::make_move_iterator(it2));
	}

	/*!
	\brief 构造指定序列范围（包含序列容器及内建数组等）的转移迭代器对。
	\note 使用 ADL \c begin 和 \c end 指定范围迭代器。
	\bug decltype 指定的返回类型不能使用 ADL 。
	\since build 1.4
	*/
	template<typename _tRange>
	inline auto
		make_move_iterator_pair(_tRange& c)
		-> decltype(leo::make_move_iterator_pair(begin(c), end(c)))
	{
		return leo::make_move_iterator_pair(begin(c), end(c));
	}

	namespace details
	{

		template<typename _tIter, typename _tTraits,
			typename _tTag = typename _tTraits::iterator_category>
		struct iterator_operators;

		template<typename _tIter, typename _tTraits>
		struct iterator_operators<_tIter, _tTraits, std::input_iterator_tag>
		{
			using type = input_iteratable<_tIter, typename _tTraits::reference>;
		};

		template<typename _tIter, typename _tTraits>
		struct iterator_operators<_tIter, _tTraits, std::output_iterator_tag>
		{
			using type = output_iteratable<_tIter, typename _tTraits::reference>;
		};

		template<typename _tIter, typename _tTraits>
		struct iterator_operators<_tIter, _tTraits, std::forward_iterator_tag>
		{
			using type = forward_iteratable<_tIter, typename _tTraits::reference>;
		};

		template<typename _tIter, typename _tTraits>
		struct iterator_operators<_tIter, _tTraits, std::bidirectional_iterator_tag>
		{
			using type = bidirectional_iteratable<_tIter, typename _tTraits::reference>;
		};

		template<typename _tIter, typename _tTraits>
		struct iterator_operators<_tIter, _tTraits, std::random_access_iterator_tag>
		{
			using type = random_access_iteratable<_tIter,
				typename _tTraits::difference_type, typename _tTraits::reference>;
		};

	} // namespace details;


	  /*!
	  \ingroup metafunctions
	  \brief 按迭代器类别取可实现迭代器的重载操作符集合的实现。
	  \note 仅使用第二参数的特定成员，可以是兼容 std::iterator 的实例的类型。
	  \warning 实例作为实现迭代器的基类时不应使用默认参数，因为此时无法访问成员类型。
	  */
	template<typename _tIter, typename _tTraits = std::iterator_traits<_tIter>>
	//using iterator_operators_t = _t<details::iterator_operators<_tIter, _tTraits>>;
	using iterator_operators_t =typename details::iterator_operators<_tIter, _tTraits>::type;

}

#endif
