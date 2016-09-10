#ifndef LBase_any_iterator_h
#define LBase_any_iterator_h 1

#include "LBase/any.h"
#include "LBase/functional.hpp"
#include "LBase/iterator.hpp"

namespace leo
{
	namespace any_ops
	{
		enum iterator_op : op_code
		{
			check_undereferenceable = end_base_op,
			dereference,
			increase,
			end_iterator_op
		};

		enum input_iterator_op : op_code
		{
			equals = end_iterator_op,
			end_input_iterator_op,
			end_output_iterator_op = end_input_iterator_op,
			end_forward_iterator_op = end_input_iterator_op
		};


		enum bidirectional_iteartor_op : op_code
		{
			decrease = end_forward_iterator_op,
			end_bidirectional_iterator_op
		};


		enum random_access_iteartor_op : op_code
		{
			advance = end_forward_iterator_op,
			distance,
			less_compare,
			end_random_access_iteartor_op
		};

		template<typename _type>
		struct wrap_handler
		{
			using value_type = unwrap_reference_t<_type>;
			using type = cond_t<is_reference_wrapper<_type>,
				ref_handler<value_type>, value_handler<value_type >> ;
		};

		template<typename _type>
		class iterator_handler : public wrap_handler<_type>::type
		{
		public:
			using base = typename wrap_handler<_type>::type;
			using value_type = typename base::value_type;

			using base::get_reference;

			using base::init;

			static void
				manage(any_storage& d, const any_storage& s, any_ops::op_code op)
			{
				switch (op)
				{
				case check_undereferenceable:
					d.access<bool>() = is_undereferenceable(get_reference(s));
					break;
				case dereference:
					d = void_ref(*get_reference(s));
					break;
				case increase:
					++get_reference(d);
					break;
				default:
					base::manage(d, s, op);
				}
			}
		};

		template<typename _type>
		class input_iterator_handler : public iterator_handler<_type>
		{
		public:
			using base = iterator_handler<_type>;
			using value_type = typename base::value_type;

			using base::get_reference;

			using base::init;

			static void
				manage(any_storage& d, const any_storage& s, any_ops::op_code op)
			{
				switch (op)
				{
				case equals:
				{
					const auto p(d.access<any_storage*>());

					d.access<bool>() = get_reference(*p) == get_reference(s);
				}
					break;
				default:
					base::manage(d, s, op);
				}
			}
		};

		template<typename _type>
		class forward_iterator_handler : public input_iterator_handler<_type>
		{
		public:
			using base = input_iterator_handler<_type>;
			using value_type = typename base::value_type;

			using base::get_reference;

			using base::init;

			using base::manage;
		};

		template<typename _type>
		class bidirectional_iterator_handler : public forward_iterator_handler<_type>
		{
		public:
			using base = forward_iterator_handler<_type>;
			using value_type = typename base::value_type;

			using base::get_reference;

			using base::init;

			static void
				manage(any_storage& d, const any_storage& s, any_ops::op_code op)
			{
				switch (op)
				{
				case decrease:
					--get_reference(d);
					break;
				default:
					base::manage(d, s, op);
				}
			}
		};
	}//namespace any_ops
#define LB_IterOp1(_n, _t, _it, _e) \
	template<typename _type, typename _tDifference, typename _tPointer, \
		typename _tReference> \
	inline _t \
	_n(const _it<_type, _tDifference, _tPointer, _tReference>& i) \
		{ \
		return _e; \
		}

#define LB_IterOp2(_n, _t, _it, _e) \
	template<typename _type, typename _tDifference, typename _tPointer, \
		typename _tReference> \
	inline _t \
	_n(const _it<_type, _tDifference, _tPointer, _tReference>& x, \
		const _it<_type, _tDifference, _tPointer, _tReference>& y) \
		{ \
		return _e; \
		}

#define LB_IterOpPost(_op, _it) \
	_it \
	operator _op(int) \
		{ \
		auto tmp = *this; \
	\
		_op *this; \
		return tmp; \
		}

