/*! \file pseudo_mutex.h
\ingroup LBase
\brief α��������
*/

#ifndef LBase_pseudo_mutex_h
#define LBase_pseudo_mutex_h 1

#include "LBase/sutility.h" // for noncopyable, nonmovable, std::declval;
#include "LBase/exception.h" // for throw_error, std::errc;
#include <chrono> // for std::chrono::duration, std::chrono::time_point;

namespace leo
{
	namespace threading
	{
		/*!
		\see ISO C++14 30.4.2[thread.lock] ��
		*/
		//@{
		lconstexpr const struct defer_lock_t
		{} defer_lock{};

		lconstexpr const struct try_to_lock_t
		{} try_to_lock{};

		lconstexpr const struct adopt_lock_t
		{} adopt_lock{};
		//@}

		/*!
		\note ��һģ���������Ҫ��֤���� BasicLockable Ҫ��
		\warning ����������
		*/
		//@{
		//! \pre _tReference ֵ�� get() ���ص��������� BasicLockable Ҫ��
		template<class _type, typename _tReference = lref<_type>>
		class lockable_adaptor
		{
		public:
			//! \since build 722
			using holder_type = _type;
			using reference = _tReference;

		private:
			reference ref;

		public:
			//@{
			lockable_adaptor(holder_type& m) lnothrow
				: ref(m)
			{}

			explicit
				operator holder_type&() lnothrow
			{
				return ref;
			}
			explicit
				operator const holder_type&() const lnothrow
			{
				return ref;
			}

			void
				lock()
			{
				ref.get().lock();
			}

			//! \pre holder_type ���� Lockable Ҫ��
			bool
				try_lock()
			{
				return ref.get().try_lock();
			}

			//! \pre holder_type ���� TimedLockable Ҫ��
			//@{
			template<typename _tRep, typename _tPeriod>
			bool
				try_lock_for(const std::chrono::duration<_tRep, _tPeriod>& rel_time)
			{
				return ref.get().try_lock_for(rel_time);
			}

			template<typename _tClock, typename _tDuration>
			bool
				try_lock_until(const std::chrono::time_point<_tClock, _tDuration>& abs_time)
			{
				return ref.get().try_lock_until(abs_time);
			}
			//@}

			void
				unlock() lnothrowv
			{
				ref.get().unlock();
			}
			//@}
		};


		//! \pre _tReference ֵ�� get() ���ص��������㹲�� BasicLockable Ҫ��
		template<class _type, typename _tReference = lref<_type>>
		class shared_lockable_adaptor
		{
		public:
			using holder_type = _type;
			using reference = _tReference;

		private:
			reference ref;

		public:
			shared_lockable_adaptor(holder_type& m) lnothrow
				: ref(m)
			{}

			explicit
				operator holder_type&() lnothrow
			{
				return ref;
			}
			explicit
				operator const holder_type&() const lnothrow
			{
				return ref;
			}

			//@{
			void
				lock()
			{
				ref.get().lock_shared();
			}

			//! \pre holder_type ���� Lockable Ҫ��
			bool
				try_lock()
			{
				return ref.get().try_lock_shared();
			}

			//! \pre holder_type ���� TimedLockable Ҫ��
			//@{
			template<typename _tRep, typename _tPeriod>
			bool
				try_lock_for(const std::chrono::duration<_tRep, _tPeriod>& rel_time)
			{
				return ref.get().try_lock_shared_for(rel_time);
			}

			template<typename _tClock, typename _tDuration>
			bool
				try_lock_until(const std::chrono::time_point<_tClock, _tDuration>& abs_time)
			{
				return ref.get().try_lock_shared_until(abs_time);
			}
			//@}

			void
				unlock() lnothrow
			{
				ref.get().unlock_shared();
			}
			//@}
		};
		//@}


