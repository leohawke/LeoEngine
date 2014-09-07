#ifndef IndePlatform_any_h
#define IndePlatform_any_h

#include "utility.hpp"
#include <memory> //std::addressof,std::unique_ptr
#include <typeinfo>//typeid,std::bad_cast

namespace leo
{
	union non_aggregate_pod
	{
		void* object_ptr;
		const void* const_object_ptr;
		volatile void* volatile_object_ptr;
		const volatile void* const_volatile_object_ptr;
		void(*function_ptr)();
		int(non_aggregate_pod::*member_object_pointer);
		void(non_aggregate_pod::*member_function_pointer)();
	};

	template<typename _type, typename _tDst>
	struct is_aligned_storable
	{
		static lconstexpr bool value = sizeof(_type) <= sizeof(_tDst)
			&& lalignof(_type) <= lalignof(_tDst)
			&& lalignof(_tDst) % lalignof(_type) == 0;
	};

	template<typename _tPOD = aligned_storage_t<sizeof(void*)>>
	union pod_storage
	{
		static_assert(is_pod<_tPOD>::value, "Non-POD underlying type found.");

		using underlying = _tPOD;

		underlying object;
		stdex::byte data[sizeof(underlying)];

		pod_storage() = default;
		pod_storage(const pod_storage&) = default;
		template<typename _type,
			limpl(typename = exclude_self_ctor_t<pod_storage, _type>)>
			inline
			pod_storage(_type&& x)
		{
			new(access()) decay_t<_type>(lforward(x));
		}

#ifdef LB_IMPL_MSCPP
		pod_storage(pod_storage&& p)
			:object(std::move(p.object))
		{}
#else
		pod_storage(pod_storage&& p) = default;
#endif
		pod_storage&
			operator=(const pod_storage&) = default;
		
		template<typename _type,
			limpl(typename = exclude_self_ctor_t<pod_storage, _type>)>
			inline pod_storage&
			operator=(_type&& x)
		{
			assign(lforward(x));
			return *this;
		}

		LB_PURE void*
			access()
		{
			return &data[0];
		}
		lconstfn LB_PURE const void*
			access() const
		{
			return &data[0];
		}
		template<typename _type>
		LB_PURE _type&
			access()
		{
			static_assert(is_aligned_storable<_type, pod_storage>::value,
				"Invalid type found.");

			return *static_cast<_type*>(access());
		}
		template<typename _type>
		lconstfn LB_PURE const _type&
			access() const
		{
			static_assert(is_aligned_storable<_type, pod_storage>::value,
				"Invalid type found.");

			return *static_cast<const _type*>(access());
		}

		template<typename _type>
		inline void
			assign(_type&& x)
		{
			access<decay_t<_type>>() = lforward(x);
		}

		void
			swap(pod_storage& a) lnothrow
		{
			underlying b = std::move(a.object);
			a.object = std::move(object);
			object = std::move(b);
		}
	};

	template<typename T>
	inline void
		swap(pod_storage<T>& x, pod_storage<T>& y) lnothrow
	{
		x.swap(y);
	}

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

	namespace any_ops
	{
		class LB_API holder : public cloneable
		{
		public:
			virtual ~holder()
			{}

			virtual void*
			get() const = 0;

			virtual holder*
			clone() const override = 0;

			virtual const std::type_info&
			type() const lnothrow = 0;
		};

		template<typename _type>
		class value_holder : public holder
		{
			static_assert(is_object<_type>::value, "Non-object type found.");
			static_assert(!(is_const<_type>::value || is_volatile<_type>::value),
				"Cv-qualified type found.");

		public:
			using value_type = _type;

		protected:
			mutable _type held;

		public:
			value_holder(const _type& value)
				: held(value)
			{}

			value_holder(_type&& value) lnoexcept(lnoexcept(_type(std::move(value))))
				: held(std::move(value))
			{}

			value_holder*
				clone() const override
			{
				return new value_holder(held);
			}

