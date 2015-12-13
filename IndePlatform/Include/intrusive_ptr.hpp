////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/intrusive_ptr.hpp
//  Version:     v1.00
//  Created:     12/13/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: «÷»Î Ω÷∏’Î
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////

#ifndef IndePlatform_intrusive_ptr_hpp
#define IndePlatform_intrusive_ptr_hpp

#include <cassert> // assert 
#include <cstddef> // nullptr_t, size_t, ptrdiff_t 
#include <atomic> // atomic<ptrdiff_t>, atomic<bool>, atomic<T *> 
#include <memory> // default_delete 
#include <utility> // forward, declval 
#include <type_traits> 
#include <functional> // rational functors 
#include <exception> // terminate 

namespace std {

	template<typename _T, typename _D = default_delete<_T>>
	class intrusive_base;

	template<typename _T, typename _D = default_delete<_T>>
	class intrusive_ptr;
	template<typename _T, typename _D = default_delete<_T>>
	class intrusive_weak_ptr;

	namespace _Intrusive_ptr_impl {
		template<typename _Dst, typename _Src, typename = _Dst>
		struct _Static_cast_or_dynamic_cast {
			_Dst operator()(_Src && __src) const {
				return dynamic_cast<_Dst>(forward<_Src>(__src));
			}
		};
		template<typename _Dst, typename _Src>
		struct _Static_cast_or_dynamic_cast<_Dst, _Src, decltype(static_cast<_Dst>(declval<_Src>()))> {
			constexpr _Dst operator()(_Src && __src) const noexcept {
				return static_cast<_Dst>(forward<_Src>(__src));
			}
		};

		template<typename _Dst, typename _Src>
		_Dst __static_cast_or_dynamic_cast(_Src && __src) {
			return _Static_cast_or_dynamic_cast<_Dst, _Src>()(forward<_Src>(__src));
		}

		class _Ref_count_base {
			mutable atomic<ptrdiff_t> __ref;

		protected:
			_Ref_count_base() noexcept {
				__ref.store(1, memory_order_relaxed);
			}
			_Ref_count_base(const _Ref_count_base &) noexcept
				: _Ref_count_base()
			{
			}
			_Ref_count_base & operator=(const _Ref_count_base &) noexcept {
				return *this;
			}

		public:
			ptrdiff_t __get_ref() const volatile noexcept {
				return __ref.load(memory_order_relaxed);
			}
			bool __try_add_ref() const volatile noexcept {
				auto __old = __ref.load(memory_order_relaxed);
				for (;;) {
					assert(__old >= 0);

					if (__old == 0) {
						return false;
					}
					if (__ref.compare_exchange_strong(__old, __old + 1, memory_order_relaxed)) {
						return true;
					}
				}
			}
			void __add_ref() const volatile noexcept {
				const auto __old = __ref.fetch_add(1, memory_order_relaxed);
				assert(__old > 0);
				(void)__old;
			}
			bool __drop_ref() const volatile noexcept {
				const auto __old = __ref.fetch_sub(1, memory_order_relaxed);
				assert(__old > 0);
				return __old == 1;
			}
		};

		class _Spin_lock {
			atomic<bool> __b;

		public:
			_Spin_lock() noexcept {
				__b.store(false, memory_order_seq_cst);
			}

			void __lock() noexcept {
				while (__b.exchange(true, memory_order_seq_cst)) {
#if defined(__x86_64__) || defined(__i386__) 
					__builtin_ia32_pause(); // FIXME: add pause implementation for other platforms. 
#endif 
				}
			}
			void __unlock() noexcept {
				__b.store(false, memory_order_seq_cst);
			}
		};

		template<typename _D>
		class _Deleteable : public _Ref_count_base {
			template<typename, class>
			friend class intrusive_ptr;
			template<typename, class>
			friend class intrusive_weak_ptr;

			class _Weak_observer : public _Ref_count_base {
				mutable _Spin_lock __parent_lock;
				_Deleteable * __parent;

			public:
				explicit _Weak_observer(_Deleteable * __rhs) noexcept
					: __parent(__rhs)
				{
				}

				bool __expired() const noexcept {
					__parent_lock.__lock();
					auto __test_parent = __parent;
					if (__test_parent) {
						if (static_cast<const volatile _Ref_count_base *>(__test_parent)->__get_ref() == 0) {
							__test_parent = nullptr;
						}
					}
					__parent_lock.__unlock();
					return __test_parent == nullptr;
				}
				void __clear() noexcept {
					__parent_lock.__lock();
					__parent = nullptr;
					__parent_lock.__unlock();
				}

