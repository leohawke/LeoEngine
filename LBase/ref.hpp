/*!	\file ref.hpp
\ingroup LBase
\brief 引用包装。
扩展标准库头 <functional> ，提供替代 std::reference_wrapper 的接口。
*/

#ifndef LBase_ref_hpp
#define LBase_ref_hpp 1

#include "LBase/addressof.hpp"// for "addressof.hpp", leo::constfn_addressof,
//	exclude_self_t, cond_t, not_, is_object;
#include <functional> // for std::reference_wrapper;

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
			: ptr(leo::addressof(t))
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

	//! \since build 1.4
	//@{
	/*!
	\ingroup unary_type_traits
	\brief 判断模板参数指定的类型是否。
	\note 接口含义类似 boost::is_reference_wrapper 。
	*/
	//@{
	template<typename _type>
	struct is_reference_wrapper : false_type
	{};

	template<typename _type>
	struct is_reference_wrapper<std::reference_wrapper<_type>> : true_type
	{};

	template<typename _type>
	struct is_reference_wrapper<lref<_type>> : true_type
	{};
	//@}

	/*!
	\ingroup metafunctions
	\brief 取引用包装的类型或未被包装的模板参数类型。
	\note 接口含义类似 boost::unwrap_reference 。
	\since build 1.4
	*/
	//@{
	template<typename _type>
	struct unwrap_reference
	{
		using type = _type;
	};

	template<typename _type>
	using unwrap_reference_t = _t<unwrap_reference<_type>>;

	template<typename _type>
	struct unwrap_reference<std::reference_wrapper<_type>>
	{
		using type = _type;
	};

	template<typename _type>
	struct unwrap_reference<lref<_type>>
	{
		using type = _type;
	};
	//@}
	//@}


	//@{
	template<typename _type>
	struct decay_unwrap : unwrap_reference<decay_t<_type>>
	{};

	template<typename _type>
	using decay_unwrap_t = _t<decay_unwrap<_type>>;
	//@}
	//@}


	/*!
	\brief 包装间接操作的引用适配器。
	*/
	template<typename _type, typename _tReference = lref<_type>>
	class indirect_ref_adaptor
	{
	public:
		using value_type = _type;
		using reference = _tReference;

	private:
		reference ref;

	public:
		indirect_ref_adaptor(value_type& r)
			: ref(r)
		{}

		auto
			get() lnothrow -> decltype(ref.get().get())
		{
			return ref.get().get();
		}
	};


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

	/*!
	\brief 伪输出对象。
	\note 吸收所有赋值操作。
	\since build 1.4
	*/
	struct pseudo_output
	{
		//! \since build 690
		//@{
		template<typename... _tParams>
		lconstfn
			pseudo_output(_tParams&&...) lnothrow
		{}

		template<typename _tParam,
			limpl(exclude_self_t<pseudo_output, _tParam>)>
			lconstfn const pseudo_output&
			operator=(_tParam&&) const lnothrow
		{
			return *this;
		}
		template<typename... _tParams>
		lconstfn const pseudo_output&
			operator()(_tParams&&...) const lnothrow
		{
			return *this;
		}
		//@}
	};

	/*!
	\ingroup metafunctions
	\since build 1.4
	\see 关于相关的核心语言特性： WG21 P0146R0 。
	*/
	//@{
	//! \brief 若类型不是空类型则取后备结果类型（默认为 pseudo_output ）。
	template<typename _type, typename _tRes = pseudo_output>
	using nonvoid_result_t = cond_t<not_<is_void<_type>>, _type, pseudo_output>;

	//! \brief 若类型不是对象类型则取后备结果类型（默认为 pseudo_output ）。
	template<typename _type, typename _tRes = pseudo_output>
	using object_result_t = cond_t<is_object<_type>, _type, pseudo_output>;
	//@}

}

#endif