		//@{
		/*!
		\note ��һ����Ϊ�洢�Ļ��������͡�
		\note �ڶ����������Ƿ���м�飬�����ʱ�Ż�ʵ��Ϊ�ղ�����
		\warning �����ʱ����֤�̰߳�ȫ�����̻߳����¼ٶ��߳�ͬ�������������δ������Ϊ��
		*/
		//@{
		/*!
		\pre _tReference ���� BasicLockable Ҫ��
		\pre ��̬���ԣ� _tReference ʹ�� _tMutex ��ֵ��ʼ����֤���쳣�׳���
		\note ��������Ϊ���õ��������͡�
		\warning ����������
		*/
		//@{
		template<class _tMutex, bool = true, typename _tReference = _tMutex&>
		class lock_guard : private limpl(noncopyable), private limpl(nonmovable)
		{
			lnoexcept_assert("Invalid type found.",
				_tReference(std::declval<_tMutex&>()));

		public:
			using mutex_type = _tMutex;
			using reference = _tReference;

		private:
			mutex_type& owned;

		public:
			/*!
			\pre �� mutex_type �ǵݹ����������̲߳���������
			\post <tt>std::addressof(owned) == std::addressof(m)</tt> ��
			*/
			explicit
				lock_guard(mutex_type& m)
				: owned(m)
			{
				reference(owned).lock();
			}
			/*!
			\pre �����̳߳�������
			\post <tt>std::addressof(owned) == std::addressof(m)</tt> ��
			*/
			lock_guard(mutex_type& m, adopt_lock_t) limpl(lnothrow)
				: owned(m)
			{}
			~lock_guard()
			{
				reference(owned).unlock();
			}
		};

		template<class _tMutex, typename _tReference>
		class lock_guard<_tMutex, false, _tReference>
			: private limpl(noncopyable), private limpl(nonmovable)
		{
		public:
			using mutex_type = _tMutex;

			explicit
				lock_guard(mutex_type&) limpl(lnothrow)
			{}
			lock_guard(mutex_type&, adopt_lock_t) limpl(lnothrow)
			{}
		};


		//! \brief �����ࣺ�������ռ���͹�������
		//@{
		template<class _tMutex, bool _bEnableCheck = true, class _tReference = _tMutex&>
		class lock_base : private noncopyable
		{
			lnoexcept_assert("Invalid type found.",
				_tReference(std::declval<_tMutex&>()));

		public:
			using mutex_type = _tMutex;
			using reference = _tReference;

		private:
			mutex_type* pm;
			bool owns;

		public:
			lock_base() lnothrow
				: pm(), owns()
			{}

			//! \post <tt>pm == std::addressof(m)</tt> ��
			//@{
			explicit
				lock_base(mutex_type& m)
				: lock_base(m, defer_lock)
			{
				lock();
				owns = true;
			}
			lock_base(mutex_type& m, defer_lock_t) lnothrow
				: pm(std::addressof(m)), owns()
			{}
			/*!
			\pre mutex_type ���� Lockable Ҫ��
			\pre �� mutex_type �ǵݹ����������̲߳���������
			*/
			lock_base(mutex_type& m, try_to_lock_t)
				: pm(std::addressof(m)), owns(get_ref().try_lock())
			{}
			//! \pre �����̳߳�������
			lock_base(mutex_type& m, adopt_lock_t) limpl(lnothrow)
				: pm(std::addressof(m)), owns(true)
			{}
			/*!
			\pre mutex_type ���� TimedLockable Ҫ��
			\pre �� mutex_type �ǵݹ����������̲߳���������
			*/
			//@{
			template<typename _tClock, typename _tDuration>
			lock_base(mutex_type& m, const std::chrono::time_point<_tClock,
				_tDuration>& abs_time)
				: pm(std::addressof(m)), owns(get_ref().try_lock_until(abs_time))
			{}
			template<typename _tRep, typename _tPeriod>
			lock_base(mutex_type& m,
				const std::chrono::duration<_tRep, _tPeriod>& rel_time)
				: pm(std::addressof(m)), owns(get_ref().try_lock_for(rel_time))
			{}
			//@}
			//@}
			/*!
			\post <tt>pm == u_p.pm && owns == u_p.owns</tt> �� \c u_p �� u ֮ǰ��״̬��
			\post <tt>!u.pm && !u.owns</tt> ��
			*/
			lock_base(lock_base&& u) lnothrow
				: pm(u.pm), owns(u.owns)
			{
				lunseq(u.pm = {}, u.owns = {});
			}
			~lock_base()
			{
				if (owns)
					unlock();
			}

