/*! \file any.h
\ingroup LBase
\brief 动态泛型类型。
\par 修改时间:
2016-11-23 11:49 +0800

\see WG21 N4582 20.6[any] 。
\see http://www.boost.org/doc/libs/1_60_0/doc/html/any/reference.html 。
*/

#ifndef LBase_any_h
#define LBase_any_h 1

#include "LBase/typeinfo.h" //for "typeinfo.h“，cloneable,type_id_info,
// leo::type_id,std::bad_cast
#include "LBase/memory.hpp" //for leo::addressof
#include "LBase/utility.hpp" // "utility.hpp", for boxed_value,
//	standard_layout_storage, aligned_storage_t, is_aligned_storable,
//	exclude_self_t, enable_if_t, decay_t, lconstraint;
#include "LBase/exception.h"
#include "LBase/ref.hpp" // for is_reference_wrapper, unwrap_reference_t;
#include "LBase/placement.hpp"
#include <initializer_list>
#include <memory> //std::unique_ptr

namespace leo
{
	/*!
	\brief any 操作的命名空间。
	\sa any
	\since change 1.4
	*/
	namespace any_ops
	{
		//! \since build 1.4
		template<typename>
		struct with_handler_t
		{};


		/*!
		\brief 抽象动态泛型持有者接口。
		\since build 1.4
		*/
		class LB_API holder : public cloneable
		{
		public:
			//! \since build 1.4
			//@{
			holder() = default;
			holder(const holder&) = default;
			//! \brief 虚析构：类定义外默认实现。
			~holder() override;
			//@}

			virtual void*
				get() const = 0;

			virtual holder*
				clone() const override = 0;

			//! \since build 1.4
			virtual const type_info&
				type() const lnothrow = 0;
		};


		/*!
		\brief 值类型动态泛型持有者。
		\pre 值类型不被 cv-qualifier 修饰。
		*/
		template<typename _type>
		class value_holder : protected boxed_value<_type>, public holder
		{
			static_assert(is_object<_type>::value, "Non-object type found.");
			static_assert(!is_cv<_type>::value, "Cv-qualified type found.");

		public:
			using value_type = _type;

			//! \since build 1.4
			//@{
			value_holder() = default;
			template<typename _tParam,
				limpl(typename = exclude_self_t<value_holder, _tParam>)>
				value_holder(_tParam&& arg)
				lnoexcept(is_nothrow_constructible<_type, _tParam&&>())
				: boxed_value<_type>(lforward(arg))
			{}
			using boxed_value<_type>::boxed_value;
			//@}
			//! \since build 1.4
			//@{
			value_holder(const value_holder&) = default;
			value_holder(value_holder&&) = default;

			value_holder&
				operator=(const value_holder&) = default;
			value_holder&
				operator=(value_holder&&) = default;
			//@}

			value_holder*
				clone() const override
			{
				return new value_holder(this->value);
			}

			//! \since build 1.3
			void*
				get() const override
			{
				return addressof(this->value);
			}

			//! \since build 1.4
			const type_info&
				type() const lnothrow override
			{
				return leo::type_id<_type>();
			}
		};


		/*!
		\brief 指针类型动态泛型持有者。
		\tparam _type 对象类型。
		\tparam _tPointer 智能指针类型。
		\pre _tPointer 具有 _type 对象所有权。
		\pre 静态断言： <tt>is_object<_type>()</tt> 。
		\since build 1.4
		*/
		template<typename _type, class _tPointer = std::unique_ptr<_type>>
		class pointer_holder : public holder
		{
			static_assert(is_object<_type>::value, "Invalid type found.");

		public:
			using value_type = _type;
			using holder_pointer = _tPointer;
			using pointer = typename holder_pointer::pointer;

		protected:
			holder_pointer p_held;

		public:
			//! \brief 取得所有权。
			pointer_holder(pointer value)
				: p_held(value)
			{}
			//! \since build 1.3
			//@{
			pointer_holder(const pointer_holder& h)
				: pointer_holder(h.p_held ? new value_type(*h.p_held) : nullptr)
			{}
			pointer_holder(pointer_holder&&) = default;
			//@}

			pointer_holder&
				operator=(const pointer_holder&) = default;
			pointer_holder&
				operator=(pointer_holder&&) = default;

			pointer_holder*
				clone() const override
			{
				return new pointer_holder(*this);
			}