			void*
				get() const override
			{
				return std::addressof(held);
			}

			const std::type_info&
				type() const lnothrow override
			{
				return typeid(_type);
			}
		};

		template<typename _type>
		class pointer_holder : public holder
		{
			static_assert(is_object<_type>::value, "Invalid type found.");

		public:
			using value_type = _type;

		protected:
			_type* p_held;

		public:
			pointer_holder(_type* value)
				: p_held(value)
			{}
			pointer_holder(const pointer_holder& h)
				: pointer_holder(h.p_held ? new _type(*h.p_held) : nullptr)
			{}
			pointer_holder(pointer_holder&& h)
				: p_held(h.p_held)
			{
				h.p_held = {};
			}
			virtual
				~pointer_holder()
			{
				delete p_held;
			}

			pointer_holder*
				clone() const override
			{
				return new pointer_holder(*this);
			}

			void*
				get() const override
			{
				return p_held;
			}

			const std::type_info&
				type() const lnothrow override
			{
				return p_held ? typeid(_type) : typeid(void);
			}
		};

		using op_code = std::uint32_t;

		enum base_op : op_code
		{
			no_op,
			get_type,
			get_ptr,
			clone,
			destroy,
			get_holder_type,
			get_holder_ptr,
			end_base_op
		};

		using any_storage = pod_storage < non_aggregate_pod > ;
		using any_manager = void(*)(any_storage&, const any_storage&, op_code);

		struct  holder_tag
		{};

		namespace details
		{
			template<typename _type,bool _bStoredLocally>
			struct value_handler_op
			{
				static const _type*
					get_pointer(const any_storage& s)
				{
					return s.access<const _type*>();
				}
			};

			template<typename _type>
			struct value_handler_op<_type, true>
			{
				static const _type*
					get_pointer(const any_storage& s)
				{
					return std::addressof(s.access<_type>());
				}
			};
		}

		template<typename _type,
			bool _bStoredLocally = is_aligned_storable<_type, any_storage>::value>
		class value_handler
		{
		public:
			using value_type = _type;
			using local_storage = integral_constant<bool, _bStoredLocally>;

			static inline value_type*
				get_pointer(const any_storage& s)
			{
				return const_cast<_type*>(
					details::value_handler_op<_type, _bStoredLocally>::get_pointer(s));
			}

			static value_type&
				get_reference(const any_storage& s)
			{
				lassume(get_pointer(s));
				return *get_pointer(s);
			}

			static void
				copy(any_storage& d, const any_storage& s, true_type)
			{
				new(d.access()) value_type(s.access<value_type>());
			}
			static void
				copy(any_storage& d, const any_storage& s, false_type)
			{
				d = new value_type(*s.access<value_type*>());
			}

			static void
				uninit(any_storage& d, true_type)
			{
				d.access<value_type>().~value_type();
			}
			static void
				uninit(any_storage& d, false_type)
			{
				delete d.access<value_type*>();
			}

			template<typename _tValue>
			static void
				init(any_storage& d, _tValue&& x)
			{
				init_impl(d, lforward(x), local_storage());
			}

		private:
			template<typename _tValue>
			static void
				init_impl(any_storage& d, _tValue&& x, true_type)
			{
				new(d.access()) value_type(lforward(x));
			}
			template<typename _tValue>
			static void
				init_impl(any_storage& d, _tValue&& x, false_type)
			{
				d = new value_type(lforward(x));
			}

		public:
			static void
				manage(any_storage& d, const any_storage& s, op_code op)
			{
				switch (op)
				{
				case get_type:
					d = &typeid(value_type);
					break;
				case get_ptr:
					d = get_pointer(s);
					break;
				case clone:
					copy(d, s, local_storage());
					break;
				case destroy:
					uninit(d, local_storage());
					break;
				case get_holder_type:
					d = &typeid(void);
					break;
				case get_holder_ptr:
					d = static_cast<holder*>(nullptr);
				}
			}
		};