			/*!
			\post <tt>pm == u_p.pm && owns == u_p.owns</tt> �� \c u_p �� u ֮ǰ��״̬��
			\post <tt>!u.pm && !u.owns</tt> ��
			\see LWG 2104 ��
			*/
			lock_base&
				operator=(lock_base&& u) limpl(lnothrow)
			{
				if (owns)
					unlock();
				swap(u, *this);
				u.clear_members();
				return *this;
			}

			explicit
				operator bool() const lnothrow
			{
				return owns;
			}

		private:
			void
				check_lock()
			{
				using namespace std;

				if (!pm)
					throw_error(errc::operation_not_permitted);
				if (owns)
					throw_error(errc::resource_deadlock_would_occur);
			}

			void
				clear_members() limpl(lnothrow)
			{
				lunseq(pm = {}, owns = {});
			}

			reference
				get_ref() lnothrow
			{
				lassume(pm);
				return reference(*pm);
			}

		public:
			void
				lock()
			{
				check_lock();
				get_ref().lock();
				owns = true;
			}

			//! \pre reference ���� Lockable Ҫ��
			bool
				try_lock()
			{
				check_lock();
				return owns = get_ref().try_lock();
			}

			//! \pre reference ���� TimedLockable Ҫ��
			//@{
			template<typename _tRep, typename _tPeriod>
			bool
				try_lock_for(const std::chrono::duration<_tRep, _tPeriod>& rel_time)
			{
				check_lock();
				return owns = get_ref().try_lock_for(rel_time);
			}

			template<typename _tClock, typename _tDuration>
			bool
				try_lock_until(const std::chrono::time_point<_tClock, _tDuration>& abs_time)
			{
				check_lock();
				return owns = get_ref().try_lock_until(abs_time);
			}
			//@}

			void
				unlock()
			{
				if (!owns)
					throw_error(std::errc::operation_not_permitted);
				if (pm)
				{
					get_ref().unlock();
					owns = {};
				}
			}

			friend void
				swap(lock_base& x, lock_base& y) lnothrow
			{
				std::swap(x.pm, y.pm),
					std::swap(x.owns, y.owns);
			}

			mutex_type*
				release() lnothrow
			{
				const auto res(pm);

				clear_members();
				return res;
			}

			bool
				owns_lock() const lnothrow
			{
				return owns;
			}

			mutex_type*
				mutex() const lnothrow
			{
				return pm;
			}
		};

		template<class _tMutex, class _tReference>
		class lock_base<_tMutex, false, _tReference>
		{
		public:
			using mutex_type = _tMutex;

			lock_base() lnothrow limpl(= default);
			explicit
				lock_base(mutex_type&) limpl(lnothrow)
			{}
			lock_base(mutex_type&, defer_lock_t) lnothrow
			{}
			lock_base(mutex_type&, try_to_lock_t) limpl(lnothrow)
			{}
			lock_base(mutex_type&, adopt_lock_t) limpl(lnothrow)
			{}
			template<typename _tClock, typename _tDuration>
			lock_base(mutex_type&,
				const std::chrono::time_point<_tClock, _tDuration>&) limpl(lnothrow)
			{}
			template<typename _tRep, typename _tPeriod>
			lock_base(mutex_type&, const std::chrono::duration<_tRep, _tPeriod>&)
				limpl(lnothrow)
			{}
			lock_base(lock_base&&) limpl(= default);

			lock_base&
				operator=(lock_base&&) = limpl(default);

			explicit
				operator bool() const lnothrow
			{
				return true;
			}

