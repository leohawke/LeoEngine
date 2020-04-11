/*! \file Engine\WIN32\COM.h
\ingroup Engine
\brief COM 接口。
*/
#ifndef LE_WIN32_COM_H
#define LE_WIN32_COM_H 1

#include <LFramework/LCLib/Platform.h>
#include <objbase.h>

namespace platform_ex {

	//! \since build 1.4
	//@{
	//! \brief COM 全局状态。
	class LF_API COM
	{
	protected:
		::HRESULT hResult;

	public:
		COM()
			: hResult(::CoInitialize(nullptr))
		{}
		~COM()
		{
			if (hResult == S_OK)
				::CoUninitialize();
		}

		DefGetter(lnothrow, ::HRESULT, HResult, hResult)
	};

	//! \brief COM 异常基类。
	// \todo fomratmessage
	class LF_API COMException : public std::runtime_error
	{
	protected:
		::HRESULT hResult;

	public:
		COMException(::HRESULT h)
			: runtime_error("COMException"), hResult(h)
		{
			LAssert(FAILED(h) || h == S_FALSE, "Invalid argument found.");
		}

		DefGetter(lnothrow, ::HRESULT, HResult, hResult)
	};

	/*!
	\brief 检查 ::HRESULT 值，若表示失败则抛出 COMException 。
	\return 表示成功的值。
	*/
	inline ::HRESULT
		CheckHResult(::HRESULT h)
	{
		if (FAILED(h))
			throw COMException(h);
		return h;
	}

	/*!
	\brief 检查指针值，若为空则抛出 COMException 。
	*/
	inline PDefH(void, EnsureNonNull, void* p)
		ImplExpr(p ? void() : throw COMException(S_FALSE))


	/*!
	\brief COM 指针。
	\warning 非虚析构。
	*/
	template<class _iCOM>
	class COMPtr
	{
		template<class _iOther>
		friend class COMPtr;

	public:
		using InterfaceType = _iCOM;

	protected:
		InterfaceType* pInterface;

	public:
		COMPtr() lnothrow
			: pInterface()
		{}
		COMPtr(std::nullptr_t) lnothrow
			: pInterface()
		{}
		template<class _iOther>
		COMPtr(_iOther* ptr) lnothrow
			: pInterface(ptr)
		{}
		template<class _iOther>
		COMPtr(_iOther& intf, std::enable_if_t<!std::is_convertible<_iOther&,
			COMPtr&>::value, int> = 0) lnothrow
			: pInterface(&intf)
		{
			pInterface->AddRef();
		}
		COMPtr(const COMPtr& ptr) lnothrow
			: pInterface(ptr.pInterface)
		{
			InternalAddRef();
		}
		template<class _iOther>
		COMPtr(const COMPtr<_iOther>& ptr, std::enable_if_t<
			std::is_convertible<_iOther*, _iCOM*>::value, int> = 0) lnothrow
			: pInterface(ptr.pInterface)
		{
			InternalAddRef();
		}
		COMPtr(COMPtr&& ptr) lnothrow
			: pInterface()
		{
			ptr.swap(*this);
		}
		template<class _iOther>
		COMPtr(COMPtr<_iOther>&& ptr, std::enable_if_t<
			std::is_convertible<_iOther*, _iCOM*>::value, int> = 0) lnothrow
			: pInterface(ptr.pInterface)
		{
			ptr.pInterface = nullptr;
		}
		~COMPtr()
		{
			InternalRelease();
		}

		COMPtr&
			operator=(std::nullptr_t) lnothrow
		{
			InternalRelease();
			return *this;
		}
		COMPtr&
			operator=(_iCOM* p) lnothrow
		{
			if (pInterface != p)
				COMPtr(p).swap(*this);
			return *this;
		}
		COMPtr&
			operator=(const COMPtr& ptr) lnothrow
		{
			COMPtr(ptr).swap(*this);
			return *this;
		}
		COMPtr&
			operator=(COMPtr&& ptr) lnothrow
		{
			ptr.swap(*this);
			return *this;
		}

		_iCOM&
			operator*() const lnothrowv
		{
			return Deref(pInterface);
		}

		_iCOM*
			operator->() const lnothrow
		{
			return pInterface;
		}

		_iCOM**
			operator&() lnothrow
		{
			return &pInterface;
		}

		explicit
			operator bool() const lnothrow
		{
			return Get() != nullptr;
		}

		DefGetter(const lnothrow, _iCOM*, , pInterface)
			DefGetter(const, _iCOM&, Object, EnsureNonNull(pInterface), *pInterface)
			DefGetter(lnothrow, _iCOM*&, Ref, pInterface)

			/*!
			\throw COMException 转换失败。
			*/
			//@{
			COMPtr<IUnknown>
			As(const ::IID& riid) const
		{
			COMPtr<IUnknown> res;

			CheckHResult(Deref(pInterface).QueryInterface(riid,
				leo::replace_cast<void**>(&res.ReleaseAndGetRef())));
			return res;
		}
		template<class _iOther>
		COMPtr<_iOther>
			As() const
		{
			COMPtr<_iOther> res;

			CheckHResult(Deref(pInterface).QueryInterface(__uuidof(_iOther),
				leo::replace_cast<void**>(&res.ReleaseAndGetRef())));
			return res;
		}
		//@}