			void*
				get() const override
			{
				return p_held.get();
			}

			const type_info&
				type() const lnothrow override
			{
				return p_held ? leo::type_id<_type>() : leo::type_id<void>();
			}
		};


		//! \since build 1.3
		using op_code = std::uint32_t;

		//! \since build 1.3
		enum base_op : op_code
		{
			//! \since build 1.3
			no_op,
			get_type,
			get_ptr,
			clone,
			destroy,
			get_holder_type,
			get_holder_ptr,
			//! \since build 1,3
			end_base_op
		};


		//! \since build 1.3
		using any_storage
			= standard_layout_storage<aligned_storage_t<sizeof(void*), lalignof(void*)>>;
		//! \since build 1.3
		using any_manager = void(*)(any_storage&, any_storage&, op_code);

		/*!
		\brief 使用指定处理器初始化存储。
		\since build 1.4
		*/
		template<class _tHandler, typename... _tParams>
		any_manager
			construct(any_storage& storage, _tParams&&... args)
		{
			_tHandler::init(storage, lforward(args)...);
			return _tHandler::manage;
		}


		/*!
		\brief 使用持有者标记。
		\since build 1.4
		*/
		lconstexpr const struct use_holder_t {} use_holder{};


		/*!
		\brief 动态泛型对象处理器。
		\since build 1.4
		*/
		template<typename _type,
			bool _bStoredLocally = and_<is_nothrow_move_constructible<_type>,
			is_aligned_storable<any_storage, _type>>::value>
			class value_handler
		{
		public:
			//! \since build 1.3
			//@{
			using value_type = _type;
			using local_storage = bool_<_bStoredLocally>;
			//@}

			//! \since build 1.4
			//@{
			static void
				copy(any_storage& d, const any_storage& s)
			{
				try_init(is_copy_constructible<value_type>(), local_storage(), d,
					get_reference(s));
			}

			static void
				dispose(any_storage& d) lnothrowv
			{
				dispose_impl(local_storage(), d);
			}

		private:
			static void
				dispose_impl(false_, any_storage& d) lnothrowv
			{
				delete d.access<value_type*>();
			}
			static void
				dispose_impl(true_, any_storage& d) lnothrowv
			{
				d.access<value_type>().~value_type();
			}
			//@}

		public:
			//! \since build 1.4
			//@{
			static value_type*
				get_pointer(any_storage& s)
			{
				return get_pointer_impl(local_storage(), s);
			}
			static const value_type*
				get_pointer(const any_storage& s)
			{
				return get_pointer_impl(local_storage(), s);
			}

		private:
			static value_type*
				get_pointer_impl(false_, any_storage& s)
			{
				return s.access<value_type*>();
			}
			static const value_type*
				get_pointer_impl(false_, const any_storage& s)
			{
				return s.access<const value_type*>();
			}
			static value_type*
				get_pointer_impl(true_, any_storage& s)
			{
				return std::addressof(get_reference_impl(true_(), s));
			}
			static const value_type*
				get_pointer_impl(true_, const any_storage& s)
			{
				return std::addressof(get_reference_impl(true_(), s));
			}

		public:
			static value_type&
				get_reference(any_storage& s)
			{
				return get_reference_impl(local_storage(), s);
			}
			static const value_type&
				get_reference(const any_storage& s)
			{
				return get_reference_impl(local_storage(), s);
			}

		private:
			static value_type&
				get_reference_impl(false_, any_storage& s)
			{
				const auto p(get_pointer_impl(false_(), s));

				lassume(p);
				return *p;
			}
			static const value_type&
				get_reference_impl(false_, const any_storage& s)
			{
				const auto p(get_pointer_impl(false_(), s));

				lassume(p);
				return *p;
			}
			static value_type&
				get_reference_impl(true_, any_storage& s)
			{
				return s.access<value_type>();
			}
			static const value_type&
				get_reference_impl(true_, const any_storage& s)
			{
				return s.access<const value_type>();
			}
			//@}

		public:
			//! \since build 1.4
			//@{
			template<typename... _tParams>
			static void
				init(any_storage& d, _tParams&&... args)
			{
				init_impl(local_storage(), d,lforward(args)...);
			}

