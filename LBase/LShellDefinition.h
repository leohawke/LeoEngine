/*!	\file LShellDefinition.h
\ingroup Core
\brief 宏定义和类型描述。
*/

#ifndef LEO_Core_LShellDefinition_h
#define LEO_Core_LShellDefinition_h 1

#include <LBase/LAdaptor.h>

namespace leo {

	/*!	\defgroup reset Reset Pointers
	\brief 安全删除指定引用的句柄指向的对象。
	\post 指定引用的句柄值等于 nullptr 。
	*/
	//@{
	template<typename _type>
	inline bool
		reset(_type*& p) lnothrow
	{
		bool b(p);

		ldelete(p);
		p = {};
		return b;
	}
	//@}

	/*!
	\brief 比较： shared_ptr 和内建指针类型的相等关系。
	*/
	template<typename _type>
	inline bool
		operator==(const shared_ptr<_type>& sp, _type* p)
	{
		return sp.get() == p;
	}
	/*!
	\brief 比较： weak_ptr 相等关系。
	\note 注意和 shared_ptr 比较托管对象指针的语义不同。
	*/
	template<typename _type1, typename _type2>
	inline bool
		operator==(const weak_ptr<_type1>& x, const weak_ptr<_type2>& y)
	{
		return !x.owner_before(y) && !y.owner_before(x);
	}

	/*!
	\brief 比较： shared_ptr 左值和内建指针类型的不等关系。
	*/
	template<typename _type>
	inline bool
		operator!=(const shared_ptr<_type>& sp, _type* p)
	{
		return sp.get() != p;
	}
	/*!
	\brief 比较： weak_ptr 不等关系。
	\note 注意和 shared_ptr 比较托管对象指针的语义不同。
	*/
	template<typename _type1, typename _type2>
	inline bool
		operator!=(const weak_ptr<_type1>& x, const weak_ptr<_type2>& y)
	{
		return !(x == y);
	}


	/*!
	\brief 独占所有权的锁定指针：使用线程模型对应的互斥量和锁以及 unique_ptr 。
	\sa leo::threading::locked_ptr
	*/
	template<typename _type, class _tMutex = typename unlock_delete<>::mutex_type,
		class _tLock = typename unlock_delete<_tMutex>::lock_type>
		using locked_ptr = unique_ptr<_type, unlock_delete<_tMutex, _tLock>>;
}

#endif