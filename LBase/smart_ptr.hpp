#pragma once

#include "ldef.h"
#include <memory>
#include <type_traits>


namespace leo
{
	template<typename _type>
	lconstfn _type*
		//取内建指针
		get_raw(_type* const& p) lnothrow
	{
		return p;
	}

	template<typename _type>
	lconstfn auto
		get_raw(const std::unique_ptr<_type>& p) lnothrow -> decltype(p.get())
	{
		return p.get();
	}
	template<typename _type>
	lconstfn _type*
		get_raw(const std::shared_ptr<_type>& p) lnothrow
	{
		return p.get();
	}
	template<typename _type>
	lconstfn _type*
		get_raw(const std::weak_ptr<_type>& p) lnothrow
	{
		return p.lock().get();
	}


	/*!	\defgroup owns_any Owns Any Check
	\brief 检查是否是所有者。
	*/
	//@{
	template<typename _type>
	lconstfn bool
		owns_any(_type* const&) lnothrow
	{
		return {};
	}
	template<typename _type, typename _fDeleter>
	lconstfn bool
		owns_any(const std::unique_ptr<_type, _fDeleter>& p) lnothrow
	{
		return bool(p);
	}
	template<typename _type>
	lconstfn bool
		owns_any(const std::shared_ptr<_type>& p) lnothrow
	{
		return p.use_count() > 0;
	}
	template<typename _type>
	lconstfn bool
		owns_any(const std::weak_ptr<_type>& p) lnothrow
	{
		return !p.expired();
	}
	//@}

	/*!	\defgroup owns_nonnull Owns Nonnull Check
	\brief 检查是否是非空对象的所有者。
	*/
	//@{
	template<typename _type>
	lconstfn bool
		owns_nonnull(_type* const&) lnothrow
	{
		return {};
	}
	//! \since build 550
	template<typename _type, typename _fDeleter>
	lconstfn bool
		owns_nonnull(const std::unique_ptr<_type, _fDeleter>& p) lnothrow
	{
		return bool(p);
	}
	template<typename _type>
	lconstfn bool
		owns_nonnull(const std::shared_ptr<_type>& p) lnothrow
	{
		return bool(p);
	}
	template<typename _type>
	lconstfn bool
		owns_nonnull(const std::weak_ptr<_type>& p) lnothrow
	{
		return bool(p.lock());
	}
	//@}

	/*!	\defgroup owns_unique Owns Uniquely Check
	\brief 检查是否是唯一所有者。
	*/
	//@{
	template<typename _type>
	lconstfn bool
		owns_unique(const _type&) lnothrow
	{
		return !is_reference_wrapper<_type>();
	}
	template<typename _type, class _tDeleter>
	inline bool
		owns_unique(const std::unique_ptr<_type, _tDeleter>& p) lnothrow
	{
		return bool(p);
	}
	template<typename _type>
	inline bool
		owns_unique(const std::shared_ptr<_type>& p) lnothrow
	{
		return p.unique();
	}

	template<typename _type>
	lconstfn bool
		owns_unique_nonnull(const _type&) lnothrow
	{
		return !is_reference_wrapper<_type>();
	}
	//! \pre 参数非空。
	//@{
	template<typename _type, class _tDeleter>
	inline bool
		owns_unique_nonnull(const std::unique_ptr<_type, _tDeleter>& p) lnothrow
	{
		lconstraint(p);
		return true;
	}
	template<typename _type>
	inline bool
		owns_unique_nonnull(const std::shared_ptr<_type>& p) lnothrow
	{
		lconstraint(p);
		return p.unique();
	}
	//@}

	template<typename _type, class _tDeleter>
	inline bool
		reset(std::unique_ptr<_type, _tDeleter>& p) lnothrow
	{
		if (p.get())
		{
			p.reset();
			return true;
		}
		return false;
	}
	template<typename _type>
	inline bool
		reset(std::shared_ptr<_type>& p) lnothrow
	{
		if (p.get())
		{
			p.reset();
			return true;
		}
		return false;
	}

	template<typename _type, typename _pSrc>
	lconstfn std::unique_ptr<_type>
		//_pSrc是内建指针
		unique_raw(_pSrc* p) lnothrow
	{
		return std::unique_ptr<_type>(p);
	}
	template<typename _type>
	lconstfn std::unique_ptr<_type>
		unique_raw(_type* p) lnothrow
	{
		return std::unique_ptr<_type>(p);
	}
	template<typename _type, typename _tDeleter, typename _pSrc>
	lconstfn std::unique_ptr<_type, _tDeleter>
		unique_raw(_pSrc* p, _tDeleter&& d) lnothrow
	{
		return std::unique_ptr<_type, _tDeleter>(p, lforward(d));
	}

	template<typename _type, typename _tDeleter>
	lconstfn std::unique_ptr<_type, _tDeleter>
		unique_raw(_type* p, _tDeleter&& d) lnothrow
	{
		return std::unique_ptr<_type, _tDeleter>(p, lforward(d));
	}

	template<typename _type>
	lconstfn std::unique_ptr<_type>
		unique_raw(nullptr_t) lnothrow
	{
		return std::unique_ptr<_type>();
	}


	template<typename _type, typename... _tParams>
	lconstfn std::shared_ptr<_type>
		share_raw(_type* p, _tParams&&... args)
	{
		return std::shared_ptr<_type>(p, lforward(args)...);
	}

	template<typename _type, typename _pSrc, typename... _tParams>
	lconstfn std::shared_ptr<_type>
		share_raw(_pSrc&& p, _tParams&&... args)
	{
		static_assert(is_pointer<remove_reference_t<_pSrc>>(),
			"Invalid type found.");

		return std::shared_ptr<_type>(p, lforward(args)...);
	}
	template<typename _type>
	lconstfn std::shared_ptr<_type>
		share_raw(nullptr_t) lnothrow
	{
		return std::shared_ptr<_type>();
	}
	/*!
	\note 使用空指针和其它参数构造空对象。
	\pre 参数复制构造不抛出异常。
	*/
	template<typename _type, class... _tParams>
	lconstfn limpl(std::enable_if_t) < sizeof...(_tParams) != 0, std::shared_ptr<_type> >
		share_raw(nullptr_t, _tParams&&... args) lnothrow
	{
		return std::shared_ptr<_type>(nullptr, lforward(args)...);
	}

	/*!
	\note 使用默认初始化。
	\see WG21 N3588 A4 。
	\since build 1.4
	*/
	//@{
	template<typename _type, typename... _tParams>
	lconstfn limpl(std::enable_if_t<!std::is_array<_type>::value, std::unique_ptr<_type>>)
		make_unique_default_init()
	{
		return std::unique_ptr<_type>(new _type);
	}
	template<typename _type, typename... _tParams>
	lconstfn limpl(std::enable_if_t<std::is_array<_type>::value&& std::extent<_type>::value == 0,
		std::unique_ptr<_type>>)
		make_unique_default_init(size_t size)
	{
		return std::unique_ptr<_type>(new std::remove_extent_t<_type>[size]);
	}
	template<typename _type, typename... _tParams>
	limpl(std::enable_if_t<std::extent<_type>::value != 0>)
		make_unique_default_init(_tParams&&...) = delete;
	//@}


	//@}
}