			void
				lock()
			{}

			bool
				try_lock() limpl(lnothrow)
			{
				return true;
			}

			template<typename _tRep, typename _tPeriod>
			bool
				try_lock_for(const std::chrono::duration<_tRep, _tPeriod>&) limpl(lnothrow)
			{
				return true;
			}

			template<typename _tClock, typename _tDuration>
			bool
				try_lock_until(const std::chrono::time_point<_tClock, _tDuration>&)
				limpl(lnothrow)
			{
				return true;
			}

			void
				unlock() limpl(lnothrow)
			{}

			friend void
				swap(lock_base&, lock_base&) lnothrow
			{}

			mutex_type*
				release() lnothrow
			{
				return {};
			}

			bool
				owns_lock() const lnothrow
			{
				return true;
			}

			mutex_type*
				mutex() const lnothrow
			{
				return {};
			}
		};
		//@}
		//@}


		//! \warning ����������
		template<class _tMutex, bool _bEnableCheck = true>
		class unique_lock : private limpl(lock_base<_tMutex, _bEnableCheck>)
		{
		private:
			using base = limpl(lock_base<_tMutex, _bEnableCheck>);

		public:
			using mutex_type = _tMutex;

		public:
			unique_lock() lnothrow limpl(= default);
			using limpl(base::base);
			unique_lock(unique_lock&&) limpl(= default);

			/*!
			\post <tt>pm == u_p.pm && owns == u_p.owns</tt> �� \c u_p �� u ֮ǰ��״̬��
			\post <tt>!u.pm && !u.owns</tt> ��
			\see LWG 2104 ��
			\since build 722
			*/
			unique_lock&
				operator=(unique_lock&&) limpl(= default);

			using limpl(base)::operator bool;

			using limpl(base)::lock;

			//! \pre mutex_type ���� Lockable Ҫ��
			using limpl(base)::try_lock;

			//! \pre mutex_type ���� TimedLockable Ҫ��
			//@{
			using limpl(base)::try_lock_for;

			using limpl(base)::try_lock_until;
			//@}

			using limpl(base)::unlock;

			void
				swap(unique_lock& u) lnothrow
			{
				swap(static_cast<base&>(*this), static_cast<base&>(u));
			}

			using limpl(base)::release;

			using limpl(base)::owns_lock;

			using limpl(base)::mutex;
		};

		//! \relates unique_lock
		template<class _tMutex, bool _bEnableCheck>
		inline void
			swap(unique_lock<_tMutex, _bEnableCheck>& x,
				unique_lock<_tMutex, _bEnableCheck>& y) lnothrow
		{
			x.swap(y);
		}


		//! \warning ����������
		template<class _tMutex, bool _bEnableCheck = true>
		class shared_lock : private limpl(lock_base<_tMutex, _bEnableCheck,
			shared_lockable_adaptor<_tMutex>>)
		{
		private:
			using base = limpl(lock_base<_tMutex, _bEnableCheck,
				shared_lockable_adaptor<_tMutex>>);

		public:
			using mutex_type = _tMutex;

		public:
			shared_lock() lnothrow limpl(= default);
			using limpl(base::base);
			shared_lock(shared_lock&&) limpl(= default);

			/*!
			\post <tt>pm == u_p.pm && owns == u_p.owns</tt> �� \c u_p �� u ֮ǰ��״̬��
			\post <tt>!u.pm && !u.owns</tt> ��
			\see LWG 2104 ��
			*/
			shared_lock&
				operator=(shared_lock&&) limpl(= default);

			using limpl(base)::operator bool;

			//@{
			void
				lock()
			{
				base::lock_shared();
			}

			//! \pre mutex_type ���� Lockable Ҫ��
			bool
				try_lock()
			{
				return base::try_lock_shared();
			}

			//! \pre mutex_type ���� TimedLockable Ҫ��
			//@{
			template<typename _tRep, typename _tPeriod>
			bool
				try_lock_shared_for(const std::chrono::duration<_tRep, _tPeriod>& rel_time)
			{
				return base::template try_lock_shared_for(rel_time);
			}

			template<typename _tClock, typename _tDuration>
			bool
				try_lock_shared_until(const
					std::chrono::time_point<_tClock, _tDuration>& abs_time)
			{
				return base::template try_lock_shared_until(abs_time);
			}
			//@}
			//@}

			using limpl(base)::unlock;

			void
				swap(shared_lock& u) lnothrow
			{
				swap(static_cast<base&>(*this), static_cast<base&>(u));
			}

			using limpl(base)::release;

			using limpl(base)::owns_lock;

			using limpl(base)::mutex;
		};

