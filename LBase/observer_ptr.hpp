#pragma once

#include "cassert.h"
#include <type_traits>
#include <compare>

namespace leo
{
	//! \since build 1.4
	//@{
	/*!
	\brief 观察者指针：无所有权的智能指针。
	\see WG21 N4529 8.12[memory.observer.ptr] 。
	*/
	template<typename _type>
	class observer_ptr
	{
	public:
		using element_type = _type;
		using pointer = limpl(std::add_pointer_t<_type>);
		using reference = limpl(std::add_lvalue_reference_t<_type>);

	private:
		_type* ptr{};

	public:
		//! \post <tt>get() == nullptr</tt> 。
		//@{
		lconstfn
			observer_ptr() lnothrow limpl(= default);
		lconstfn
			observer_ptr(nullptr_t) lnothrow
			: ptr()
		{}
		//@}
		explicit lconstfn
			observer_ptr(pointer p) lnothrow
			: ptr(p)
		{}
		template<typename _tOther>
		lconstfn
			observer_ptr(observer_ptr<_tOther> other) lnothrow
			: ptr(other.get())
		{}

		//! \pre 断言： <tt>get() != nullptr</tt> 。
		lconstfn reference
			operator*() const lnothrowv
		{
			return lconstraint(get() != nullptr), * ptr;
		}

		lconstfn pointer
			operator->() const lnothrow
		{
			return ptr;
		}

		//! \since build 1.4
		friend lconstfn bool
			operator==(observer_ptr p, nullptr_t) lnothrow
		{
			return !p.ptr;
		}

		explicit lconstfn
			operator bool() const lnothrow
		{
			return ptr;
		}

		explicit lconstfn
			operator pointer() const lnothrow
		{
			return ptr;
		}

		lconstfn pointer
			get() const lnothrow
		{
			return ptr;
		}

		lconstfn_relaxed pointer
			release() lnothrow
		{
			const auto res(ptr);

			reset();
			return res;
		}

		lconstfn_relaxed void
			reset(pointer p = {}) lnothrow
		{
			ptr = p;
		}

		lconstfn_relaxed void
			swap(observer_ptr& other) lnothrow
		{
			std::swap(ptr, other.ptr);
		}

		friend auto operator<=>(const observer_ptr<_type>& p1, const observer_ptr<_type>& p2) = default;
		friend bool operator==(const observer_ptr<_type>& p1, const observer_ptr<_type>& p2) = default;
	};


	template<typename _type>
	auto operator<=>(const observer_ptr<_type>& p1, std::nullptr_t)
		->decltype(p1.get() <=> nullptr)
	{
		return p1.get() <=> nullptr;
	}

	template<typename _type>
	inline void
		swap(observer_ptr<_type>& p1, observer_ptr<_type>& p2) lnothrow
	{
		p1.swap(p2);
	}
	template<typename _type>
	inline observer_ptr<_type>
		make_observer(_type* p) lnothrow
	{
		return observer_ptr<_type>(p);
	}

	//@}
}

namespace std
{

	/*!
	\brief leo::observer_ptr 散列支持。
	\see ISO WG21 N4529 8.12.7[memory.observer.ptr.hash] 。
	\since build 1.4
	*/
	//@{
	template<typename>
	struct hash;

	template<typename _type>
	struct hash<leo::observer_ptr<_type>>
	{
		size_t
			operator()(const leo::observer_ptr<_type>& p) const limpl(lnothrow)
		{
			return hash<_type*>(p.get());
		}
	};
	//@}

} // namespace std;