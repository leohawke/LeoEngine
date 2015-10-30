////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/utility.hpp
//  Version:     v1.01
//  Created:     ?/?/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 
// -------------------------------------------------------------------------
//  History:
//		2014-3-9 14:54 加入新的工具类{临时状态暂存对象的 scope guard}
//
////////////////////////////////////////////////////////////////////////////
#ifndef IndePlatform_utility_hpp
#define IndePlatform_utility_hpp

#include "type_op.hpp" //leo::qualified_decay,assert
#include "Abstract.hpp"
#include <utility>
#include <stdexcept> //std::logic_error

namespace leo
{
	//不满足此断言的行为是接口明确地未定义的，行为不可预测
#define lconstraint assert
//运行时检查的环境条件约束断言。用于明确地非 yconstraint 适用的情形
#define lassume assert

	class LB_API unsupported : public std::logic_error
	{
	public:
		unsupported()
			:logic_error("Unsupported operation found.")
		{}
		template<typename type>
		unsupported(type && arg)
			: logic_error(lforward(arg))
		{}
	};

	class LB_API unimplemented : public unsupported
	{
	public:
		unimplemented()
			: unsupported("Unimplemented operation found.")
		{}
		template<typename _type>
		unimplemented(_type&& arg)
			: unsupported(lforward(arg))
		{}
	};

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
#if LB_IMPL_MSCPP
		cloneable() = default;
		cloneable(const cloneable&) = default;
		cloneable(cloneable&&)
		{}
#endif
		virtual cloneable*
			clone() const = 0;