		//! \relates shared_lock
		template<class _tMutex, bool _bEnableCheck>
		inline void
			swap(shared_lock<_tMutex, _bEnableCheck>& x,
				shared_lock<_tMutex, _bEnableCheck>& y) lnothrow
		{
			x.swap(y);
		}
		//@}
		//@}

	} // namespace threading;

	/*!
	\brief ���̲߳�������֤���̻߳����½ӿڼ����϶�Ӧ�� std �����ռ��µĽӿڡ�
	\note ����������������صĽӿڡ�
	\since build 1.3
	\todo ��� ISO C++ 14 ��������
	*/
	namespace single_thread
	{

		//! \since build 1.3
		//@{
		lconstexpr const struct defer_lock_t
		{} defer_lock{};

		lconstexpr const struct try_to_lock_t
		{} try_to_lock{};

		lconstexpr const struct adopt_lock_t
		{} adopt_lock{};


		//! \warning ����֤�̰߳�ȫ�����̻߳����¼ٶ��߳�ͬ�������������δ������Ϊ��
		//@{
		//! \see ISO C++11 [thread.mutex.requirements.mutex] ��
		//@{
		class LB_API mutex : private limpl(noncopyable), private limpl(nonmovable)
		{
		public:
			lconstfn
				mutex() limpl(= default);
			~mutex() limpl(= default);

			//! \pre �����̲߳���������
			void
				lock() limpl(lnothrow)
			{}

			bool
				try_lock() limpl(lnothrow)
			{
				return true;
			}

			//! \pre �����̳߳�������
			void
				unlock() limpl(lnothrow)
			{}
		};


		class LB_API recursive_mutex
			: private limpl(noncopyable), private limpl(nonmovable)
		{
		public:
			recursive_mutex() limpl(= default);
			~recursive_mutex() limpl(= default);

			void
				lock() limpl(lnothrow)
			{}

			bool
				try_lock() limpl(lnothrow)
			{
				return true;
			}

			//! \pre �����̳߳�������
			void
				unlock() limpl(lnothrow)
			{}
		};


		//! \see ISO C++11 [thread.timedmutex.requirements] ��
		//@{
		class LB_API timed_mutex : private limpl(noncopyable), private limpl(nonmovable)
		{
		public:
			timed_mutex() limpl(= default);
			~timed_mutex() limpl(= default);

			//! \pre �����̲߳���������
			//@{
			void
				lock() limpl(lnothrow)
			{}

			bool
				try_lock() limpl(lnothrow)
			{
				return true;
			}

			template<typename _tRep, typename _tPeriod>
			bool
				try_lock_for(const std::chrono::duration<_tRep, _tPeriod>&) limpl(lnothrow)
			{
				return true;
			}

			template<typename _tClock, typename _tDuration>
			bool
				try_lock_until(const std::chrono::time_point<_tClock, _tDuration>&)
				limpl(lnothrow)
			{
				return true;
			}
			//@}

			//! \pre �����̳߳�������
			void
				unlock() limpl(lnothrow)
			{}
		};