		template<typename _type>
		class ref_handler : public value_handler<_type*>
		{
		public:
			using value_type = _type;
			using base = value_handler<value_type*>;

			static value_type*
				get_pointer(const any_storage& s)
			{
				return base::get_reference(s);
			}

			static value_type&
				get_reference(const any_storage& s)
			{
				lassume(get_pointer(s));
				return *get_pointer(s);
			}

			static void
				init(any_storage& d, std::reference_wrapper<value_type> x)
			{
				base::init(d, std::addressof(x.get()));
			}

			static void
				manage(any_storage& d, const any_storage& s, op_code op)
			{
				switch (op)
				{
				case get_type:
					d = &typeid(value_type);
					break;
				case get_ptr:
					d = get_pointer(s);
					break;
				default:
					base::manage(d, s, op);
				}
			}
		};

		template<typename _tHolder>
		class holder_handler : public value_handler<_tHolder>
		{
			static_assert(is_convertible<_tHolder&, holder&>::value,
				"Invalid holder type found.");

		public:
			using value_type = typename _tHolder::value_type;
			using base = value_handler<_tHolder>;
			using local_storage = typename base::local_storage;

			static value_type*
				get_pointer(const any_storage& s)
			{
				return static_cast<value_type*>(base::get_pointer(s)->_tHolder::get());
			}

		private:
			static void
				init(any_storage& d, std::unique_ptr<_tHolder> p, true_type)
			{
				new(d.access()) _tHolder(std::move(*p));
			}
			static void
				init(any_storage& d, std::unique_ptr<_tHolder> p, false_type)
			{
				d = p.release();
			}

		public:
			static void
				init(any_storage& d, std::unique_ptr<_tHolder> p)
			{
				init(d, std::move(p), local_storage());
			}
			static void
				init(any_storage& d, _tHolder&& x)
			{
				base::init(d, std::move(x));
			}
			template<typename... _tParams>
			static void
				init(any_storage& d, _tParams&&... args)
			{
				init(d, _tHolder(lforward(args)...));
			}

			static void
				manage(any_storage& d, const any_storage& s, op_code op)
			{
				switch (op)
				{
				case get_type:
					d = &typeid(value_type);
					break;
				case get_ptr:
					d = get_pointer(s);
					break;
				case clone:
					base::copy(d, s, local_storage());
					break;
				case destroy:
					base::uninit(d, local_storage());
					break;
				case get_holder_type:
					d = &typeid(_tHolder);
					break;
				case get_holder_ptr:
					d = static_cast<holder*>(base::get_pointer(s));
				}
			}
		};
	}// namespace any_ops;

	/*
	基于类型擦除的动态泛型对象
	值语义,基于接口和语义同 std::any 提议
	warning 非虚析构
	see http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2013/n3508.html#synopsis
	see http://www.boost.org/doc/libs/1_53_0/doc/html/any/reference.html#any.ValueType
	*/
	class LB_API any
	{
	protected:
		any_ops::any_storage storage;
		any_ops::any_manager manager;

	public:
		lconstfn
			any() lnothrow
			: storage(), manager()
		{}
		template<typename _type, limpl(typename = exclude_self_ctor_t<any, _type>)>
		any(_type&& x)
			: manager(any_ops::value_handler<remove_reference_t<_type>>::manage)
		{
			any_ops::value_handler<typename remove_rcv<_type>::type>::init(storage,
				lforward(x));
		}
		template<typename _type>
		any(std::reference_wrapper<_type> x)
			: manager(any_ops::ref_handler<_type>::manage)
		{
			any_ops::ref_handler<_type>::init(storage, x);
		}
		template<typename _tHolder>
		any(any_ops::holder_tag, std::unique_ptr<_tHolder> p)
			: manager(any_ops::holder_handler<_tHolder>::manage)
		{
			any_ops::holder_handler<_tHolder>::init(storage, std::move(p));
		}
		template<typename _type>
		any(_type&& x, any_ops::holder_tag)
			: manager(any_ops::holder_handler<any_ops::value_holder<typename
			remove_rcv<_type>::type >> ::manage)
		{
			any_ops::holder_handler<any_ops::value_holder<
				remove_cv_t<_type >> >::init(storage, lforward(x));
		}
		any(const any&);
		any(any&& a) lnothrow
			: any()
		{
			a.swap(*this);
		}
		~any();

