/*! \file sceope_gurad.hpp
\ingroup LBase
\brief 作用域守护。

提供作用域守护的便利接口。
*/

#ifndef LBase_sceope_gurad_hpp
#define LBase_sceope_gurad_hpp 1


#include "LBase/type_traits.hpp" // for is_constructible, is_reference,
//	is_nothrow_swappable, std::swap, std::declval, is_nothrow_copyable;
#include "LBase/sutility.h" // for noncopyable;
#include "LBase/memory.hpp" // for leo::construct_in, leo::destruct_in;
#include "LBase/functional.hpp" //for leo::one_shot

namespace leo
{
	//! \since build 1.3
	//@{
	/*!
	\brief 作用域守护：析构时调用保存的函数对象或引用。
	\note 不可复制，不提供其它状态。
	*/
	template<typename _func, bool _bNoThrow = true>
	struct guard
	{
		_func func;

		//! \since build 1.4
		template<typename... _tParams>
		guard(_tParams&&... args)
			lnoexcept(is_constructible<_func, _tParams&&...>::value)
			: func(lforward(args)...)
		{}
		guard(guard&&) = default;
		~guard() lnoexcept_spec(!_bNoThrow)
		{
			func();
		}

		guard&
			operator=(guard&&) = default;
	};

	/*!
	\brief 创建作用域守护。
	\relates guard
	\since build 1.4
	*/
	//! \since build 1.4
	template<typename _type, bool _bNoThrow = true, typename... _tParams>
	guard<_type>
		make_guard(_tParams&&... args) lnoexcept_spec(guard<_type, _bNoThrow>(
			guard<_type, _bNoThrow>(lforward(args)...)))
	{
		return guard<_type, _bNoThrow>(lforward(args)...);
	}
	template<bool _bNoThrow = true, typename _type>
	guard<_type>
		make_guard(_type f)
		lnoexcept_spec(guard<_type, _bNoThrow>(guard<_type, _bNoThrow>(f)))
	{
		return guard<_type, _bNoThrow>(f);
	}

	//! \since build 1.3
	template<bool _bNoThrow = true, typename _type>
	guard<one_shot<_type>>
		unique_guard(_type f) lnoexcept_spec(
			guard<one_shot<_type>, _bNoThrow>(guard<one_shot<_type>, _bNoThrow>(f)))
	{
		return guard<one_shot<_type>, _bNoThrow>(f, true);
	}
	//@}


	/*!
	\brief 创建共享作用域守护。
	\since build 1.3
	*/
	template<typename _func, typename _type = void>
	inline std::shared_ptr<_type>
		share_guard(_func f, _type* p = {})
	{
		return leo::share_raw(p, f);
	}


	/*!
	\brief 解除操作。
	\since build 1.4
	*/
	//@{
	//! \brief 使用成员 \c dismiss 。
	template<class _type>
	lconstfn auto
		dismiss(_type& gd) -> decltype(gd.dismiss())
	{
		return gd.dismiss();
	}
	template<typename _func, typename _tRes, typename _tState>
	lconstfn void
		dismiss(const one_shot<_func, _tRes, _tState>& gd)
	{
		gd.fresh = {};
	}
	//! \brief 使用 ADL \c dismiss 。
	template<typename _func, bool _bNoThrow>
	lconstfn auto
		dismiss(guard<_func, _bNoThrow>& gd) -> decltype(dismiss(gd.func))
	{
		return dismiss(gd.func);
	}
	//@}


	namespace details
	{
		//! \since build 1.4
		//@{
		template<typename _type, typename _tToken,
			bool _bRef = is_reference<_tToken>::value>
			struct state_guard_traits
		{
			//! \since build 1.3
			//@{
			static void
				save(_tToken t, _type& val) lnoexcept_spec(t(true, val))
			{
				t(true, val);
			}

			static void
				restore(_tToken t, _type& val) lnoexcept_spec(t(false, val))
			{
				t(false, val);
			}
			//@}
		};

		template<typename _type, typename _tToken>
		struct state_guard_traits<_type, _tToken, true>
		{
			//! \since build 1.3
			//@{
			static void
				save(_tToken t, _type& val) lnoexcept(is_nothrow_swappable<_type>())
			{
				using std::swap;

				swap(val, static_cast<_type&>(t));
			}

			static void
				restore(_tToken t, _type& val) lnoexcept(is_nothrow_swappable<_type>())
			{
				using std::swap;

				swap(val, static_cast<_type&>(t));
			}
			//@}
		};


