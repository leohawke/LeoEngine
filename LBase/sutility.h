/*! \file sutility.h
\ingroup LBase
\brief 基础实用设施。

*/
#ifndef LBase_sutility_h
#define LBase_sutility_h 1

#include "LBase/ldef.h"

namespace leo
{
	/*!
	\brief 不可复制对象：禁止派生类调用默认原型的复制构造函数和复制赋值操作符。
	\warning 非虚析构。
	\since change 1.4
	*/
	class noncopyable
	{
	protected:
		/*!
		\brief \c protected 构造：默认实现。
		\note 保护非多态类。
		*/
		lconstfn noncopyable() = default;
		//! \brief \c protected 析构：默认实现。
		~noncopyable() = default;

	public:
		//! \brief 禁止复制构造。
		lconstfn noncopyable(const noncopyable&) = delete;

		/*!
		\brief 允许转移构造。
		\since build 1.4
		*/
		lconstfn
			noncopyable(noncopyable&&) = default;

		//! \brief 禁止复制赋值。
		noncopyable&
			operator=(const noncopyable&) = delete;

		/*!
		\brief 允许转移赋值。
		\since build 1.4
		*/
		noncopyable&
			operator=(noncopyable&&) = default;
	};

	/*
	\brief 不可转移对象：禁止继承此类的对象调用默认原型的转移构造函数和转移赋值操作符。
	\warning 非虚析构。
	\since change 1.4
	*/
	class nonmovable
	{
	protected:
		/*!
		\brief \c protected 构造：默认实现。
		\note 保护非多态类。
		*/
		lconstfn nonmovable() = default;
		//! \brief \c protected 析构：默认实现。
		~nonmovable() = default;

	public:
		//! \since build 1.4
		//@{
		//! \brief 允许复制构造。
		lconstfn nonmovable(const nonmovable&) = default;
		//! \brief 禁止转移构造。
		lconstfn
			nonmovable(nonmovable&&) = delete;

		//! \brief 允许复制赋值。
		nonmovable&
			operator=(const nonmovable&) = default;
		//! \brief 禁止转移赋值。
		nonmovable&
			operator=(const nonmovable&&) = delete;
		//@}
	};


	/*!
	\brief 可动态复制的抽象基类。
	*/
	class LB_API cloneable
	{
	public:
		cloneable() = default;
		cloneable(const cloneable&) = default;
		//! \brief 虚析构：类定义外默认实现。
		virtual
		~cloneable();

		virtual cloneable*
		clone() const = 0;
	};

	/*!
	\brief 间接操作：返回派生类自身引用。
	\note 可用于 ::indirect_input_iterator 和转换函数访问。
	\todo 提供接口允许用户指定使用 ::polymorphic_cast 等检查转换。
	*/
	template<typename _type>
	struct deref_self
	{
		_type&
			operator*() lnothrow
		{
			return *static_cast<_type*>(this);
		}
		const _type&
			operator*() const lnothrow
		{
			return *static_cast<const _type*>(this);
		}
		volatile _type&
			operator*() volatile lnothrow
		{
			return *static_cast<volatile _type*>(this);
		}
		const volatile _type&
			operator*() const volatile lnothrow
		{
			return *static_cast<const volatile _type*>(this);
		}
	};
}


#endif