				template<typename _T>
				intrusive_ptr<_T, _D> __get() const noexcept;
			};

			mutable atomic<_Weak_observer *> __observer;

			_Weak_observer * __create_observer() const volatile {
				auto __test_observer = __observer.load(memory_order_consume);
				if (!__test_observer) {
					auto __new_observer = new _Weak_observer(const_cast<_Deleteable *>(this));
					if (__observer.compare_exchange_strong(__test_observer, __new_observer, memory_order_acq_rel)) {
						__test_observer = __new_observer;
					}
					else {
						// __test_observer now holds the old value of __observer, which is nonnull. 
						delete __new_observer;
					}
				}
				return __test_observer;
			}

		public:
			_Deleteable() noexcept {
				__observer.store(nullptr, memory_order_release);
			}
			_Deleteable(const _Deleteable &) noexcept
				: _Deleteable()
			{
			}
			_Deleteable & operator=(const _Deleteable &) noexcept {
				return *this;
			}
			~_Deleteable() {
				auto __test_observer = __observer.load(memory_order_consume);
				if (__test_observer) {
					bool __free_it = static_cast<const volatile _Ref_count_base *>(__test_observer)->__drop_ref();
					if (__free_it) {
						delete __test_observer;
					}
					else {
						__test_observer->__clear();
					}
				}
			}
		};
	}

	template<typename _T, class _D>
	class intrusive_base : public _Intrusive_ptr_impl::_Deleteable<_D> {
		static_assert(!is_array<_T>::value, "intrusive_base doesn't accept arrays.");

		template<typename, class>
		friend class intrusive_ptr;
		template<typename, class>
		friend class intrusive_weak_ptr;

		template<typename _Cv_other, typename _Cv_this>
		intrusive_ptr<_Cv_other, _D> __fork_shared(_Cv_this * __this) noexcept;
		template<typename _Cv_other, typename _Cv_this>
		intrusive_weak_ptr<_Cv_other, _D> __fork_weak(_Cv_this * __this);

	protected:
		~intrusive_base() {
			if (use_count() > 1) {
				terminate();
			}
		}

	public:
		size_t use_count() const volatile noexcept {
			return static_cast<size_t>(static_cast<const volatile _Intrusive_ptr_impl::_Ref_count_base *>(this)->__get_ref());
		}

		template<typename _Other = _T>
		intrusive_ptr<const volatile _Other, _D> shared_from_this() const volatile noexcept {
			return __fork_shared<const volatile _Other>(this);
		}
		template<typename _Other = _T>
		intrusive_ptr<const _Other, _D> shared_from_this() const noexcept {
			return __fork_shared<const _Other>(this);
		}
		template<typename _Other = _T>
		intrusive_ptr<volatile _Other, _D> shared_from_this() volatile noexcept {
			return __fork_shared<volatile _Other>(this);
		}
		template<typename _Other = _T>
		intrusive_ptr<_Other, _D> shared_from_this() noexcept {
			return __fork_shared<_Other>(this);
		}

		template<typename _Other = _T>
		intrusive_weak_ptr<const volatile _Other, _D> weak_from_this() const volatile {
			return __fork_weak<const volatile _Other>(this);
		}
		template<typename _Other = _T>
		intrusive_weak_ptr<const _Other, _D> weak_from_this() const {
			return __fork_weak<const _Other>(this);
		}
		template<typename _Other = _T>
		intrusive_weak_ptr<volatile _Other, _D> weak_from_this() volatile {
			return __fork_weak<volatile _Other>(this);
		}
		template<typename _Other = _T>
		intrusive_weak_ptr<_Other, _D> weak_from_this() {
			return __fork_weak<_Other>(this);
		}
	};

	template<typename _T, class _D>
	class intrusive_ptr {
		static_assert(sizeof(intrusive_base<_T, _D>), "intrusive_base<_T, _D> is not an object type or is an incomplete type.");
		static_assert(sizeof(dynamic_cast<const volatile intrusive_base<_T, _D> *>(declval<_T *>())), "Unable to locate intrusive_base for the managed object type.");

		_T * __ptr;

	public:
		using element_type = _T;
		using deleter_type = _D;

