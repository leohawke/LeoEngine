#ifndef IndePlatform_algorithm_hpp
#define IndePlatform_algorithm_hpp

#include "utility.hpp"
#include "iterator.hpp"
#include <algorithm>
#include "type_op.hpp"
#include <cstring>

namespace leo
{
	/*Gerneral Algorithms
	算法
	范围(序列容器,内键数组) // ISO C++ Clause 23
	*/


	/*!	\defgroup mutating_algorithms Mutating Sequence Operations
	\brief 可变序列操作。
	\see WG21/N3936 25.3[alg.modifying.operations] 。
	*/
	//@{
	/*!
	\tparam _tIn 输入迭代器类型。
	\tparam _tOut 输出迭代器类型。
	\tparam _fPred 谓词类型。
	\param first 被复制序列的起始输入迭代器。
	\param result 输出迭代器。
	\param pred 一元谓词。
	\pre 迭代器可解引用。
	*/
	//@{
	/*!
	\brief 变换满足谓词的序列的连续元素。
	\tparam _fOp 变换操作类型。
	\param op 一元变换操作。
	*/
	template<typename _tIn, typename _tOut, typename _fPred, typename _fOp>
	_tOut
		transform_when(_tIn first, _tOut result, _fPred pred, _fOp op)
	{
		lunseq((lconstraint(!is_undereferenceable(first)), 0),
			(lconstraint(!is_undereferenceable(result)), 0));

		for (; pred(*first); lunseq((++first, 0), (++result, 0)))
			*result = op(*first);
		return result;
	}

	//! \brief 复制满足谓词的序列的连续元素。
	template<typename _tIn, typename _tOut, typename _fPred>
	_tOut
		copy_when(_tIn first, _tOut result, _fPred pred)
	{
		return transform_when(first, result, pred,
			[](decltype(*first)& v) lnothrow{
			return static_cast<decltype(*first)&>(v);
		});
	}


	/*序列转换
	  _fop 操作类型
	  _tOut 输出迭代器类型
	  _fIns 输入迭代器类型
	*/
	template<typename _fOp, typename _tOut, typename... _tIns>
	void
		transform_n(_fOp op, _tOut result, size_t n, _tIns... iters)
	{
		while (n-- != 0)
		{
			lunseq((lconstraint(!is_undereferenceable(result)), 0),
				(lconstraint(!is_undereferenceable(iters)), void(iters), 0)...);
			*result = op((*iters)...);
			lunseq(++result, ++iters...);
		}
	}

	/*POD Type Operations
	  _type 对象类型
	  静态断言:检测是否为POD类型
	  警告:不检测指针有效性
	*/

	template<class _type>
		inline _type*
		pod_fill(_type* first, _type* last, const _type& value)
	{
		static_assert(is_pod<remove_reference_t<_type>>::value,
			"Non-POD type found @ pod_fill;");

		switch ((last - first) & 7)
		{
		case 0:
			while (first != last)
			{
				*first = value; ++first;
		case 7: *first = value; ++first;
		case 6: *first = value; ++first;
		case 5: *first = value; ++first;
		case 4: *first = value; ++first;
		case 3: *first = value; ++first;
		case 2: *first = value; ++first;
		case 1: *first = value; ++first;
			}
		}
		return last;
	}

	template<class _type>
		inline _type*
			pod_copy_n(const _type* first, size_t n, _type* result)
		{
			static_assert(is_pod<remove_reference_t<_type>>::value,
				"Non-POD type found @ pod_copy_n;");

			std::memcpy(result, first, sizeof(*first) * n);
			return result + n;
		}

	template<class _type>
		inline _type*
			pod_copy(const _type* first, const _type* last, _type* result)
		{
			return leo::pod_copy_n(first, last - first, result);
		}

	template<class _type>
		inline _type*
			pod_move_n(const _type* first, size_t n, _type* result)
		{
			static_assert(is_pod<remove_reference_t<_type>>::value,
				"Non-POD type found @ pod_move_n;");

			std::memmove(result, first, sizeof(*first) * n);
			return result + n;
		}

	template<class _type>
		inline _type*
			pod_move(const _type* first, const _type* last, _type* result)
		{
			return leo::pod_move_n(first, last - first, result);
		}

	/*
	去除迭代器范围中的重复元素,且不改变元素之间的相对顺序.
	range[first,last)
	return newlast
	complex time:O(n^2) n ==> std::advance(first,n) == last
	*/
	template<typename _tFwd>
	_tFwd
		stable_range_unique(_tFwd first, _tFwd last)
	{
		_tFwd result(first);

		for (_tFwd i(first); i != last; ++i)
			if (std::find(first, result, *i) == result)
			{
				using std::swap;

				swap(*i, *result);
				++result;
			}
		return result;
	}
}

#endif