		class LB_API recursive_timed_mutex
			: private limpl(noncopyable), private limpl(nonmovable)
		{
		public:
			recursive_timed_mutex() limpl(= default);
			~recursive_timed_mutex() limpl(= default);

			void
				lock() limpl(lnothrow)
			{}

			bool
				try_lock() limpl(lnothrow)
			{
				return true;
			}

			template<typename _tRep, typename _tPeriod>
			bool
				try_lock_for(const std::chrono::duration<_tRep, _tPeriod>&) limpl(lnothrow)
			{
				return true;
			}

			template<typename _tClock, typename _tDuration>
			bool
				try_lock_until(const std::chrono::time_point<_tClock, _tDuration>&)
				limpl(lnothrow)
			{
				return true;
			}

			//! \pre �����̳߳�������
			void
				unlock() limpl(lnothrow)
			{}
		};
		//@}
		//@}


		template<class _tMutex>
		class lock_guard : private limpl(noncopyable), private limpl(nonmovable)
		{
		public:
			using mutex_type = _tMutex;

#ifdef NDEBUG
			explicit
				lock_guard(mutex_type&) limpl(lnothrow)
			{}
			lock_guard(mutex_type&, adopt_lock_t) limpl(lnothrow)
			{}

#else
		private:
			mutex_type& pm;

		public:
			/*!
			\pre �� mutex_type �ǵݹ����������̲߳���������
			\post <tt>pm == &m</tt> ��
			*/
			explicit
				lock_guard(mutex_type& m) limpl(lnothrow)
				: pm(m)
			{
				m.lock();
			}
			/*!
			\pre �����̳߳�������
			\post <tt>pm == &m</tt> ��
			*/
			lock_guard(mutex_type& m, adopt_lock_t) limpl(lnothrow)
				: pm(&m)
			{}
			~lock_guard() limpl(lnothrow)
			{
				pm.unlock();
			}
#endif
		};


		//! \note ����� \c NDEBUG ʱ�����м�飬�Ż�ʵ��Ϊ�ղ�����
		//@{
		template<class _tMutex>
		class unique_lock : private limpl(noncopyable)
		{
		public:
			using mutex_type = _tMutex;

#ifdef NDEBUG
			//! \since build 1.4
			//@{
			unique_lock() limpl(= default);
			explicit
				unique_lock(mutex_type&) limpl(lnothrow)
			{}
			//! \since build 1.4
			unique_lock(mutex_type&, defer_lock_t) lnothrow
			{}
			unique_lock(mutex_type&, try_to_lock_t) limpl(lnothrow)
			{}
			unique_lock(mutex_type&, adopt_lock_t) limpl(lnothrow)
			{}
			template<typename _tClock, typename _tDuration>
			unique_lock(mutex_type&,
				const std::chrono::time_point<_tClock, _tDuration>&) limpl(lnothrow)
			{}
			template<typename _tRep, typename _tPeriod>
			unique_lock(mutex_type&, const std::chrono::duration<_tRep, _tPeriod>&)
				limpl(lnothrow)
			{}
			unique_lock(unique_lock&&) limpl(= default);

			explicit
				operator bool() const lnothrow
			{
				return true;
			}

			void
				lock()
			{}

			bool
				owns_lock() const lnothrow
			{
				return true;
			}

			mutex_type*
				release() lnothrow
			{
				return{};
			}

			void
				swap(unique_lock&) lnothrow
			{}

			bool
				try_lock() limpl(lnothrow)
			{
				return true;
			}

			template<typename _tRep, typename _tPeriod>
			bool
				try_lock_for(const std::chrono::duration<_tRep, _tPeriod>&) limpl(lnothrow)
			{
				return true;
			}

			template<typename _tClock, typename _tDuration>
			bool
				try_lock_until(const std::chrono::time_point<_tClock, _tDuration>&)
				limpl(lnothrow)
			{
				return true;
			}

			void
				unlock() limpl(lnothrow)
			{}

			mutex_type*
				mutex() const lnothrow
			{
				return{};
			}
			//@}
#else
		private:
			mutex_type* pm;
			bool owns;

		public:
			unique_lock() lnothrow
				: pm(), owns()
			{}