		constexpr intrusive_ptr(nullptr_t = nullptr) noexcept
			: __ptr(nullptr)
		{
		}
		explicit intrusive_ptr(element_type * __rhs) noexcept
			: __ptr(__rhs)
		{
		}
		template<typename _Other, typename _OtherDeleter,
			typename enable_if<
			is_convertible<_Other *, element_type *>::value &&
			is_convertible<_OtherDeleter, deleter_type>::value,
			int>::type = 0>
			intrusive_ptr(const intrusive_ptr<_Other, _OtherDeleter> & __rhs) noexcept
			: __ptr(__rhs.__ptr)
		{
			if (__ptr) {
				static_cast<const volatile _Intrusive_ptr_impl::_Ref_count_base *>(__ptr)->__add_ref();
			}
		}
		template<typename _Other, typename _OtherDeleter,
			typename enable_if<
			is_convertible<_Other *, element_type *>::value &&
			is_convertible<_OtherDeleter, deleter_type>::value,
			int>::type = 0>
			intrusive_ptr(intrusive_ptr<_Other, _OtherDeleter> && __rhs) noexcept
			: __ptr(__rhs.__ptr)
		{
			__rhs.__ptr = nullptr;
		}
		intrusive_ptr(const intrusive_ptr & __rhs) noexcept
			: __ptr(__rhs.__ptr)
		{
			if (__ptr) {
				static_cast<const volatile _Intrusive_ptr_impl::_Ref_count_base *>(__ptr)->__add_ref();
			}
		}
		intrusive_ptr(intrusive_ptr && __rhs) noexcept
			: __ptr(__rhs.__ptr)
		{
			__rhs.__ptr = nullptr;
		}

		intrusive_ptr &operator=(nullptr_t) noexcept {
			reset();
			return *this;
		}
		intrusive_ptr &operator=(const intrusive_ptr & __rhs) noexcept {
			reset(__rhs);
			return *this;
		}
		intrusive_ptr &operator=(intrusive_ptr && __rhs) noexcept {
			reset(move(__rhs));
			return *this;
		}

		~intrusive_ptr() {
			auto __old = __ptr;
			if (__old) {
				bool __free_it = static_cast<const volatile _Intrusive_ptr_impl::_Ref_count_base *>(__old)->__drop_ref();
				if (__free_it) {
					_D()(const_cast<typename remove_cv<_T>::type *>(__old));
				}
			}
		}

	public:
		element_type * get() const noexcept {
			return __ptr;
		}
		size_t use_count() const noexcept {
			return static_cast<size_t>(static_cast<const _Intrusive_ptr_impl::_Ref_count_base *>(__ptr)->__get_ref());
		}
		element_type * release() noexcept {
			auto __ret = __ptr;
			__ptr = nullptr;
			return __ret;
		}

		void reset(element_type * __rhs = nullptr) noexcept {
			intrusive_ptr(__rhs).swap(*this);
		}
		template<typename _Other, typename _OtherDeleter>
		void reset(const intrusive_ptr<_Other, _OtherDeleter> & __rhs) noexcept {
			intrusive_ptr(__rhs).swap(*this);
		}
		template<typename _Other, typename _OtherDeleter>
		void reset(intrusive_ptr<_Other, _OtherDeleter> && __rhs) noexcept {
			intrusive_ptr(move(__rhs)).swap(*this);
		}

		void swap(intrusive_ptr & __rhs) noexcept {
			const auto __old = __ptr;
			__ptr = __rhs.__ptr;
			__rhs.__ptr = __old;
		}

		explicit operator bool() const noexcept {
			return __ptr != nullptr;
		}
		explicit operator element_type *() const noexcept {
			return __ptr;
		}

		element_type & operator*() const {
			assert(__ptr);
			return *__ptr;
		}
		element_type * operator->() const {
			assert(__ptr);
			return __ptr;
		}
	};

	namespace _Intrusive_ptr_impl {
		template<class _D>
		template<typename _Other>
		intrusive_ptr<_Other, _D> _Deleteable<_D>::_Weak_observer::__get() const noexcept {
			__parent_lock.__lock();
			auto __other = __static_cast_or_dynamic_cast<_Other *>(__parent);
			if (__other) {
				if (!static_cast<const volatile _Intrusive_ptr_impl::_Ref_count_base *>(__other)->__try_add_ref()) {
					__other = nullptr;
				}
			}
			__parent_lock.__unlock();
			return intrusive_ptr<_Other, _D>(__other);
		}
	}