		private:
			template<typename... _tParams>
			static LB_ATTR(always_inline) void
				init_impl(false_, any_storage& d, _tParams&&... args)
			{
				d.construct<value_type*>(new value_type(lforward(args)...));
			}
			template<typename... _tParams>
			static LB_ATTR(always_inline) void
				init_impl(true_, any_storage& d, _tParams&&... args)
			{
				d.construct<value_type>(lforward(args)...);
			}
			//@}

		public:
			//! \since build 1.4
			static void
				manage(any_storage& d, any_storage& s, op_code op)
			{
				switch (op)
				{
				case get_type:
					d = &leo::type_id<value_type>();
					break;
				case get_ptr:
					d = static_cast<void*>(get_pointer(s));
					break;
				case clone:
					copy(d, s);
					break;
				case destroy:
					dispose(d);
					break;
				case get_holder_type:
					d = &leo::type_id<void>();
					break;
				case get_holder_ptr:
					d = static_cast<holder*>(nullptr);
				}
			}

		private:
			//! \since build 1.4
			//@{
			template<typename... _tParams>
			LB_NORETURN static LB_ATTR(always_inline) void
				try_init(false_, _tParams&&...)
			{
				throw_invalid_construction();
			}
			template<class _bInPlace, typename... _tParams>
			static LB_ATTR(always_inline) void
				try_init(true_, _bInPlace b, any_storage& d, _tParams&&... args)
			{
				init_impl(b, d, lforward(args)...);
			}
			//@}
		};


		/*!
		\brief 动态泛型引用处理器。
		\since build 1.3
		*/
		template<typename _type>
		class ref_handler : public value_handler<_type*>
		{
		public:
			using value_type = _type;
			using base = value_handler<value_type*>;

			//! \since build 1.4
			static value_type*
				get_pointer(any_storage& s)
			{
				return base::get_reference(s);
			}

			//! \since build 1.4
			static value_type&
				get_reference(any_storage& s)
			{
				lassume(get_pointer(s));
				return *get_pointer(s);
			}

			//! \since build 678
			template<typename _tWrapper,
				limpl(typename = enable_if_t<is_reference_wrapper<_tWrapper>::value>)>
				static auto
				init(any_storage& d, _tWrapper x)
				-> decltype(base::init(d, addressof(x.get())))
			{
				base::init(d, addressof(x.get()));
			}

			//! \since build 1.4
			static void
				manage(any_storage& d, any_storage& s, op_code op)
			{
				switch (op)
				{
				case get_type:
					d = &leo::type_id<value_type>();
					break;
				case get_ptr:
					d =static_cast<void*>(get_pointer(s));
					break;
				default:
					base::manage(d, s, op);
				}
			}
		};


		/*!
		\brief 动态泛型持有者处理器。
		\since build 1.3
		*/
		template<typename _tHolder>
		class holder_handler : public value_handler<_tHolder>
		{
			static_assert(is_convertible<_tHolder&, holder&>::value,
				"Invalid holder type found.");

		public:
			using value_type = typename _tHolder::value_type;
			using base = value_handler<_tHolder>;

			//! \since build 1.4
			static _tHolder*
				get_holder_pointer(any_storage& s)
			{
				return base::get_pointer(s);
			}

			static value_type*
				get_pointer(any_storage& s)
			{
				const auto p(get_holder_pointer(s));

				lassume(p);
				return static_cast<value_type*>(p->_tHolder::get());
			}

		private:
			//! \since build 1.4
			static void
				init(false_, any_storage& d, std::unique_ptr<_tHolder> p)
			{
				d.construct<value_type*>(p.release());
			}
			//! \since build 1.4
			static void
				init(true_, any_storage& d, std::unique_ptr<_tHolder> p)
			{
				d.construct<_tHolder>(std::move(*p));
			}

		public:
			//! \since build 1.3
			static void
				init(any_storage& d, std::unique_ptr<_tHolder> p)
			{
				init(typename base::local_storage(), d, std::move(p));
			}
			//! \since build 1.4
			using base::init;

			//! \since build 1.4
			static void
				manage(any_storage& d, any_storage& s, op_code op)
			{
				switch (op)
				{
				case get_type:
					d = &type_id<value_type>();
					break;
				case get_ptr:
					d = get_pointer(s);
					break;
				case get_holder_type:
					d = &type_id<_tHolder>();
					break;
				case get_holder_ptr:
					d = static_cast<holder*>(get_holder_pointer(s));
					break;
				default:
					base::manage(d, s, op);
				}
			}
		};

	} // namespace any_ops;