		virtual
			~cloneable()
		{}
	};


	/*
	\see ISO WG21/N3797 20.2.3[utility.exchange] 。
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


	/*!
	\see ISO C++11 30.2.6[thread.decaycopy] 。
	\see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2011/n3255.html 。
	*/
	template<typename _type>
	decay_t<_type>
		decay_copy(_type&& arg)
	{
		return std::forward<_type>(arg);
	}

	/*!
	\brief 退化传递。
	\note 类似 decay_copy ，但仅对函数或数组及其引用类型退化，不复制其它值。
	*/
	template<typename _type>
	typename qualified_decay<_type>::type
		decay_forward(_type&& arg)
	{
		return std::forward<_type>(arg);
	}


	/*!
	\brief 计算指定数组类型对象的长度。
	*/
	template<typename _type, size_t _vN>
	lconstfn size_t
		arrlen(_type(&)[_vN])
	{
		return _vN;
	}
	template<typename _type, size_t _vN>
	lconstfn size_t
		arrlen(_type(&&)[_vN])
	{
		return _vN;
	}


	/*!
	\brief 包装类类型的值的对象。
	\warning 非虚析构。
	*/
	template<typename _type>
	struct boxed_value
	{
		_type value;

		template<typename... _tParams>
		lconstfn
			boxed_value(_tParams&&... args)
			: value(lforward(args)...)
		{}

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
	using classify_value_t = conditional_t < std::is_class<_type>::value, _type,
		boxed_value < _type >> ;


	/*!
	\brief 按标识调用函数，保证调用一次。
	\note 类似 std::call_once ，但不保证线程安全性。
	\note ISO C++11（至 N3691 ） 30.4 synopsis 处的声明存在错误。
	\see https://github.com/cplusplus/draft/issues/151 。

	当标识为 true 时候无作用，否则调用函数。
	*/
	template<typename _fCallable, typename... _tParams>
	inline void
		call_once(bool& b, _fCallable&& f, _tParams&&... args)
	{
		if (!b)
		{
			f(lforward(args)...);
			b = true;
		}
	}


	/*!
	\brief 类型参数化静态对象。
	\warning 不可重入。
	\warning 非线程安全。
	\since build 303
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
	\since build 301
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
	\since build 327
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
	\since build 327
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
	\since build 328

	实现保存初始化和反初始化的状态的对象。不直接初始化对象，可以在头文件中直接定义。
	保证初始化满足特定条件。
	*/

	/*!
	\ingroup init_mgr
	\brief 使用引用计数的静态初始化管理器。
	\pre _type 满足 Destructible 。
	\note 当实现支持静态 TLS 时为每线程单例，否则为全局静态单例。
	\warning 对于不支持 TLS 的实现非线程安全。
	\sa ythread
	\see http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Nifty_Counter 。
	\since build 425

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
		//! \since build 425
		//@{
		//! \since build 461
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
	\since build 1.02
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
	class call_once_init
	{
	public:
		using object_type = _type;
		using flag_type = _tOnceFlag;

		template<typename... _tParams>
		call_once_init(_tParams&&... args)
		{
			call_once(get_init_flag(), init<_tParams...>, lforward(args)...);
		}
		~call_once_init()
		{
			call_once(get_uninit_flag(), uninit);
		}

		static object_type&
			get()
		{
			lassume(get_object_ptr());
			return *get_object_ptr();
		}

	private:
		static flag_type&
			get_init_flag()
		{
			static flag_type flag;

			return flag;
		}

		static object_type*&
			get_object_ptr()
		{
			static object_type* ptr;

			return ptr;
		}

		static flag_type&
			get_uninit_flag()
		{
			static flag_type flag;

			return flag;
		}

		template<typename... _tParams>
		static void
			init(_tParams&&... args)
		{
			get_object_ptr() = new object_type(lforward(args)...);
		}

		static void
			uninit()
		{
			delete get_object_ptr();
		}
	};

	namespace details
	{

		//! \since build 1.01
		//@{
		template<typename _type, typename _tToken,
			bool _bRef = is_reference<_tToken>::value>
		struct state_guard_traits
		{
			static void
				save(_tToken t, _type& val) lnoexcept(
				lnoexcept(std::declval<_tToken&>()(true, std::declval<_type&>())))
			{
				t(true, val);
			}

			static void
				restore(_tToken t, _type& val) lnoexcept(
				lnoexcept(std::declval<_tToken&>()(false, std::declval<_type&>())))
			{
				t(false, val);
			}
		};

		template<typename _type, typename _tToken>
		struct state_guard_traits<_type, _tToken, true>
		{
			//! \todo 按 ISO C++ [utility.swap] 要求确定异常规范。
			static void
				save(_tToken t, _type& val)
			{
				using std::swap;

				swap(val, static_cast<_type&>(t));
			}

			//! \todo 按 ISO C++ [utility.swap] 要求确定异常规范。
			static void
				restore(_tToken t, _type& val)
			{
				using std::swap;

				swap(val, static_cast<_type&>(t));
			}
		};


		template<typename _type, typename _tToken>
		struct state_guard_impl : private state_guard_traits<_type, _tToken>
		{
			using value_type = _type;
			using token_type = _tToken;

			token_type token;
			union
			{
				value_type value;
				stdex::byte data[sizeof(value_type)];
			};

			state_guard_impl(token_type t)
				: token(t)
			{}
			state_guard_impl(const state_guard_impl&) = delete;
			~state_guard_impl()
			{}

			template<typename... _tParams>
			void
				construct_and_save(_tParams&&... args)
			{
				new(std::addressof(value)) value_type(lforward(args)...);
				save();
			}

			void
				destroy()
			{
				value.~value_type();
			}

			void
				save() lnoexcept(noexcept(state_guard_traits<value_type, token_type>
				::save(std::declval<token_type&>(), std::declval<value_type&>())))
			{
				state_guard_traits<value_type, token_type>::save(token, value);
			}

			void
				restore() lnoexcept(noexcept(state_guard_traits<value_type, token_type>
				::restore(std::declval<token_type&>(),
				std::declval<value_type&>())))
			{
				state_guard_traits<value_type, token_type>::restore(token, value);
			}

			void
				restore_and_destroy() lnoexcept(
				noexcept(state_guard_traits<value_type, token_type>::restore(
				std::declval<token_type&>(), std::declval<value_type&>())))
			{
				restore();
				destroy();
			}
		};
		//@}

	} // namespace details;

	 /*!
	 \brief 使用临时状态暂存对象的 scope guard 。
	 \since build 1.01
	 \todo 支持分配器。
	 \todo 支持有限的复制和转移。
	 */
	 //@{
	template<typename _type, typename _tCond = bool,
		typename _tToken = std::function<void(bool, _type&) >>
	class state_guard : private details::state_guard_impl<_type, _tToken>
	{
	private:
		using base = details::state_guard_impl<_type, _tToken>;

	public:
		using typename base::value_type;
		using typename base::token_type;
		using condition_type = _tCond;

		using base::token;
		using base::value;
		using base::data;
		mutable condition_type enabled{};

		template<typename... _tParams>
		state_guard(condition_type cond, token_type t, _tParams&&... args)
			: base(t),
			enabled(cond)
		{
			if (enabled)
				base::construct_and_save(lforward(args)...);
		}
		~state_guard() lnoexcept(
			noexcept(std::declval<state_guard&>().base::restore_and_destroy()))
		{
			if (enabled)
				base::restore_and_destroy();
		}

		void
			dismiss()
		{
			if (enabled)
				base::destroy();
			enabled = condition_type();
		}
	};

	template<typename _type, typename _tToken>
	class state_guard<_type, void, _tToken>
		: private details::state_guard_impl<_type, _tToken>
	{
	private:
		using base = details::state_guard_impl<_type, _tToken>;

	public:
		using typename base::value_type;
		using typename base::token_type;
		using condition_type = void;

		using base::token;
		using base::value;
		using base::data;

		template<typename... _tParams>
		state_guard(token_type t, _tParams&&... args)
			: base(t)
		{
			base::construct_and_save(lforward(args)...);
		}
		~state_guard()
		{
			base::restore_and_destroy();
		}
	};
	//@}

	/*!
	\brief 使用 ADL swap 调用暂存对象的 scope guard 。
	\since build 1.01
	\todo 支持分配器。
	\todo 支持有限的复制和转移。
	\todo 等待帝球完成
	*/
	template<typename _type, typename _tCond = bool, typename _tRef = _type&>
	using swap_guard = state_guard<_type, _tCond, _tRef>;

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

		sentry(sentry &&) = delete;
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
		return {std::move(o)};
	}
}