	template<typename _T, class _D>
	template<typename _Cv_other, typename _Cv_this>
	intrusive_ptr<_Cv_other, _D> intrusive_base<_T, _D>::__fork_shared(_Cv_this * __this) noexcept {
		auto __other = _Intrusive_ptr_impl::__static_cast_or_dynamic_cast<_Cv_other *>(__this);
		if (!__other) {
			return nullptr;
		}
		static_cast<const volatile _Intrusive_ptr_impl::_Ref_count_base *>(__other)->__add_ref();
		return intrusive_ptr<_Cv_other, _D>(__other);
	}

	template<typename _T1, typename _T2, class _D>
	bool operator==(const intrusive_ptr<_T1, _D> & __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return equal_to<void>()(__lhs.get(), __rhs.get());
	}
	template<typename _T1, typename _T2, class _D>
	bool operator==(const intrusive_ptr<_T1, _D> & __lhs, typename intrusive_ptr<_T2, _D>::element_type * __rhs) noexcept {
		return equal_to<void>()(__lhs.get(), __rhs);
	}
	template<typename _T1, typename _T2, class _D>
	bool operator==(typename intrusive_ptr<_T1, _D>::element_type * __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return equal_to<void>()(__lhs, __rhs.get());
	}

	template<typename _T1, typename _T2, class _D>
	bool operator!=(const intrusive_ptr<_T1, _D> & __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return not_equal_to<void>()(__lhs.get(), __rhs.get());
	}
	template<typename _T1, typename _T2, class _D>
	bool operator!=(const intrusive_ptr<_T1, _D> & __lhs, typename intrusive_ptr<_T2, _D>::element_type * __rhs) noexcept {
		return not_equal_to<void>()(__lhs.get(), __rhs);
	}
	template<typename _T1, typename _T2, class _D>
	bool operator!=(typename intrusive_ptr<_T1, _D>::element_type * __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return not_equal_to<void>()(__lhs, __rhs.get());
	}

	template<typename _T1, typename _T2, class _D>
	bool operator<(const intrusive_ptr<_T1, _D> & __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return less<void>()(__lhs.get(), __rhs.get());
	}
	template<typename _T1, typename _T2, class _D>
	bool operator<(const intrusive_ptr<_T1, _D> & __lhs, typename intrusive_ptr<_T2, _D>::element_type * __rhs) noexcept {
		return less<void>()(__lhs.get(), __rhs);
	}
	template<typename _T1, typename _T2, class _D>
	bool operator<(typename intrusive_ptr<_T1, _D>::element_type * __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return less<void>()(__lhs, __rhs.get());
	}

	template<typename _T1, typename _T2, class _D>
	bool operator>(const intrusive_ptr<_T1, _D> & __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return greater<void>()(__lhs.get(), __rhs.get());
	}
	template<typename _T1, typename _T2, class _D>
	bool operator>(const intrusive_ptr<_T1, _D> & __lhs, typename intrusive_ptr<_T2, _D>::element_type * __rhs) noexcept {
		return greater<void>()(__lhs.get(), __rhs);
	}
	template<typename _T1, typename _T2, class _D>
	bool operator>(typename intrusive_ptr<_T1, _D>::element_type * __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return greater<void>()(__lhs, __rhs.get());
	}

	template<typename _T1, typename _T2, class _D>
	bool operator<=(const intrusive_ptr<_T1, _D> & __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return less_equal<void>()(__lhs.get(), __rhs.get());
	}
	template<typename _T1, typename _T2, class _D>
	bool operator<=(const intrusive_ptr<_T1, _D> & __lhs, typename intrusive_ptr<_T2, _D>::element_type * __rhs) noexcept {
		return less_equal<void>()(__lhs.get(), __rhs);
	}
	template<typename _T1, typename _T2, class _D>
	bool operator<=(typename intrusive_ptr<_T1, _D>::element_type * __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return less_equal<void>()(__lhs, __rhs.get());
	}

	template<typename _T1, typename _T2, class _D>
	bool operator>=(const intrusive_ptr<_T1, _D> & __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return greater_equal<void>()(__lhs.get(), __rhs.get());
	}
	template<typename _T1, typename _T2, class _D>
	bool operator>=(const intrusive_ptr<_T1, _D> & __lhs, typename intrusive_ptr<_T2, _D>::element_type * __rhs) noexcept {
		return greater_equal<void>()(__lhs.get(), __rhs);
	}
	template<typename _T1, typename _T2, class _D>
	bool operator>=(typename intrusive_ptr<_T1, _D>::element_type * __lhs, const intrusive_ptr<_T2, _D> & __rhs) noexcept {
		return greater_equal<void>()(__lhs, __rhs.get());
	}