	/*!
	\ingroup unary_type_traits
	\brief 判断类型是否可以作为 any 引用转换的目标。
	\sa any_cast
	\since build 1.4
	*/
	template<typename _type>
	using is_any_cast_dest = or_<is_reference<_type>, is_copy_constructible<_type>>;

	/*!
	\ingroup exceptions
	\brief 动态泛型转换失败异常。
	\note 基本接口和语义同 boost::bad_any_cast 。
	\note 非标准库提案扩展：提供标识转换失败的源和目标类型。
	\sa any_cast
	\see WG21 N4582 20.6.2[any.bad_any_cast] 。
	\since build 1.4
	*/
	class LB_API bad_any_cast : public std::bad_cast
	{
	private:
		lref<const type_info> from_ti, to_ti;

	public:
		//! \since build 1.3
		//@{
		bad_any_cast()
			: std::bad_cast(),
			from_ti(type_id<void>()), to_ti(type_id<void>())
		{}
		//! \since build 1.4
		bad_any_cast(const type_info& from_, const type_info& to_)
			: std::bad_cast(),
			from_ti(from_), to_ti(to_)
		{}
		//! \since build 1.4
		bad_any_cast(const bad_any_cast&) = default;
		/*!
		\brief 虚析构：类定义外默认实现。
		\since build 1.4
		*/
		~bad_any_cast() override;

		//! \note LBase 扩展。
		//@{
		LB_ATTR_returns_nonnull const char*
			from() const lnothrow;

		//! \since build 1.4
		const type_info&
			from_type() const lnothrow
		{
			return from_ti.get();
		}

		LB_ATTR_returns_nonnull const char*
			to() const lnothrow;

		//! \since build 1.4
		const type_info&
			to_type() const lnothrow
		{
			return to_ti.get();
		}
		//@}
		//@}

		virtual LB_ATTR_returns_nonnull const char*
			what() const lnothrow override;
	};


	//! \since build 1.4
	//@{
	namespace details
	{
		struct any_base
		{
			mutable any_ops::any_storage storage{};
			any_ops::any_manager manager{};

			any_base() = default;
			template<class _tHandler, typename... _tParams>
			inline
				any_base(any_ops::with_handler_t<_tHandler>, _tParams&&... args)
				: manager(any_ops::construct<_tHandler>(storage, lforward(args)...))
			{}


		protected:
			any_base(const any_base& a)
				: manager(a.manager)
			{}
			~any_base() = default;
		public:
			LB_API any_ops::any_storage&
				call(any_ops::any_storage&, any_ops::op_code) const;

			LB_API void
				clear() lnothrowv;

			void
				copy(const any_base&);

			void
				destroy() lnothrowv;

			bool
				has_value() const lnothrow
			{
				return manager;
			}

			//! \pre 断言：\c manager 。
			//@{
			LB_API void*
				get() const lnothrowv;

			LB_API any_ops::holder*
				get_holder() const;

			//! \since build 1.4
			any_ops::any_storage&
				get_storage() const
			{
				return storage;
			}

			LB_API void
				swap(any_base&) lnothrow;

			template<typename _type>
			_type*
				target() lnothrowv
			{
				return type() == type_id<_type>() ? static_cast<_type*>(get())
					: nullptr;
			}
			template<typename _type>
			const _type*
				target() const lnothrowv
			{
				return type() == type_id<_type>()
					? static_cast<const _type*>(get()) : nullptr;
			}

			LB_API const type_info&
				type() const lnothrowv
			{
				return *unchecked_access<const type_info*>(any_ops::get_type);
			}

			//! \since build 1.4
			template<typename _type, typename... _tParams>
			inline _type
				unchecked_access(any_ops::op_code op, _tParams&&... args) const
			{
				any_ops::any_storage t;

				const auto gd(t.pun<_type>(lforward(args)...));

				return unchecked_access<_type>(t, op);
			}
			template<typename _type>
			inline _type
				unchecked_access(default_init_t, any_ops::op_code op) const
			{
				any_ops::any_storage t;
				const auto gd(t.pun_default<_type>());

				return unchecked_access<_type>(t, op);
			}
			//! \since build 1.4
			template<typename _type>
			inline _type
				unchecked_access(any_ops::any_storage& t, any_ops::op_code op) const
			{
				return call(t, op).access<_type>();
			}
			//@}
		};


