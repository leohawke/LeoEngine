#ifndef LBase_placement_hpp
#define LBase_placement_hpp 1

#include "LBase/addressof.hpp"
#include "LBase/deref_op.hpp"
#include "LBase/cassert.h"

#include <new>
#include <iterator>

namespace leo {

	/*!
	\brief 默认初始化标记。
	*/
	lconstexpr const struct default_init_t {} default_init{};

	/*!
	\brief 值初始化标记。
	*/
	lconstexpr const struct value_init_t {} value_init{};


	/*!
	\see WG21 P0032R3 。
	\see WG21 N4606 20.2.7[utility.inplace] 。
	*/
	//@{
	//! \brief 原地标记类型。
	struct in_place_tag
	{
		in_place_tag() = delete;
	};

	//! \brief 原地空标记类型。
	using in_place_t = in_place_tag(&)(limpl(empty_base<>));

	//! \brief 原地类型标记模板。
	template<typename _type>
	using in_place_type_t = in_place_tag(&)(limpl(empty_base<_type>));

	//! \brief 原地索引标记模板。
	template<size_t _vIdx>
	using in_place_index_t = in_place_tag(&)(limpl(size_t_<_vIdx>));

#ifdef LB_IMPL_MSCPP
#pragma warning(disable:4646)
#endif

	/*!
	\ingroup helper_functions
	\brief 原地标记函数。
	\warning 调用引起未定义行为。
	*/
	limpl(LB_NORETURN) inline in_place_tag
		in_place(limpl(empty_base<>))
	{
		LB_ASSUME(false);
	}
	template<typename _type>
	limpl(LB_NORETURN) in_place_tag
		in_place(limpl(empty_base<_type>))
	{
		LB_ASSUME(false);
	}
	template<size_t _vIdx>
	limpl(LB_NORETURN) in_place_tag
		in_place(limpl(size_t_<_vIdx>))
	{
		LB_ASSUME(false);
	}
	//@}


	/*!
	\tparam _type 构造的对象类型。
	\param obj 构造的存储对象。
	*/
	//@{
	//! \brief 以默认初始化在对象中构造。
	template<typename _type, typename _tObj>
	inline _type*
		construct_default_within(_tObj& obj)
	{
		return ::new(static_cast<void*>(static_cast<_tObj*>(
			constfn_addressof(obj)))) _type;
	}

	/*!
	\brief 以值初始化在对象中构造。
	\tparam _tParams 用于构造对象的参数包类型。
	\param args 用于构造对象的参数包。
	*/
	template<typename _type, typename _tObj, typename... _tParams>
	inline _type*
		construct_within(_tObj& obj, _tParams&&... args)
	{
		return ::new(static_cast<void*>(static_cast<_tObj*>(
			constfn_addressof(obj)))) _type(lforward(args)...);
	}
	//@}

	/*!
	\brief 以默认初始化原地构造。
	\tparam _tParams 用于构造对象的参数包类型。
	\param args 用于构造对象的参数包。
	*/
	template<typename _type>
	inline void
		construct_default_in(_type& obj)
	{
		construct_default_within<_type>(obj);
	}

	/*!
	\brief 以值初始化原地构造。
	\tparam _tParams 用于构造对象的参数包类型。
	\param args 用于构造对象的参数包。
	*/
	template<typename _type, typename... _tParams>
	inline void
		construct_in(_type& obj, _tParams&&... args)
	{
		construct_within<_type>(obj, yforward(args)...);
	}

	//@{
	//! \tparam _tIter 迭代器类型。
	//@{
	/*!
	\tparam _tParams 用于构造对象的参数包类型。
	\param args 用于构造对象的参数包。
	\pre 断言：指定范围末尾以外的迭代器满足 <tt>!is_undereferenceable</tt> 。
	*/
	//@{
	/*!
	\brief 使用指定参数在迭代器指定的位置以指定参数初始化构造对象。
	\param i 迭代器。
	\note 显式转换为 void* 指针以实现标准库算法 uninitialized_* 实现类似的语义。
	\see libstdc++ 5 和 Microsoft VC++ 2013 标准库在命名空间 std 内对指针类型的实现：
	_Construct 模板。
	*/
	template<typename _tIter, typename... _tParams>
	void
		construct(_tIter i, _tParams&&... args)
	{
		using value_type = typename std::iterator_traits<_tIter>::value_type;

		lconstraint(!is_undereferenceable(i));
		construct_within<value_type>(*i, yforward(args)...);
	}

	/*!
	\brief 使用指定参数在迭代器指定的位置以默认初始化构造对象。
	\param i 迭代器。
	*/
	template<typename _tIter>
	void
		construct_default(_tIter i)
	{
		using value_type = typename std::iterator_traits<_tIter>::value_type;

		lconstraint(!is_undereferenceable(i));
		construct_default_within<value_type>(*i);
	}

	/*!
	\brief 使用指定的参数重复构造迭代器范围内的对象序列。
	\note 参数被传递的次数和构造的对象数相同。
	*/
	template<typename _tIter, typename... _tParams>
	void
		construct_range(_tIter first, _tIter last, _tParams&&... args)
	{
		for (; first != last; ++first)
			construct(first, yforward(args)...);
	}
	//@}


	/*!
	\brief 原地销毁。
	\see WG21 N4606 20.10.10.7[specialized.destroy] 。
	*/
	//@{
	/*!
	\see libstdc++ 5 和 Microsoft VC++ 2013 标准库在命名空间 std 内对指针类型的实现：
	_Destroy 模板。
	*/
	template<typename _type>
	inline void
		destroy_at(_type* location)
	{
		lconstraint(location);
		location->~_type();
	}

	//! \see libstdc++ 5 标准库在命名空间 std 内对迭代器范围的实现： _Destroy 模板。
	template<typename _tFwd>
	inline void
		destroy(_tFwd first, _tFwd last)
	{
		for (; first != last; ++first)
			destroy_at(std::addressof(*first));
	}

	template<typename _tFwd, typename _tSize>
	inline _tFwd
		destroy_n(_tFwd first, _tSize n)
	{
		// XXX: To reduce dependency on resolution of LWG 2598.
		static_assert(is_lvalue_reference<decltype(*first)>(),
			"Invalid iterator reference type found.");

		// XXX: Excessive order refinment by ','?
		for (; n > 0; static_cast<void>(++first), --n)
			destroy_at(std::addressof(*first));
		return first;
	}
	//@}
	//@}


	/*!
	\brief 原地析构。
	\tparam _type 用于析构对象的类型。
	\param obj 析构的对象。
	\sa destroy_at
	*/
	template<typename _type>
	inline void
		destruct_in(_type& obj)
	{
		obj.~_type();
	}

	/*!
	\brief 析构迭代器指向的对象。
	\param i 迭代器。
	\pre 断言：<tt>!is_undereferenceable(i)</tt> 。
	\sa destroy
	*/
	template<typename _tIter>
	void
		destruct(_tIter i)
	{
		using value_type = typename std::iterator_traits<_tIter>::value_type;

		lconstraint(!is_undereferenceable(i));
		destruct_in<value_type>(*i);
	}

	/*!
	\brief 析构d迭代器范围内的对象序列。
	\note 保证顺序析构。
	\sa destroy
	*/
	template<typename _tIter>
	void
		destruct_range(_tIter first, _tIter last)
	{
		for (; first != last; ++first)
			destruct(first);
	}
	//@}
}

#endif
