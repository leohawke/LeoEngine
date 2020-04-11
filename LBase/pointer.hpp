/*! \file pointer.hpp
\ingroup LBase
\brief ͨ��ָ�롣
\par �޸�ʱ��:
2017-01-02 01:37 +0800
�����չ��׼��ͷ <iterator> ���ṩָ��ĵ�������������װ������ģ�塣
*/

#ifndef LBase_pointer_hpp
#define LBase_pointer_hpp 1

#include "iterator_op.hpp" // for totally_ordered,
//	iterator_operators_t, std::iterator_traits, lconstraint;
#include "observer_ptr.hpp"
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
	  \brief �ɿ�ָ���װ������ \c NullablePointer Ҫ��ͬʱ����ת�ƺ�Ϊ�ա�
	  \tparam _type ����װ��ָ�롣
	  \pre _type ���� \c NullablePointer Ҫ��
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
		//! \pre ���ʽ \c *ptr ��ʽ��
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

	
	
	//@}

	template<typename _type>
	using tidy_ptr = nptr<observer_ptr<_type>>;

	/*!
	\ingroup iterator_adaptors
	\brief ָ���������
	\note ת��Ϊ bool ������ȽϵȲ���ʹ��ת��Ϊ��Ӧָ��ʵ�֡�
	\warning ����������
	\since build 1.4

	ת��ָ��Ϊ�����͵�������ʵ�������
	\todo �� std::pointer_traits ������
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
	\brief ָ���װΪ�����͵�������
	\since build 1.4

	��������ָ���������װΪ pointer_iterator ��
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