		template<class _tAny>
		struct any_emplace
		{
			template<typename _type, typename... _tParams>
			void
				emplace(_tParams&&... args)
			{
				emplace_with_handler<any_ops::value_handler<decay_t<_type>>>(
					lforward(args)...);
			}
			template<typename _type, typename _tOther, typename... _tParams>
			void
				emplace(std::initializer_list<_tOther> il, _tParams&&... args)
			{
				emplace_with_handler<any_ops::value_handler<decay_t<_type>>>(
					il, lforward(args)...);
			}
			template<typename _tHolder, typename... _tParams>
			void
				emplace(any_ops::use_holder_t, _tParams&&... args)
			{
				emplace_with_handler<any_ops::holder_handler<decay_t<_tHolder>>>(
					lforward(args)...);
			}

			template<typename _tHandler, typename... _tParams>
			void
				emplace_with_handler(_tParams&&... args)
			{
				auto& a(static_cast<_tAny&>(*this));

				a.reset();
				a.manager = any_ops::construct<decay_t<_tHandler>>(a.storage,
					lforward(args)...);
			}
		};

	} // namespace details;
	//@}

	/*
	\brief 基于类型擦除的动态泛型对象
	\note  值语义,基于接口和语义同 std::experimental::any 提议
	\warning 非虚析构
	\see WG21 N4582 20.6.3[any.class] 。
	\see http://www.boost.org/doc/libs/1_53_0/doc/html/any/reference.html#any.ValueType
	*/
	class LB_API any : private details::any_base, private details::any_emplace<any>
	{
		friend details::any_emplace<any>;
	public:
		//! \post \c this->empty() 。
		any() lnothrow = default;
		//! \since build 1.4
		template<typename _type, limpl(typename = exclude_self_t<any, _type>,
			typename = enable_if_t<!is_reference_wrapper<decay_t<_type>>::value>)>
			inline
			any(_type&& x)
			: any(any_ops::with_handler_t<
				any_ops::value_handler<decay_t<_type>>>(), lforward(x))
		{}
		//! \note LBase 扩展。
		//@{
		//! \since build 1.4
		template<typename _type, limpl(typename
			= enable_if_t<is_reference_wrapper<decay_t<_type>>::value>)>
			inline
			any(_type&& x)
			: any(any_ops::with_handler_t<
				any_ops::ref_handler<remove_reference_t<decay_unwrap_t<_type>>>>(), x)
		{}
		//! \since build 1.4
		template<typename _type, typename... _tParams>
		inline
			any(in_place_type_t<_type>, _tParams&&... args)
			: any(any_ops::with_handler_t<
				any_ops::value_handler<_type>>(), lforward(args)...)
		{}
		/*!
		\brief 构造：使用指定持有者。
		\since build 1.4
		*/
		//@{
		template<typename _tHolder>
		inline
			any(any_ops::use_holder_t, std::unique_ptr<_tHolder> p)
			: any(any_ops::with_handler_t<
				any_ops::holder_handler<_tHolder>>(), std::move(p))
		{}
		template<typename _tHolder>
		inline
			any(any_ops::use_holder_t, _tHolder&& h)
			: any(any_ops::with_handler_t<
				any_ops::holder_handler<decay_t<_tHolder>>>(), lforward(h))
		{}
		template<typename _tHolder, typename... _tParams>
		inline
			any(any_ops::use_holder_t, in_place_type_t<_tHolder>,
				_tParams&&... args)
			: any(any_ops::with_handler_t<any_ops::holder_handler<_tHolder>>(),
				lforward(args)...)
		{}
		//@}
		template<typename _type>
		inline
			any(_type&& x, any_ops::use_holder_t)
			: any(any_ops::with_handler_t<any_ops::holder_handler<
				any_ops::value_holder<decay_t<_type>>>>(), lforward(x))
		{}
		//! \since build 1.4
		template<class _tHandler, typename... _tParams>
		inline
			any(any_ops::with_handler_t<_tHandler> t, _tParams&&... args)
			: any_base(t, lforward(args)...)
		{}
		//@}
		any(const any&);
		any(any&& a) lnothrow
			: any()
		{
			a.swap(*this);
		}
		//! \since build 1.3
		~any();