	template<typename _T, class _D>
	void swap(intrusive_ptr<_T, _D> & __lhs, intrusive_ptr<_T, _D> & __rhs) noexcept {
		__lhs.swap(__rhs);
	}

	template<typename _T, class _D = default_delete<_T>, typename ... _Params>
	intrusive_ptr<_T, _D> make_intrusive(_Params &&... __params) {
		static_assert(!is_array<_T>::value, "T shall not be an array type.");

		return intrusive_ptr<_T, _D>(new _T(forward<_Params>(__params)...));
	}

	template<typename _Dst, typename _Src, class _D>
	intrusive_ptr<_Dst, _D> static_pointer_cast(intrusive_ptr<_Src, _D> __src) noexcept {
		return intrusive_ptr<_Dst, _D>(static_cast<_Dst *>(__src.release()));
	}
	template<typename _Dst, typename _Src, class _D>
	intrusive_ptr<_Dst, _D> dynamic_pointer_cast(intrusive_ptr<_Src, _D> __src) noexcept {
		auto __test = dynamic_cast<_Dst *>(__src.get());
		if (!__test) {
			return nullptr;
		}
		__src.release();
		return intrusive_ptr<_Dst, _D>(__test);
	}
	template<typename _Dst, typename _Src, class _D>
	intrusive_ptr<_Dst, _D> const_pointer_cast(intrusive_ptr<_Src, _D> __src) noexcept {
		return intrusive_ptr<_Dst, _D>(const_cast<_Dst *>(__src.release()));
	}

	template<typename _T, class _D>
	class intrusive_weak_ptr {
		static_assert(sizeof(intrusive_base<_T, _D>), "intrusive_base<_T, _D> is not an object type or is an incomplete type.");
		static_assert(sizeof(dynamic_cast<const volatile intrusive_base<_T, _D> *>(declval<_T *>())), "Unable to locate intrusive_base for the managed object type.");

		using _Observer = typename intrusive_base<_T, _D>::_Weak_observer;

		_Observer * __observer;

	public:
		using element_type = _T;
		using deleter_type = _D;

		constexpr intrusive_weak_ptr(nullptr_t = nullptr) noexcept
			: __observer(nullptr)
		{
		}
		explicit intrusive_weak_ptr(element_type * __rhs) // throw(bad_alloc) 
			: __observer(nullptr)
		{
			auto __deleteable = static_cast<const volatile _Intrusive_ptr_impl::_Deleteable<_D> *>(__rhs);
			if (__deleteable) {
				__observer = __rhs->__create_observer();
				static_cast<const volatile _Intrusive_ptr_impl::_Ref_count_base *>(__observer)->__add_ref();
			}
		}
		template<typename _Other, typename _OtherDeleter,
			typename enable_if<
			is_convertible<_Other *, element_type *>::value &&
			is_convertible<_OtherDeleter, deleter_type>::value,
			int>::type = 0>
			intrusive_weak_ptr(const intrusive_ptr<_Other, _OtherDeleter> & __rhs) // throw(bad_alloc) 
			: intrusive_weak_ptr(__rhs.get())
		{
		}
		template<typename _Other, typename _OtherDeleter,
			typename enable_if<
			is_convertible<_Other *, element_type *>::value &&
			is_convertible<_OtherDeleter, deleter_type>::value,
			int>::type = 0>
			intrusive_weak_ptr(const intrusive_weak_ptr<_Other, _OtherDeleter> & __rhs) noexcept
			: __observer(__rhs.__observer)
		{
			if (__observer) {
				static_cast<const volatile _Intrusive_ptr_impl::_Ref_count_base *>(__observer)->__add_ref();
			}
		}
		template<typename _Other, typename _OtherDeleter,
			typename enable_if<
			is_convertible<_Other *, element_type *>::value &&
			is_convertible<_OtherDeleter, deleter_type>::value,
			int>::type = 0>
			intrusive_weak_ptr(intrusive_weak_ptr<_Other, _OtherDeleter> && __rhs) noexcept
			: __observer(__rhs.__observer)
		{
			__rhs.__observer = nullptr;
		}
		intrusive_weak_ptr(const intrusive_weak_ptr & __rhs) noexcept
			: __observer(__rhs.__observer)
		{
			if (__observer) {
				static_cast<const volatile _Intrusive_ptr_impl::_Ref_count_base *>(__observer)->__add_ref();
			}
		}
		intrusive_weak_ptr(intrusive_weak_ptr && __rhs) noexcept
			: __observer(__rhs.__observer)
		{
			__rhs.__observer = nullptr;
		}