	template<typename _type, typename _tDifference = ptrdiff_t,
		typename _tPointer = _type*, typename _tReference = _type&>
	//动态泛型输入迭代器
	class any_input_iterator : public std::iterator<std::input_iterator_tag, _type,
		_tDifference, _tPointer, _tReference>, protected any
	{
	public:
		using pointer = _tPointer;
		using reference = _tReference;

		any_input_iterator() = default;
		template<typename _tIter>
		any_input_iterator(_tIter&& i)
			: any()
		{
			using param_obj_type = typename remove_rcv<_tIter>::type;
			using handler = any_ops::input_iterator_handler<param_obj_type>;

			static_assert(is_convertible<decltype(*std::declval<typename
				wrapped_traits<param_obj_type>::type&>()), reference>::value,
				"Wrong target iterator type found.");

			manager = handler::manage;
			handler::init(storage, lforward(i));
		}
		any_input_iterator(const any_input_iterator&) = default;

		any_input_iterator(any_input_iterator&&) = default;

		any_input_iterator&
			operator=(const any_input_iterator&) = default;
		any_input_iterator&

		operator=(any_input_iterator&&) = default;


		reference
			operator*() const
		{
			lassume(manager);

			any_ops::any_storage t;

			manager(t, storage, any_ops::dereference);
			return reference(t.access<void_ref>());
		}

		pointer
			operator->() const
		{
			return &**this;
		}

		any_input_iterator&
			operator++()
		{
			lassume(manager);
			manager(storage, storage, any_ops::increase);
			return *this;
		}

		any
			get() const
		{
			return static_cast<const any&>(*this);
		}

		bool
			check_undereferenceable() const
		{
			if (manager)
			{
				any_ops::any_storage t;

				manager(t, storage, any_ops::check_undereferenceable);
				return t.access<bool>();
			}
			return true;
		}

		bool
			equals(const any_input_iterator& i) const
		{
			if (!*this && !i)
				return true;
			lassume(type() == i.type());

			any_ops::any_storage t(&storage);

			manager(t, i.storage, any_ops::equals);
			return t.access<bool>();
		}

		using any::type;
	};

	LB_IterOp2(operator==, bool, any_input_iterator, x.equals(y))

	LB_IterOp2(operator!=, bool, any_input_iterator, !(x == y))

	LB_IterOp1(is_undereferenceable, bool, any_input_iterator,
		i.check_undereferenceable())

	using input_monomorphic_iterator
		= any_input_iterator<void_ref, ptrdiff_t, void*, void_ref>;

	template<typename _type, typename _tDifference = ptrdiff_t,
		typename _tPointer = _type*, typename _tReference = _type&>
	//动态泛型前向迭代器
	class any_forward_iterator
		: public any_input_iterator<_type, _tDifference, _tPointer, _tReference>
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using pointer = _tPointer;
		using reference = _tReference;

		any_forward_iterator() = default;
		template<typename _tIter>
		any_forward_iterator(_tIter&& i)
			: any_input_iterator<_type, _tPointer, _tReference>(lforward(i))
		{}
		any_forward_iterator(const any_forward_iterator&) = default;

		any_forward_iterator(any_forward_iterator&&) = default;


		any_forward_iterator&
			operator=(const any_forward_iterator&) = default;
		any_forward_iterator&

		operator=(any_forward_iterator&&) = default;


		any_forward_iterator&
			operator++()
		{
			any_input_iterator<_type, _tPointer, _tReference>::operator++();
			return *this;
		}
		LB_IterOpPost(++, any_forward_iterator)
	};

	LB_IterOp2(operator==, bool, any_forward_iterator, x.equals(y))

	LB_IterOp2(operator!=, bool, any_forward_iterator, !(x == y))

	LB_IterOp1(is_undereferenceable, bool, any_forward_iterator,
		i.check_undereferenceable())

	using forward_monomorphic_iterator
		= any_forward_iterator<void_ref, ptrdiff_t, void*, void_ref>;

	template<typename _type, typename _tDifference = ptrdiff_t,
		typename _tPointer = _type*, typename _tReference = _type&>
	//动态泛型双向迭代器
	class any_bidirectional_iterator
		: public any_forward_iterator<_type, _tDifference, _tPointer, _tReference>
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using pointer = _tPointer;
		using reference = _tReference;

		any_bidirectional_iterator() = default;
		template<typename _tIter>
		any_bidirectional_iterator(_tIter&& i)
			: any_input_iterator<_type, _tPointer, _tReference>(lforward(i))
		{}
		any_bidirectional_iterator(const any_bidirectional_iterator&) = default;
#if LB_IMPL_MSCPP
		any_bidirectional_iterator(any_bidirectional_iterator&& i)
			: any_forward_iterator(static_cast<any_forward_iterator&&>(i))
		{}
#else
		any_bidirectional_iterator(any_bidirectional_iterator&&) = default;
#endif

		any_bidirectional_iterator&
			operator=(const any_bidirectional_iterator&) = default;
		any_bidirectional_iterator&

		operator=(any_bidirectional_iterator&&) = default;


		any_bidirectional_iterator&
			operator++()
		{
			any_forward_iterator<_type, _tPointer, _tReference>::operator++();
			return *this;
		}
		LB_IterOpPost(++, any_bidirectional_iterator)

			any_bidirectional_iterator&
			operator--()
		{
			lassume(this->manager);
			this->manager(this->storage, this->storage, any_ops::decrease);
			return *this;
		}
		LB_IterOpPost(--, any_bidirectional_iterator)
	};

	LB_IterOp2(operator==, bool, any_bidirectional_iterator, x.equals(y))

		LB_IterOp2(operator!=, bool, any_bidirectional_iterator, !(x == y))

		LB_IterOp1(is_undereferenceable, bool, any_bidirectional_iterator,
		i.check_undereferenceable())

		using bidirectional_monomorphic_iterator
		= any_bidirectional_iterator<void_ref, ptrdiff_t, void*, void_ref>;


#undef LB_IterOp1
#undef LB_IterOp2
#undef LB_IterOpPost
}

#endif