		template<typename _type, typename _tToken>
		struct state_guard_impl
			: private state_guard_traits<_type, _tToken>, private noncopyable
		{
			using value_type = _type;
			using token_type = _tToken;

			token_type token;
			union
			{
				value_type value;
			};

			state_guard_impl(token_type t)
				: token(t)
			{}
			~state_guard_impl()
			{}

			//! \since build 1.3
			template<typename... _tParams>
			void
				construct_and_save(_tParams&&... args)
				lnoexcept(noexcept(value_type(lforward(args)...))
					&& noexcept(std::declval<state_guard_impl&>().save()))
			{
				leo::construct_in(value, lforward(args)...);
				save();
			}

			//! \since build 1.3
			void
				destroy() lnoexcept(is_nothrow_destructible<value_type>())
			{
				leo::destruct_in(value);
			}

			void
				save() lnoexcept(noexcept(state_guard_traits<value_type, token_type>
					::save(std::declval<token_type&>(), std::declval<value_type&>())))
			{
				state_guard_traits<value_type, token_type>::save(token, value);
			}

			void
				restore()
				lnoexcept_spec(state_guard_traits<value_type, token_type>::restore(
					std::declval<token_type&>(), std::declval<value_type&>()))
			{
				state_guard_traits<value_type, token_type>::restore(token, value);
			}

			void
				restore_and_destroy()
				lnoexcept_spec(state_guard_traits<value_type, token_type>::restore(
					std::declval<token_type&>(), std::declval<value_type&>()))
			{
				restore();
				destroy();
			}
		};
		//@}

	} // namespace details;

	  /*!
	  \brief 使用临时状态暂存对象的作用域守护。
	  \since build 1.3
	  \todo 支持分配器。
	  \todo 支持有限的复制和转移。
	  */
	  //@{
	template<typename _type, typename _tCond = bool,
		typename _tToken = std::function<void(bool, _type&)>>
		class state_guard : private details::state_guard_impl<_type, _tToken>
	{
		static_assert(is_nothrow_copy_constructible<_tCond>::value,
			"Invalid condition type found.");

	private:
		using base = details::state_guard_impl<_type, _tToken>;

	public:
		using typename base::value_type;
		using typename base::token_type;
		using condition_type = _tCond;

		using base::token;
		using base::value;

	private:
		//! \since build 1.4
		condition_type enabled{};

	public:
		//! \since build 1.4
		//@{
		template<typename... _tParams>
		state_guard(condition_type cond, token_type t, _tParams&&... args)
			lnoexcept(and_<is_nothrow_constructible<base, token_type>,
				is_nothrow_copy_constructible<condition_type>>()
				&& noexcept(std::declval<state_guard&>()
					.base::construct_and_save(lforward(args)...)))
			: base(t),
			enabled(cond)
		{
			if (enabled)
				base::construct_and_save(lforward(args)...);
		}
		~state_guard() lnoexcept(is_nothrow_copy_constructible<condition_type>()
			&& noexcept(std::declval<state_guard&>().base::restore_and_destroy()))
		{
			if (enabled)
				base::restore_and_destroy();
		}
		//@}

		//! \since build 1.3
		void
			dismiss() lnoexcept(is_nothrow_copyable<condition_type>())
		{
			if (enabled)
				base::destroy();
			enabled = condition_type();
		}

		//! \since build 1.4
		condition_type
			engaged() const lnothrow
		{
			return engaged;
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

		template<typename... _tParams>
		state_guard(token_type t, _tParams&&... args) lnoexcept(
			is_nothrow_constructible<base, token_type>() && noexcept(std::declval<
				state_guard&>().base::construct_and_save(lforward(args)...)))
			: base(t)
		{
			base::construct_and_save(lforward(args)...);
		}
		//! \since build 1.4
		~state_guard()
			lnoexcept_spec(std::declval<state_guard&>().base::restore_and_destroy())
		{
			base::restore_and_destroy();
		}
	};
	//@}


	/*!
	\brief 使用 ADL swap 调用暂存对象的作用域守护。
	\since build 1.3
	\todo 支持分配器。
	\todo 支持有限的复制和转移。
	*/
	template<typename _type, typename _tCond = bool, typename _tReference = _type&>
	using swap_guard = state_guard<_type, _tCond, _tReference>;

} // namespace leo;


#endif