			//! \post <tt>pm == &m</tt> ��
			//@{
			explicit
				unique_lock(mutex_type& m) limpl(lnothrow)
				: unique_lock(m, defer_lock)
			{
				lock();
				owns = true;
			}
			unique_lock(mutex_type& m, defer_lock_t) lnothrow
				: pm(&m), owns()
			{}
			/*!
			\pre mutex_type ���� Lockable Ҫ��
			\pre �� mutex_type �ǵݹ����������̲߳���������
			*/
			unique_lock(mutex_type& m, try_to_lock_t) limpl(lnothrow)
				: pm(&m), owns(pm->try_lock())
			{}
			//! \pre �����̳߳�������
			unique_lock(mutex_type& m, adopt_lock_t) limpl(lnothrow)
				: pm(&m), owns(true)
			{}
			/*!
			\pre mutex_type ���� TimedLockable Ҫ��
			\pre �� mutex_type �ǵݹ����������̲߳���������
			*/
			//@{
			template<typename _tClock, typename _tDuration>
			unique_lock(mutex_type& m, const std::chrono::time_point<_tClock,
				_tDuration>& abs_time) limpl(lnothrow)
				: pm(&m), owns(pm->try_lock_until(abs_time))
			{}
			template<typename _tRep, typename _tPeriod>
			unique_lock(mutex_type& m,
				const std::chrono::duration<_tRep, _tPeriod>& rel_time) limpl(lnothrow)
				: pm(&m), owns(pm->try_lock_for(rel_time))
			{}
			//@}
			//@}
			/*!
			\post <tt>pm == u_p.pm && owns == u_p.owns</tt> �� \c u_p �� u ֮ǰ��״̬��
			\post <tt>!u.pm && !u.owns</tt> ��
			*/
			unique_lock(unique_lock&& u) lnothrow
				: pm(u.pm), owns(u.owns)
			{
				lunseq(u.pm = {}, u.owns = {});
			}
			~unique_lock()
			{
				if (owns)
					unlock();
			}

			/*!
			\post <tt>pm == u_p.pm && owns == u_p.owns</tt> �� \c u_p �� u ֮ǰ��״̬��
			\post <tt>!u.pm && !u.owns</tt> ��
			\see http://wg21.cmeerw.net/lwg/issue2104 ��
			*/
			unique_lock&
				operator=(unique_lock&& u) limpl(lnothrow)
			{
				if (owns)
					unlock();
				unique_lock(std::move(u)).swap(*this);
				u.clear_members();
				return *this;
			}

			explicit
				operator bool() const lnothrow
			{
				return owns;
			}

		private:
			void
				check_lock() limpl(lnothrow)
			{
				using namespace std;

				if (!pm)
					throw_error(errc::operation_not_permitted);
				if (owns)
					throw_error(errc::resource_deadlock_would_occur);
			}

			void
				clear_members() limpl(lnothrow)
			{
				lunseq(pm = {}, owns = {});
			}

		public:
			void
				lock() limpl(lnothrow)
			{
				check_lock();
				pm->lock();
				owns = true;
			}

			bool
				owns_lock() const lnothrow
			{
				return owns;
			}

			mutex_type*
				release() lnothrow
			{
				const auto res(pm);

				clear_members();
				return res;
			}

			void
				swap(unique_lock& u) lnothrow
			{
				std::swap(pm, u.pm),
					std::swap(owns, u.owns);
			}

			//! \pre mutex_type ���� Lockable Ҫ��
			bool
				try_lock() limpl(lnothrow)
			{
				check_lock();
				return owns = pm->lock();
			}

			//! \pre mutex_type ���� TimedLockable Ҫ��
			//@{
			template<typename _tRep, typename _tPeriod>
			bool
				try_lock_for(const std::chrono::duration<_tRep, _tPeriod>& rel_time)
				limpl(lnothrow)
			{
				check_lock();
				return owns = pm->try_lock_for(rel_time);
			}