		intrusive_weak_ptr & operator=(nullptr_t) noexcept {
			reset();
			return *this;
		}
		intrusive_weak_ptr & operator=(const intrusive_weak_ptr & __rhs) noexcept {
			reset(__rhs);
			return *this;
		}
		intrusive_weak_ptr & operator=(intrusive_weak_ptr && __rhs) noexcept {
			reset(move(__rhs));
			return *this;
		}

		~intrusive_weak_ptr() {
			auto __old = __observer;
			if (__old) {
				bool __free_it = static_cast<const volatile _Intrusive_ptr_impl::_Ref_count_base *>(__old)->__drop_ref();
				if (__free_it) {
					delete __old;
				}
			}
		}

	public:
		bool expired() const noexcept {
			return __observer ? __observer->__expired() : true;
		}
		intrusive_ptr<_T, _D> lock() const noexcept {
			return __observer ? __observer->template __get<_T>() : nullptr;
		}

		void reset(element_type * __rhs = nullptr) {
			intrusive_weak_ptr(__rhs).swap(*this);
		}
		template<typename _Other, typename _OtherDeleter>
		void reset(const intrusive_ptr<_Other, _OtherDeleter> & __rhs) {
			intrusive_weak_ptr(__rhs).swap(*this);
		}
		template<typename _Other, typename _OtherDeleter>
		void reset(const intrusive_weak_ptr<_Other, _OtherDeleter> & __rhs) noexcept {
			intrusive_weak_ptr(__rhs).swap(*this);
		}
		template<typename _Other, typename _OtherDeleter>
		void reset(intrusive_weak_ptr<_Other, _OtherDeleter> && __rhs) noexcept {
			intrusive_weak_ptr(move(__rhs)).swap(*this);
		}

		void swap(intrusive_weak_ptr & __rhs) noexcept {
			const auto __old = __observer;
			__observer = __rhs.__observer;
			__rhs.__observer = __old;
		}

		template<typename _Other>
		bool operator==(const intrusive_weak_ptr<_Other, _D> & __rhs) const noexcept {
			return equal_to<void>()(__observer, __rhs.__observer);
		}
		template<typename _Other>
		bool operator!=(const intrusive_weak_ptr<_Other, _D> & __rhs) const noexcept {
			return not_equal_to<void>()(__observer, __rhs.__observer);
		}
		template<typename _Other>
		bool operator<(const intrusive_weak_ptr<_Other, _D> & __rhs) const noexcept {
			return less<void>()(__observer, __rhs.__observer);
		}
		template<typename _Other>
		bool operator>(const intrusive_weak_ptr<_Other, _D> & __rhs) const noexcept {
			return greater<void>()(__observer, __rhs.__observer);
		}
		template<typename _Other>
		bool operator<=(const intrusive_weak_ptr<_Other, _D> & __rhs) const noexcept {
			return less_equal<void>()(__observer, __rhs.__observer);
		}
		template<typename _Other>
		bool operator>=(const intrusive_weak_ptr<_Other, _D> & __rhs) const noexcept {
			return greater_equal<void>()(__observer, __rhs.__observer);
		}
	};

	template<typename _T, class _D>
	template<typename _Cv_other, typename _Cv_this>
	intrusive_weak_ptr<_Cv_other, _D> intrusive_base<_T, _D>::__fork_weak(_Cv_this * __this) {
		auto __other = _Intrusive_ptr_impl::__static_cast_or_dynamic_cast<_Cv_other *>(__this);
		if (!__other) {
			return nullptr;
		}
		return intrusive_weak_ptr<_Cv_other, _D>(__other);
	}

	template<typename _T, class _D>
	void swap(intrusive_weak_ptr<_T, _D> & __lhs, intrusive_weak_ptr<_T, _D> & __rhs) noexcept {
		__lhs.swap(__rhs);
	}

}

#endif
