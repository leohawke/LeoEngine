////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   LBase/utility.hpp
//  Version:     v1.01
//  Created:     ?/?/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 
// -------------------------------------------------------------------------
//  History:
//		2014-3-9 14:54 加入新的工具类{临时状态暂存对象的 scope guard}
//		2016-7-15 14:03 代码重写
//
////////////////////////////////////////////////////////////////////////////
#ifndef LBase_utility_hpp
#define LBase_utility_hpp 1

#include "LBase/type_pun.hpp" // for "type_pun.hpp", is_standard_layout,
//	pun_storage_t, std::swap, aligned_replace_cast, exclude_self_t,
//	replace_storage_t;
#include "LBase/memory.hpp" // for lassume, construct_in, destruct_in;
#include "LBase/sutility.h"
#include <functional> // for std::bind, std::ref

namespace leo
{
	//! \ingroup helper_functions
	//@{
	/*!
	\brief 转换 const 引用。
	\see WG21 N4380 。
	\since build 1.4
	*/
	template<typename _type>
	inline add_const_t<_type>&
		as_const(_type& t)
	{
		return t;
	}

	/*!
	\brief 交换相同标准布局类型可修改左值的存储。
	\since build 1.4
	*/
	template<typename _type>
	void
		swap_underlying(_type& x, _type& y) lnothrow
	{
		static_assert(is_standard_layout<_type>(),
			"Invalid underlying type found.");
		using utype = pun_storage_t<_type>;

		std::swap(leo::aligned_replace_cast<utype&>(x),
			leo::aligned_replace_cast<utype&>(y));
	}


	inline namespace cpp2014
	{

#if __cpp_lib_exchange_function >= 201304 || __cplusplus > 201103L
		using std::exchange;
#else
		/*!
		\brief 交换值并返回旧值。
		\return 被替换的原值。
		\see WG21 N3797 20.2.3[utility.exchange] 。
		\see http://www.open-std.org/JTC1/sc22/WG21/docs/papers/2013/n3668.html 。
		*/
		template<typename _type, typename _type2 = _type>
		_type
			exchange(_type& obj, _type2&& new_val)
		{
			_type old_val = std::move(obj);

			obj = std::forward<_type2>(new_val);
			return old_val;
		}
		//@}
#endif

	} // inline namespace cpp2014;


	/*!
	\ingroup helper_functions
	\brief 退化复制。
	\see ISO C++11 30.2.6 [thread.decaycopy] 。
	\see WG21 N3255 。
	\since build 1.4
	*/
	template<typename _type>
	lconstfn limpl(enable_if_convertible_t) < _type, decay_t<_type>, decay_t<_type> >
		decay_copy(_type&& arg)
	{
		return std::forward<_type>(arg);
	}


	/*!
	\brief 取枚举值的底层整数。
	\since build 1.4
	*/
	template<typename _type, limpl(typename = enable_if_t<is_enum<_type>::value>)>
	lconstfn underlying_type_t<_type>
		underlying(_type val) lnothrow
	{
		return underlying_type_t<_type>(val);
	}


	/*!
	\brief 默认初始化标记。
	\since build 1.4
	*/
	lconstexpr const struct default_init_t {} default_init{};

	/*!
	\brief 包装类类型的值的对象。
	\warning 非虚析构。
	*/
	template<typename _type>
	struct boxed_value
	{
		mutable _type value;

		//! \since build 1.4
		//@{
		lconstfn
			boxed_value() lnoexcept(is_nothrow_constructible<_type>())
			: value()
		{}
		lconstfn
			boxed_value(default_init_t) lnothrow
		{}
		template<typename _tParam,
			limpl(typename = exclude_self_t<boxed_value, _tParam>)>
			lconstfn
			boxed_value(_tParam&& arg)
			lnoexcept(is_nothrow_constructible<_type, _tParam&&>())
			: value(lforward(arg))
		{}
		template<typename _tParam1, typename _tParam2, typename... _tParams>
		lconstfn
			boxed_value(_tParam1&& arg1, _tParam2&& arg2, _tParams&&... args)
			lnoexcept(is_nothrow_constructible<_type, _tParam1&&, _tParam2&&,
				_tParams&&...>())
			: value(lforward(arg1), lforward(arg2), lforward(args)...)
		{}
		//@}
		//! \since build 1.4
		//@{
		boxed_value(const boxed_value&) = default;
		boxed_value(boxed_value&&) = default;

		boxed_value&
			operator=(const boxed_value&) = default;
		boxed_value&
			operator=(boxed_value&&) = default;
		//@}

		operator _type&() lnothrow
		{
			return value;
		}

		operator const _type&() const lnothrow
		{
			return value;
		}
	};

	/*!
	\brief 包装非类类型为类类型。
	*/
	template<typename _type>
	using classify_value_t = cond_t<std::is_class<_type>, _type,
		boxed_value<_type >>;

	/*!
	\brief 类型参数化静态对象。
	\warning 不可重入。
	\warning 非线程安全。
	\since build 1.3
	*/
	template<typename _type, typename, typename...>
	inline _type&
		parameterize_static_object()
	{
		static _type obj;

		return obj;
	}
	/*!
	\brief 非类型参数化静态对象。
	\warning 不可重入。
	\warning 非线程安全。
	\since build 1.3
	*/
	template<typename _type, size_t...>
	inline _type&
		parameterize_static_object()
	{
		static _type obj;

		return obj;
	}