			template<typename _tClock, typename _tDuration>
			bool
				try_lock_until(const std::chrono::time_point<_tClock, _tDuration>& abs_time)
				limpl(lnothrow)
			{
				check_lock();
				return owns = pm->try_lock_until(abs_time);
			}
			//@}

			void
				unlock() limpl(lnothrow)
			{
				if (!owns)
					throw_error(std::errc::operation_not_permitted);
				if (pm)
				{
					pm->unlock();
					owns = {};
				}
			}

			mutex_type*
				mutex() const lnothrow
			{
				return pm;
			}
#endif
		};


		//! \since build 1.4
		struct LB_API once_flag : private limpl(noncopyable), private limpl(nonmovable)
		{
			limpl(bool) state = {};

			lconstfn
				once_flag() lnothrow limpl(= default);
		};


		/*!
		\brief ����ʶ���ú�������֤����һ�Ρ�
		\note ���� std::call_once ��������֤�̰߳�ȫ�ԡ�
		\note ISO C++11���� N3691 �� 30.4 synopsis �����������ڴ���
		\bug δʵ��֧�ֳ�Աָ�롣
		\see https://github.com/cplusplus/draft/issues/151 ��
		\see http://wg21.cmeerw.net/cwg/issue1591 ��
		\see http://wg21.cmeerw.net/cwg/issue2442 ��
		\since build 1.4

		����ʶΪ�ǳ�ֵʱ�������ã�������ú�����
		*/
		template<typename _fCallable, typename... _tParams>
		inline void
			call_once(once_flag& flag, _fCallable&& f, _tParams&&... args)
		{
			if (!flag.state)
			{
				f(lforward(args)...);
				flag.state = true;
			}
		}
		//@}

		//! \relates unique_lock
		template<class _tMutex>
		inline void
			swap(unique_lock<_tMutex>& x, unique_lock<_tMutex>& y) lnothrow
		{
			x.swap(y);
		}


		template<class _tLock1, class _tLock2, class... _tLocks>
		void
			lock(_tLock1&&, _tLock2&&, _tLocks&&...) limpl(lnothrow)
		{}

		template<class _tLock1, class _tLock2, class... _tLocks>
		lconstfn int
			try_lock(_tLock1&&, _tLock2&&, _tLocks&&...) limpl(lnothrow)
		{
			return -1;
		}
		//@}
		//@}

	} // namespace single_thread;


	  /*!
	  \brief �ڵ��̻߳����Ͷ��̻߳����¶����õ��߳�ͬ���ӿڡ�
	  \since build 1.3
	  */
	namespace threading
	{

		/*!
		\brief ����ɾ������
		\pre _tMutex ���� \c BasicLockable Ҫ��
		\since build 1.4
		*/
		template<class _tMutex = single_thread::mutex,
			class _tLock = single_thread::unique_lock<_tMutex>>
			class unlock_delete : private noncopyable
		{
		public:
			using mutex_type = _tMutex;
			using lock_type = _tLock;

			mutable lock_type lock;

			unlock_delete(mutex_type& mtx)
				: lock(mtx)
			{}
			template<typename
				= limpl(enable_if_t)<is_nothrow_move_constructible<lock_type>::value>>
				unlock_delete(lock_type&& lck) lnothrow
				: lock(std::move(lck))
			{}
			template<typename... _tParams>
			unlock_delete(mutex_type& mtx, _tParams&&... args) lnoexcept(
				std::declval<mutex_type&>()(std::declval<_tParams&&>()...))
				: lock(mtx, lforward(args)...)
			{}

			//! \brief ɾ����������
			template<typename _tPointer>
			void
				operator()(const _tPointer&) const lnothrow
			{
				lock.unlock();
			}
		};


		/*!
		\brief ��ռ����Ȩ������ָ�롣
		\since build 1.3
		*/
		template<typename _type, class _tMutex = single_thread::mutex,
			class _tLock = single_thread::unique_lock<_tMutex>>
			using locked_ptr = std::unique_ptr<_type, unlock_delete<_tMutex, _tLock>>;
	} // namespace threading;

} // namespace leo;

#endif