		//! \since build 1.4
		template<typename _type, limpl(typename = exclude_self_t<any, _type>)>
		any&
			operator=(_type&& x)
		{
			any(lforward(x)).swap(*this);
			return *this;
		}
		/*!
		\brief 复制赋值：使用复制和交换。
		\since build 1.3
		*/
		any&
			operator=(const any& a)
		{
			any(a).swap(*this);
			return *this;
		}
		/*!
		\brief 转移赋值：使用复制和交换。
		\since build 1.3
		*/
		any&
			operator=(any&& a) lnothrow
		{
			any(std::move(a)).swap(*this);
			return *this;
		}

		//! \since build 1.4
		using any_base::has_value;

		//! \note LBase 扩展。
		//@{
		//! \since build 352
		void*
			get() const lnothrow
		{
			return manager ? unchecked_get() : nullptr;
		}

		any_ops::holder*
			get_holder() const
		{
			return manager ? unchecked_get_holder() : nullptr;
		}
		//@}

	protected:
		/*!
		\note LBase 扩展。
		\since build 687
		*/
		//@{
		using any_base::get_storage;

		using any_base::call;
		//@}

	public:
		void
			reset() lnothrow;

		/*!
		\note LBase 扩展。
		\since build 1.4
		*/
		//@{
		using any_emplace<any>::emplace;

		using any_emplace<any>::emplace_with_handler;
		//@}

		void
			swap(any& a) lnothrow
		{
			any_base::swap(a);
		}

		/*!
		\brief 取目标指针。
		\return 若存储目标类型和模板参数相同则为指向存储对象的指针值，否则为空指针值。
		\note LBase 扩展。
		\since build 1.4
		*/
		//@{
		template<typename _type>
		_type*
			target() lnothrow
		{
			return manager ? any_base::template target<_type>() : nullptr;
		}
		template<typename _type>
		const _type*
			target() const lnothrow
		{
			return manager ? any_base::template target<_type>() : nullptr;
		}
		//@}

		//! \since build 1.4
		const type_info&
			type() const lnothrow
		{
			return manager ? unchecked_type() : type_id<void>();
		}

		/*!
		\note LBase 扩展。
		\pre 断言：\c !empty() 。
		\since build 1.4
		*/
		//@{
	protected:
		//! \since build 1.4
		using any_base::unchecked_access;

	public:
		//! \brief 取包含对象的指针。
		void*
			unchecked_get() const lnothrowv
		{
			return any_base::get();
		}

		//! \brief 取持有者指针。
		any_ops::holder*
			unchecked_get_holder() const
		{
			return any_base::get_holder();
		}

		/*!
		\brief 取包含对象的类型。
		\since build 1.4
		*/
		const type_info&
			unchecked_type() const lnothrowv
		{
			return any_base::type();
		}
		//@}
	};

	inline void
		swap(any& x, any& y) lnothrow
	{
		x.swap(y);
	}


	/*!
	\ingroup helper_functions
	\brief 创建 any 对象。
	\see WG21 P0032R3 。
	*/
	//@{
	template<typename _type, typename... _tParams>
	any
		make_any(_tParams&&... args)
	{
		return any(in_place<_type>, lforward(args)...);
	}
	template<typename _type, typename _tOther, typename... _tParams>
	any
		make_any(std::initializer_list<_tOther> il, _tParams&&... args)
	{
		return any(in_place<_type>, il, lforward(args)...);
	}
	//@}

