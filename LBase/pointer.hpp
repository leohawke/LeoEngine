/*! \file pointer.hpp
\ingroup LBase
\brief 通用指针。
\par 修改时间:
2017-01-02 01:37 +0800
间接扩展标准库头 <iterator> ，提供指针的迭代器适配器包装和其它模板。
*/

#ifndef LBase_pointer_hpp
#define LBase_pointer_hpp 1

#include "iterator_op.hpp" // for totally_ordered,
//	iterator_operators_t, std::iterator_traits, lconstraint;
#include <functional> //for std::equal_to,std::less

namespace leo
{

	//! \since build 1.4
	namespace details
	{
		template<typename _type>
		using nptr_eq1 = bool_<_type() == _type()>;
		template<typename _type>
		using nptr_eq2 = bool_<_type(nullptr) == nullptr>;

	} // namespace details;

	  //! \since build 1.4
	  //@{
	  /*!
	  \brief 可空指针包装：满足 \c NullablePointer 要求同时满足转移后为空。
	  \tparam _type 被包装的指针。
	  \pre _type 满足 \c NullablePointer 要求。
	  */
	template<typename _type>
	class nptr : private totally_ordered<nptr<_type>>
	{
		//! \since build 1.4
		static_assert(is_nothrow_copyable<_type>::value, "Invalid type found.");
		static_assert(is_destructible<_type>::value, "Invalid type found.");
		static_assert(detected_or_t<true_, details::nptr_eq1, _type>::value,
			"Invalid type found.");
#ifndef LB_IMPL_MSCPP
		static_assert(detected_or_t<true_, details::nptr_eq2, _type>::value,
			"Invalid type found.");
#endif

	public:
		using pointer = _type;

	private:
		pointer ptr{};

	public:
		nptr() = default;
		//! \since build 1.4
		//@{
		lconstfn
			nptr(std::nullptr_t) lnothrow
			: nptr()
		{}
		nptr(pointer p) lnothrow
			: ptr(p)
		{}
		//@}
		nptr(const nptr&) = default;
		nptr(nptr&& np) lnothrow
		{
			np.swap(*this);
		}

		nptr&
			operator=(const nptr&) = default;
		nptr&
			operator=(nptr&& np) lnothrow
		{
			np.swap(*this);
			return *this;
		}

		lconstfn bool
			operator!() const lnothrow
		{
			return bool(*this);
		}

		//! \since build 1.4
		//@{
		//! \pre 表达式 \c *ptr 合式。
		//@{
		lconstfn_relaxed auto
			operator*() lnothrow -> decltype(*ptr)
		{
			return *ptr;
		}
		lconstfn auto
			operator*() const lnothrow -> decltype(*ptr)
		{
			return *ptr;
		}
		//@}

		lconstfn_relaxed pointer&
			operator->() lnothrow
		{
			return ptr;
		}
		lconstfn const pointer&
			operator->() const lnothrow
		{
			return ptr;
		}
		//@}

		//! \since build 1.4
		friend lconstfn bool
			operator==(const nptr& x, const nptr& y) lnothrow
		{
			return std::equal_to<pointer>()(x.ptr, y.ptr);
		}

		//! \since build 1.4
		friend lconstfn bool
			operator<(const nptr& x, const nptr& y) lnothrow
		{
			return std::less<pointer>()(x.ptr, y.ptr);
		}

		//! \since build 1.4
		explicit lconstfn
			operator bool() const lnothrow
		{
			return bool(ptr);
		}

		//! \since build 1.4
		lconstfn const pointer&
			get() const lnothrow
		{
			return ptr;
		}

		lconstfn_relaxed pointer&
			get_ref() lnothrow
		{
			return ptr;
		}

		//! \since build 1.4
		void
			swap(nptr& np) lnothrow
		{
			using std::swap;

			swap(ptr, np.ptr);
		}
	};

	/*!
	\relates nptr
	\since build 1.4
	*/
	template<typename _type>
	inline void
		swap(nptr<_type>& x, nptr<_type>& y) lnothrow
	{
		x.swap(y);
	}
	//@}