	/*!
	\brief 取类型标识和初始化调用指定的对象。
	\tparam _tKey 起始类型参数化标识。
	\tparam _tKeys 非起始类型参数化标识。
	\tparam _fInit 初始化调用类型。
	\tparam _tParams 初始化参数类型。
	\return 初始化后的对象的左值引用。
	\since build 1.3
	*/
	template<typename _tKey, typename... _tKeys, typename _fInit,
		typename... _tParams>
		inline auto
		get_init(_fInit&& f, _tParams&&... args) -> decltype(f(lforward(args)...))&
	{
		using obj_type = decltype(f(lforward(args)...));

		auto& p(leo::parameterize_static_object<obj_type*, _tKey, _tKeys...>());

		if (!p)
			p = new obj_type(f(lforward(args)...));
		return *p;
	}
	/*!
	\brief 取非类型标识和初始化调用指定的对象。
	\tparam _vKeys 非类型参数化标识。
	\tparam _fInit 初始化调用类型。
	\tparam _tParams 初始化参数类型。
	\return 初始化后的对象的左值引用。
	\since build 1.4
	*/
	template<size_t... _vKeys, typename _fInit, typename... _tParams>
	inline auto
		get_init(_fInit&& f, _tParams&&... args) -> decltype(f(lforward(args)...))&
	{
		using obj_type = decltype(f(lforward(args)...));

		auto& p(leo::parameterize_static_object<obj_type*, _vKeys...>());

		if (!p)
			p = new obj_type(f(lforward(args)...));
		return *p;
	}


	/*!	\defgroup init_mgr Initialization Managers
	\brief 初始化管理器。
	\since build 1.3

	实现保存初始化和反初始化的状态的对象。不直接初始化对象，可以在头文件中直接定义。
	保证初始化满足特定条件。
	*/

	/*!
	\ingroup init_mgr
	\brief 使用引用计数的静态初始化管理器。
	\pre _type 满足 Destructible 。
	\note 当实现支持静态 TLS 时为每线程单例，否则为全局静态单例。
	\warning 对于不支持 TLS 的实现非线程安全。
	\see http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Nifty_Counter 。
	\since build 1.3

	静态初始化，通过引用计数保证所有在定义本类型的对象后已有静态对象被初始化。
	在所有翻译单元的本类型对象析构后自动反初始化。
	*/
	template<class _type>
	class nifty_counter
	{
	public:
		using object_type = _type;

		template<typename... _tParams>
		nifty_counter(_tParams&&... args)
		{
			if (get_count()++ == 0)
				get_object_ptr() = new _type(lforward(args)...);
		}
		//! \since build 1.3
		//@{
		~nifty_counter()
		{
			if (--get_count() == 0)
				delete get_object_ptr();
		}

		static object_type&
			get() lnothrow
		{
			lassume(get_object_ptr());
			return *get_object_ptr();
		}

	private:
		static size_t&
			get_count() lnothrow
		{
			lthread size_t count;

			return count;
		}
		static object_type*&
			get_object_ptr() lnothrow
		{
			lthread object_type* ptr;

			return ptr;
		}

	public:
		static size_t
			use_count() lnothrow
		{
			return get_count();
		}
		//@}
	};


	/*!
	\ingroup init_mgr
	\brief 使用 call_once 的静态初始化管理器。
	\tparam _tOnceFlag 初始化调用标识。
	\note 线程安全取决于 call_once 对 _tOnceFlag 的支持。
	若对于支持 <tt><mutex></tt> 的实现，使用 std::once_flag ，
	对应 std::call_once ，则是线程安全的；
	若使用 bool ，对应 leo::call_once ，不保证线程安全。
	其它类型可使用用户自行定义 call_once 。
	\since build 1.2
	\todo 使用支持 lambda pack 展开的实现构造模板。
	\todo 支持分配器。

	静态初始化，使用 _tOnceFlag 类型的静态对象表示初始化和反初始化状态，
	保证所有在定义本类型的对象后已有静态对象被初始化。
	在所有翻译单元的本类型对象析构后自动反初始化。
	初始化和反初始化调用没有限定符修饰的 call_once 初始化和反初始化。
	用户可以自定义 _tOnceFlag 实际参数对应的 call_once ，但声明
	应与 std::call_once 和 leo::call_once 形式一致。
	*/
	template<typename _type, typename _tOnceFlag>
	class call_once_init :private noncopyable,private nonmovable
	{
	public:
		using object_type = _type;
		using flag_type = _tOnceFlag;
	private:
		flag_type init_flag, uninit_flag;
		replace_storage_t<_type> storage;

		template<typename... _tParams>
		call_once_init(_tParams&&... args)
		{
			call_once(init_flag, std::bind(leo::construct_in<_type,
				_tParams&&...>, std::ref(get()), std::ref(args)...));
		}
		~call_once_init()
		{
			call_once(uninit_flag, std::bind(leo::destruct_in<_type>,
				std::ref(get())));
		}

		//! \since build 1.4
		object_type&
			get() lnothrow
		{
			return storage.template access<object_type>();
		}
		//! \since build 1.4
		const object_type&
			get() const lnothrow
		{
			return storage.template access<const object_type>();
		}
	};

	/*!
	\brief 哨兵对象 sentry。
	\since build 1.01
	*/
	template<typename T>
	class sentry {
		T functor;
	public:
		sentry(T fun) :functor(std::move(fun)) {
		}

		sentry(sentry &&) = default;
		sentry(sentry const &) = delete;

		~sentry() noexcept {
			static_assert(noexcept(functor()),
				"Please check that the finally block cannot throw, "
				"and mark the lambda as noexcept.");
			functor();
		}

	};

	template<typename T>
	sentry<T> finally(T o) {
		return{ std::move(o) };
	}
}

#if !LB_HAS_BUILTIN_NULLPTR
using stdex::nullptr;
#endif

#endif