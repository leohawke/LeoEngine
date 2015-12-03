#ifndef IndePlatform_ref_hpp
#define IndePlatform_ref_hpp

#include "ldef.h"
#include <functional>
#include <memory>

namespace leo
{
	//!
	//@{
	/*!
	\brief 左值引用包装。
	\tparam _type 被包装的类型。

	类似 \c std::reference_wrapper 和 \c boost::reference_wrapper 公共接口兼容的
	引用包装类实现。
	和 \c std::reference_wrapper 不同而和 \c boost::reference_wrapper 类似，
	不要求模板参数为完整类型。
	*/
	//@{
	template<typename _type>
	class lref
	{
	public:
		using type = _type;

	private:
		_type* ptr;

	public:
		lconstfn
			lref(_type& t) lnothrow
			: ptr(std::addressof(t))
		{}
		lconstfn
			lref(std::reference_wrapper<_type> t) lnothrow
			: lref(t.get())
		{}

		//! \since build 556
		lref(const lref&) = default;

		//! \since build 556
		lref&
			operator=(const lref&) = default;

		operator _type&() const lnothrow
		{
			return *ptr;
		}

		_type&
			get() const lnothrow
		{
			return *ptr;
		}
	};

	/*!
	\brief 构造引用包装。
	\relates lref
	*/
	//@{
	template<typename _type>
	lconstfn lref<_type>
		ref(_type& t)
	{
		return lref<_type>(t);
	}
	template <class _type>
	void
		ref(const _type&&) = delete;

	template<typename _type>
	lconstfn lref<const _type>
		cref(const _type& t)
	{
		return lref<const _type>(t);
	}
	template<class _type>
	void
		cref(const _type&&) = delete;
	//@}
	//@}


	/*!
	\ingroup unary_type_traits
	\brief 取引用包装实例特征。
	*/
	//@{
	template<typename _type>
	struct wrapped_traits : std::false_type
	{
		using type = _type;
	};

	template<class _tWrapped>
	struct wrapped_traits<std::reference_wrapper<_tWrapped>> : std::true_type
	{
		using type = _tWrapped;
	};

	template<class _tWrapped>
	struct wrapped_traits<lref<_tWrapped>> : std::true_type
	{
		using type = _tWrapped;
	};
	//@}

	/*!
	\ingroup metafunctions
	*/
	template<typename _type>
	using wrapped_traits_t = typename wrapped_traits<_type>::type;


	/*!
	\brief 解除引用包装。
	\note 默认仅提供对 \c std::reference_wrapper 和 lref 的实例类型的重载。
	\note 使用 ADL 。
	*/
	//@{
	template<typename _type>
	lconstfn _type&
		unref(_type&& x) lnothrow
	{
		return x;
	}
	//! \since build 554
	template<typename _type>
	lconstfn _type&
		unref(const lref<_type>& x) lnothrow
	{
		return x.get();
	}
	//@}


	/*!
	\brief 任意对象引用类型。
	\warning 不检查 cv-qualifier 。
	\todo 右值引用版本。
	*/
	class void_ref
	{
	private:
		const volatile void* ptr;

	public:
		template<typename _type>
		lconstfn
			void_ref(_type& obj)
			: ptr(&obj)
		{}

		template<typename _type>
		lconstfn LB_PURE
			operator _type&() const
		{
			return *static_cast<_type*>(&*this);
		}

		LB_PURE void*
			operator&() const volatile
		{
			return const_cast<void*>(ptr);
		}
	};
}


#endif