	//! \since build 1.4
	//@{
	/*!
	\brief 观察者指针：无所有权的智能指针。
	\see WG21 N4529 8.12[memory.observer.ptr] 。
	*/
	template<typename _type>
	class observer_ptr : private totally_ordered<observer_ptr<_type>>,
		private equality_comparable<observer_ptr<_type>, nullptr_t>
	{
	public:
		using element_type = _type;
		using pointer = limpl(add_pointer_t<_type>);
		using reference = limpl(add_lvalue_reference_t<_type>);

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
			return lconstraint(get() != nullptr), *ptr;
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
	};

	//! \relates observer_ptr
	//@{
	//! \since build 1.4
	//@{
	template<typename _type1, typename _type2>
	lconstfn bool
		operator==(observer_ptr<_type1> p1, observer_ptr<_type2> p2) lnothrowv
	{
		return p1.get() == p2.get();
	}

	template<typename _type1, typename _type2>
	lconstfn bool
		operator!=(observer_ptr<_type1> p1, observer_ptr<_type2> p2) lnothrowv
	{
		return !(p1 == p2);
	}

	template<typename _type1, typename _type2>
	lconstfn bool
		operator<(observer_ptr<_type1> p1, observer_ptr<_type2> p2) lnothrowv
	{
		return std::less<common_type_t<_type1, _type2>>(p1.get(), p2.get());
	}
	//@}

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
	//@}

	template<typename _type>
	using tidy_ptr = nptr<observer_ptr<_type>>;

	/*!
	\ingroup iterator_adaptors
	\brief 指针迭代器。
	\note 转换为 bool 、有序比较等操作使用转换为对应指针实现。
	\warning 非虚析构。
	\since build 1.4

	转换指针为类类型的随机访问迭代器。
	\todo 和 std::pointer_traits 交互。
	*/
	template<typename _type>
	class pointer_iterator : public iterator_operators_t<pointer_iterator<_type>,
		std::iterator_traits<_type* >>
	{
	public:
		using iterator_type = _type*;
		using iterator_category
			= typename std::iterator_traits<iterator_type>::iterator_category;
		using value_type = typename std::iterator_traits<iterator_type>::value_type;
		using difference_type
			= typename std::iterator_traits<iterator_type>::difference_type;
		using pointer = typename std::iterator_traits<iterator_type>::pointer;
		using reference = typename std::iterator_traits<iterator_type>::reference;

	protected:
		//! \since build 1.4
		pointer raw;

	public:
		lconstfn
			pointer_iterator(nullptr_t = {})
			: raw()
		{}
		//! \since build 1.4
		template<typename _tPointer>
		lconstfn
			pointer_iterator(_tPointer&& ptr)
			: raw(lforward(ptr))
		{}
		inline
			pointer_iterator(const pointer_iterator&) = default;

		//! \since build 1.4
		//@{
		lconstfn_relaxed pointer_iterator&
			operator+=(difference_type n) lnothrowv
		{
			lconstraint(raw);
			raw += n;
			return *this;
		}

		lconstfn_relaxed pointer_iterator&
			operator-=(difference_type n) lnothrowv
		{
			lconstraint(raw);
			raw -= n;
			return *this;
		}

		//! \since build 1.4
		lconstfn reference
			operator*() const lnothrowv
		{
			return lconstraint(raw), *raw;
		}

		lconstfn_relaxed pointer_iterator&
			operator++() lnothrowv
		{
			lconstraint(raw);
			++raw;
			return *this;
		}

		lconstfn_relaxed pointer_iterator&
			operator--() lnothrowv
		{
			--raw;
			return *this;
		}

		//! \since build 1.4
		friend lconstfn bool
			operator==(const pointer_iterator& x, const pointer_iterator& y) lnothrow
		{
			return x.raw == y.raw;
		}

		//! \since build 1.4
		friend lconstfn bool
			operator<(const pointer_iterator& x, const pointer_iterator& y) lnothrow
		{
			return x.raw < y.raw;
		}

		lconstfn
			operator pointer() const lnothrow
		{
			return raw;
		}
		//@}
	};


	/*!
	\ingroup transformation_traits
	\brief 指针包装为类类型迭代器。
	\since build 1.4

	若参数是指针类型则包装为 pointer_iterator 。
	*/
	//@{
	template<typename _type>
	struct pointer_classify
	{
		using type = _type;
	};

	template<typename _type>
	struct pointer_classify<_type*>
	{
		using type = pointer_iterator<_type>;
	};
	//@}

} // namespace leo;

#endif
