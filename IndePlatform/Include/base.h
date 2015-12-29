#ifndef IndePlatform_base_h
#define IndePlatform_base_h

#include "ldef.h"

namespace leo
{
	class noncopyable
	{
	protected:
		noncopyable() = default;
		~noncopyable() = default;

	public:
		noncopyable(const noncopyable&) = delete;

		noncopyable&
			operator=(const noncopyable&) = delete;
	};

	class nonmovable
	{
	protected:
		nonmovable() = default;
		~nonmovable() = default;

	public:
		nonmovable(const nonmovable&) = delete;

		nonmovable&
			operator=(const nonmovable&) = delete;
	};


	class LB_API cloneable
	{
	public:
		virtual cloneable*
			clone() const = 0;

		virtual
			~cloneable()
		{}
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