#include <unordered_map>

namespace leo {
	namespace details {
		static class StringTableDelegate {
		public:
			~StringTableDelegate() {
				for (auto &str : mMap)
				{
					free(str.second);
				}
			}
			inline std::size_t hash(const wchar_t* str) {
#ifdef VC_STL
				auto sid = std::_Hash_seq((unsigned char*)str, wcslen(str)*sizeof(wchar_t) / sizeof(char));
#else
				auto sid = std::_Fnv_hash_impl::hash((void*)str, wcslen(str)*sizeof(wchar_t) / sizeof(char));
#endif
				if (mMap.find(sid) == mMap.end())
					mMap[sid] = _wcsdup(str);
				return sid;
			}
			inline std::size_t hash(const std::wstring& str) {
				return hash(str.c_str());
			}

			const wchar_t* unhash(std::size_t sid) {
				auto it = mMap.find(sid);

				if (it == mMap.end())
					return nullptr;

				return mMap[sid];
			}
		private:
			std::unordered_map<std::size_t, wchar_t*> mMap;
		} mTable;
	}

	inline std::size_t hash(const wchar_t* str) {
		return details::mTable.hash(str);
	}

	inline std::size_t hash(const std::wstring& str) {
		return details::mTable.hash(str);
	}

	inline const wchar_t* unhash(std::size_t sid) noexcept(false){
		if (auto str = details::mTable.unhash(sid))
			return str;
		else
			throw std::invalid_argument("该资源不存在");
	}
}
#if !LB_HAS_BUILTIN_NULLPTR
using stdex::nullptr;
#endif

#endif