		::HRESULT
			Cast(const ::IID& riid, COMPtr<IUnknown>& ptr) const lnothrow
		{
			return Deref(pInterface).QueryInterface(riid,
				leo::replace_cast<void**>(&ptr.ReleaseAndGetRef()));
		}
		template<class _iOther>
		::HRESULT
			Cast(COMPtr<_iOther>& ptr) const lnothrow
		{
			LAssertNonnull(pInterface);
			return pInterface->QueryInterface(__uuidof(_iOther),
				leo::replace_cast<void**>(&ptr.ReleaseAndGetRef()));
		}

		InterfaceType*
			Copy() const lnothrow
		{
			InternalAddRef();
			return pInterface;
		}
		void*
			Copy(const ::IID& riid) const
		{
			void* p;

			CheckHResult(Deref(pInterface).QueryInterface(riid, &p));
			return p;
		}

		::HRESULT
			CopyTo(const ::IID& riid, void** ptr) const lnothrow
		{
			return Deref(pInterface).QueryInterface(riid, ptr);
		}
		template<typename _type>
		::HRESULT
			CopyTo(_type*& p) const lnothrow
		{
			return pInterface->QueryInterface(__uuidof(_type),
				leo::replace_cast<void**>(&p));
		}

	protected:
		void
			InternalAddRef() const lnothrow
		{
			if (pInterface)
				pInterface->AddRef();
		}

		void
			InternalRelease() lnothrow
		{
			if (const auto tmp = pInterface)
			{
				pInterface = nullptr;
				tmp->Release();
			}
		}

	public:
		_iCOM*&
			ReleaseAndGetRef() lnothrow
		{
			InternalRelease();
			return pInterface;
		}

		void
			swap(COMPtr& ptr) lnothrow
		{
			std::swap(pInterface, ptr.pInterface);
		}
	};


	//! \relates COMPtr
	//@{
	template<class _iCOM1, class _iCOM2>
	inline bool
		operator==(const COMPtr<_iCOM1>& x, const COMPtr<_iCOM2>& y) lnothrow
	{
		static_assert(leo::or_<std::is_base_of<_iCOM1, _iCOM2>,
			std::is_base_of<_iCOM1, _iCOM2>>(),
			"'_iCOM1' and '_iCOM2' pointers must be comparable");

		return x.Get() == y.Get();
	}
	template<class _iCOM>
	inline bool
		operator==(const COMPtr<_iCOM>& x, std::nullptr_t) lnothrow
	{
		return !x.Get();
	}
	template<class _iCOM>
	inline bool
		operator==(std::nullptr_t, const COMPtr<_iCOM>& x) lnothrow
	{
		return !x.Get();
	}

	template<class _iCOM1, class _iCOM2>
	inline bool
		operator!=(const COMPtr<_iCOM1>& x, const COMPtr<_iCOM2>& y) lnothrow
	{
		return !(x == y);
	}
	template<class _iCOM>
	inline bool
		operator!=(const COMPtr<_iCOM>& x, std::nullptr_t) lnothrow
	{
		return x.Get();
	}

	template<class _iCOM>
	inline bool
		operator!=(std::nullptr_t, const COMPtr<_iCOM>& x) lnothrow
	{
		return x.Get();
	}

	template<class _iCOM1, class _iCOM2>
	inline bool
		operator<(const COMPtr<_iCOM1>& x, const COMPtr<_iCOM2>& y) lnothrow
	{
		static_assert(leo::or_<std::is_base_of<_iCOM1, _iCOM2>,
			std::is_base_of<_iCOM1, _iCOM2>>(),
			"'_iCOM1' and '_iCOM2' pointers must be comparable");

		return x.Get() < y.Get();
	}

	template<class _iCOM>
	void
		Attach(COMPtr<_iCOM>& ptr, typename COMPtr<_iCOM>::InterfaceType* p) lnothrow
	{
		if (const auto p_interface = ptr.Get())
		{
			const auto ref(p_interface->Release());

			lunused(ref);
			lassume(ref != 0 || p_interface != p);
		}
		ptr.GetRef() = ptr;
	}

	template<class _iCOM>
	_iCOM*
		Detach(COMPtr<_iCOM>& ptr) lnothrow
	{
		auto p(ptr.Get());

		ptr.GetRef() = nullptr;
		return p;
	}

	template<class _iCOM>
	unsigned long
		Reset(COMPtr<_iCOM>& ptr) lnothrow
	{
		auto n(0UL);

		if (const auto tmp = ptr.Get())
		{
			ptr.GetRef() = nullptr;
			n = tmp->Release();
		}
		return n;
	}

	template<class _iCOM>
	inline DefSwap(lnothrow, COMPtr<_iCOM>)

#ifdef LB_IMPL_MSCPP
#define COMPtr_RefParam(p,guid) __uuidof(decltype(*p)),leo::replace_cast<void**>(&p.ReleaseAndGetRef())
#else
#define COMPtr_RefParam(p,guid) guid,leo::replace_cast<void**>(&p.ReleaseAndGetRef())
#endif
	//@}
	//@}
}

#endif