		template<typename _type>
		any&
			operator=(const _type& x)
		{
			any(x).swap(*this);
			return *this;
		}
		any&
			operator=(const any& a)
		{
			any(a).swap(*this);
			return *this;
		}
		any&
			operator=(any&& a) lnothrow
		{
			any(std::move(a)).swap(*this);
			return *this;
		}

			bool
			operator!() const lnothrow
		{
			return empty();
		}

			explicit
			operator bool() const lnothrow
		{
			return !empty();
		}

			bool
			empty() const lnothrow
		{
			return !manager;
		}

			void*
			get() const lnothrow;

		any_ops::holder*
			get_holder() const;

		void
			clear() lnothrow;

		void
			swap(any& a) lnothrow;

		template<typename _type>
		_type*
			target() lnothrow
		{
			return type() == typeid(_type) ? static_cast<_type*>(get()) : nullptr;
		}
			template<typename _type>
		const _type*
			target() const lnothrow
		{
			return type() == typeid(_type)
			? static_cast<const _type*>(get()) : nullptr;
		}

			const std::type_info&
			type() const lnothrow;
	};

	inline void
		swap(any& x, any& y) lnothrow
	{
		x.swap(y);
	}

	class bad_any_cast : public std::bad_cast
	{
	private:
		const char* from_name;
		const char* to_name;

	public:
		bad_any_cast()
			: std::bad_cast(),
			from_name("unknown"), to_name("unknown")
		{};
		bad_any_cast(const std::type_info& from_type, const std::type_info& to_type)
			: std::bad_cast(),
			from_name(from_type.name()), to_name(to_type.name())
		{}

		const char*
			from() const lnothrow
		{
			return from_name;
		}

			const char*
			to() const lnothrow
		{
			return to_name;
		}

			virtual const char*
			what() const lnothrow override
		{
			return "Failed conversion: any_cast.";
		}
	};

	template<typename _tPointer>
	inline _tPointer
		any_cast(any* p) lnothrow
	{
		return p ? p->target<remove_pointer_t<_tPointer>>() : nullptr;
	}
		template<typename _tPointer>
	inline _tPointer
		any_cast(const any* p) lnothrow
	{
		return p ? p->target<remove_pointer_t<_tPointer>>() : nullptr;
	}

		template<typename _tValue>
	inline _tValue
		any_cast(any& x)
	{
		const auto tmp(any_cast<remove_reference_t<_tValue>*>(&x));

		if (!tmp)
			throw bad_any_cast(x.type(), typeid(_tValue));
		return static_cast<_tValue>(*tmp);
	}
	template<typename _tValue>
	_tValue
		any_cast(const any& x)
	{
		const auto tmp(any_cast<remove_reference_t<_tValue>*>(&x));

		if (!tmp)
			throw bad_any_cast(x.type(), typeid(_tValue));
		return static_cast<_tValue>(*tmp);
	}

	template<typename _type>
	inline _type*
		unsafe_any_cast(any* p)
	{
		lconstraint(p);
		return static_cast<_type*>(p->get());
	}

	template<typename _type>
	inline const _type*
		unsafe_any_cast(const any* p)
	{
		lconstraint(p);
		return static_cast<const _type*>(p->get());
	}

	struct pseudo_output
	{
		template<typename... _tParams>
		inline pseudo_output&
			operator=(_tParams&&...)
		{
			return *this;
		}
	};
}
#endif