	/*!
	\brief 动态泛型转换。
	\return 当 <tt>p
	&& p->type() == type_id<remove_pointer_t<_tPointer>>()</tt> 时
	为指向对象的指针，否则为空指针。
	\note 语义同 \c boost::any_cast 。
	\relates any
	\since build 1.4
	\todo 检验特定环境（如使用动态库时）比较 std::type_info::name() 的必要性。
	*/
	//@{
	//@{
	template<typename _type>
	inline _type*
		any_cast(any* p) lnothrow
	{
		return p ? p->target<_type>() : nullptr;
	}
	template<typename _type>
	inline const _type*
		any_cast(const any* p) lnothrow
	{
		return p ? p->target<_type>() : nullptr;
	}
	//@}
	/*!
	\throw bad_any_cast 当 <tt>x.type()
	!= type_id<remove_reference_t<_tValue>>()</tt> 。
	*/
	//@{
	template<typename _tValue>
	_tValue
		any_cast(any& x)
	{
		static_assert(is_any_cast_dest<_tValue>(),
			"Invalid cast destination type found.");

		if (const auto p = x.template target<remove_reference_t<_tValue>>())
			return static_cast<_tValue>(*p);
		throw bad_any_cast(x.type(), leo::type_id<_tValue>());
	}
	template<typename _tValue>
	_tValue
		any_cast(const any& x)
	{
		static_assert(is_any_cast_dest<_tValue>(),
			"Invalid cast destination type found.");

		if (const auto p = x.template target<const remove_reference_t<_tValue>>())
			return static_cast<_tValue>(*p);
		throw bad_any_cast(x.type(), leo::type_id<_tValue>());
	}
	//! \since build 1.4
	template<typename _tValue>
	_tValue
		any_cast(any&& x)
	{
		static_assert(is_any_cast_dest<_tValue>(),
			"Invalid cast destination type found.");

		if (const auto p = x.template target<remove_reference_t<_tValue>>())
			return static_cast<_tValue>(*p);
		throw bad_any_cast(x.type(), leo::type_id<_tValue>());
	}
	//@}
	//@}

	//! \note LBase 扩展。
	//@{
	/*!
	\brief 未检查的动态泛型转换。
	\note 对非空对象语义同非公开接口 \c boost::unsafe_any_cast 。
	\since build 1.4
	*/
	//@{
	/*!
	\pre 断言： <tt>p && !p->empty()
	&& p->unchecked_type() == type_id<_type>()</tt> 。
	*/
	template<typename _type>
	inline _type*
		unchecked_any_cast(any* p) lnothrowv
	{
		lconstraint(p && p->has_value()
			&& p->unchecked_type() == leo::type_id<_type>());
		return static_cast<_type*>(p->unchecked_get());
	}

	/*!
	\pre 断言： <tt>p && !p->empty()
	&& p->unchecked_type() == type_id<const _type>()</tt> 。
	*/
	template<typename _type>
	inline const _type*
		unchecked_any_cast(const any* p) lnothrowv
	{
		lconstraint(p && p->has_value()
			&& p->unchecked_type() == leo::type_id<const _type>());
		return static_cast<const _type*>(p->unchecked_get());
	}
	//@}

	/*!
	\brief 非安全动态泛型转换。
	\note 语义同非公开接口 \c boost::unsafe_any_cast 。
	\since build 1.4
	*/
	//@{
	//! \pre 断言： <tt>p && p->type() == type_id<_type>()</tt> 。
	template<typename _type>
	inline _type*
		unsafe_any_cast(any* p) lnothrowv
	{
		lconstraint(p && p->type() == leo::type_id<_type>());
		return static_cast<_type*>(p->get());
	}

	//! \pre 断言： <tt>p && p->type() == type_id<const _type>()</tt> 。
	template<typename _type>
	inline const _type*
		unsafe_any_cast(const any* p) lnothrowv
	{
		lconstraint(p && p->type() == leo::type_id<const _type>());
		return static_cast<const _type*>(p->get());
	}
	//@}
	//@}


	/*!
	\note LBase 扩展。
	\sa void_ref
	*/
	class LB_API void_ref_any
	{
	private:
		mutable any data;

	public:
		void_ref_any() = default;
		template<typename _type,
			limpl(typename = exclude_self_t<void_ref_any, _type>)>
			void_ref_any(_type&& x)
			: void_ref_any(lforward(x), is_convertible<_type&&, void_ref>())
		{}

	private:
		template<typename _type>
		void_ref_any(_type&& x, true_)
			: data(void_ref(lforward(x)))
		{}
		template<typename _type>
		void_ref_any(_type&& x, false_)
			: data(lforward(x))
		{}

	public:
		void_ref_any(const void_ref_any&) = default;
		void_ref_any(void_ref_any&&) = default;

		void_ref_any&
			operator=(const void_ref_any&) = default;
		void_ref_any&
			operator=(void_ref_any&&) = default;

		//! \pre 间接断言：参数指定初始化对应的类型。
		template<typename _type,
			limpl(typename = exclude_self_t<void_ref_any, _type>)>
			LB_PURE
			operator _type() const
		{
			if (data.type() == leo::type_id<void_ref>())
				return *leo::unchecked_any_cast<void_ref>(&data);
			return std::move(*leo::unchecked_any_cast<_type>(&data));
		}
	};
}
#endif