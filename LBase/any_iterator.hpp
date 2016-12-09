/*! \file any_iterator.hpp
\ingroup LBase
\brief 动态泛型迭代器。
\par 修改时间:
2016-11-26 12:27 +0800
*/

#ifndef LBase_any_iterator_h
#define LBase_any_iterator_h 1

#include "LBase/any.h"
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

		template<typename _type, typename _tReference>
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
				{
					using obj_t = cond_t<is_reference<_tReference>,
						lref<decay_t<_tReference>>, _tReference>;
					auto& p(d.access<any*>());

					lassume(p);
					*p = obj_t(*get_reference(s));
				}
				break;
				case increase:
					++get_reference(d);
					break;
				default:
					base::manage(d, s, op);
				}
			}
		};

		template<typename _type, typename _tReference>
		class input_iterator_handler : public iterator_handler<_type, _tReference>
		{
		public:
			using base = iterator_handler<_type, _tReference>;
			using value_type = typename base::value_type;

			using base::get_reference;

			using base::init;

			static void
				manage(any_storage& d, const any_storage& s, any_ops::op_code op)
			{
				switch (op)
				{
				case equals:
					if (get_reference(*d.access<any_storage*>()) != get_reference(s))
						d = static_cast<any_storage*>(nullptr);
					break;
				default:
					base::manage(d, s, op);
				}
			}
		};

		template<typename _type, typename _tReference>
		class forward_iterator_handler : public input_iterator_handler<_type, _tReference>
		{
		public:
			using base = input_iterator_handler<_type, _tReference>;
			using value_type = typename base::value_type;

			using base::get_reference;

			using base::init;

			using base::manage;
		};

		template<typename _type, typename _tReference>
		class bidirectional_iterator_handler : public forward_iterator_handler<_type, _tReference>
		{
		public:
			using base = forward_iterator_handler<_type, _tReference>;
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

	template<typename _type, typename _tDifference = ptrdiff_t,
		typename _tPointer = _type*, typename _tReference = _type&>
		//动态泛型输入迭代器
		class any_input_iterator : protected any,
		public input_iteratable<any_input_iterator<_type, _tDifference, _tPointer,
		_tReference>, _tReference>
	{
	protected:
		using base = any;
	public:
		using iterator_category = std::input_iterator_tag;
		using value_type = _type;
		using difference_type = _tDifference;

		using pointer = _tPointer;
		using reference = _tReference;

		any_input_iterator() = default;
		template<typename _tIter,
			limpl(typename = exclude_self_t<any_input_iterator, _tIter>)>
			any_input_iterator(_tIter&& i)
			: any_input_iterator(any_ops::with_handler_t<
				any_ops::input_iterator_handler<decay_t<_tIter>, reference>>(),
				lforward(i))
		{}
	protected:
		template<typename _tIter, typename _tHandler>
		any_input_iterator(any_ops::with_handler_t<_tHandler> t, _tIter&& i)
			: base(t, lforward(i))
		{
			static_assert(is_convertible<indirect_t<decay_unwrap_t<_tIter>>,
				reference>(), "Wrong target iterator type found.");
		}

	public:
		any_input_iterator(const any_input_iterator&) = default;

		any_input_iterator(any_input_iterator&&) = default;

		any_input_iterator&
			operator=(const any_input_iterator&) = default;
		any_input_iterator&

			operator=(any_input_iterator&&) = default;


		reference
			operator*() const
		{
			// TODO: More graceful implementation for unused value?
			any res;
			any_ops::any_storage t;
			const auto gd(t.pun<any*>(&res));

			call(t, any_ops::dereference);
			return reference(
				*leo::unchecked_any_cast<remove_reference_t<reference>>(&res));
		}

		pointer
			operator->() const
		{
			return &**this;
		}

		any_input_iterator&
			operator++()
		{
			call(get_storage(), any_ops::increase);
			return *this;
		}

		friend bool
			operator==(const any_input_iterator& x, const any_input_iterator& y)
		{
			if (x.has_value() || y.has_value())
			{
				lassume(x.type() == y.type());

				using any_ops::any_storage;
				any_storage t;
				const auto gd(t.pun<any_storage*>(&x.get_storage()));

				return y.template
					unchecked_access<any_storage*>(t, any_ops::equals);
			}
			return true;
		}

		any&
			get() lnothrow
		{
			return *this;
		}

		const any&
			get() const lnothrow
		{
			return *this;
		}

		friend bool
			is_undereferenceable(const any_input_iterator& i)
		{
			return i.has_value() ? i.template unchecked_access<bool>(
				any_ops::check_undereferenceable) : true;
		}


		using any::target;
		using any::type;
	};

	using input_monomorphic_iterator
		= any_input_iterator<void_ref, ptrdiff_t, void*, void_ref>;

	template<typename _type, typename _tDifference = ptrdiff_t,
		typename _tPointer = _type*, typename _tReference = _type&>
		//动态泛型前向迭代器
		class any_forward_iterator
		: public any_input_iterator<_type, _tDifference,
		_tPointer, _tReference>, public forward_iteratable<any_forward_iterator<
		_type, _tDifference, _tPointer, _tReference>, _tReference>
	{
	protected:
		using base = any_input_iterator<_type, _tDifference,
			_tPointer, _tReference>;
	public:
		using iterator_category = std::forward_iterator_tag;
		using pointer = _tPointer;
		using reference = _tReference;

		any_forward_iterator() = default;
		template<typename _tIter,
			limpl(typename = exclude_self_t<any_forward_iterator, _tIter>)>
			any_forward_iterator(_tIter&& i)
			: any_forward_iterator(any_ops::with_handler_t<
				any_ops::forward_iterator_handler<decay_t<_tIter>, reference>>(),
				lforward(i))
		{
			static_assert(is_convertible<indirect_t<decay_unwrap_t<_tIter>>,
				reference>(), "Wrong target iterator type found.");
		}
	protected:
		template<typename _tIter, typename _tHandler>
		any_forward_iterator(any_ops::with_handler_t<_tHandler> t, _tIter&& i)
			: base(t, lforward(i))
		{}
	public:

		any_forward_iterator(const any_forward_iterator&) = default;

		any_forward_iterator(any_forward_iterator&&) = default;


		any_forward_iterator&
			operator=(const any_forward_iterator&) = default;
		any_forward_iterator&

			operator=(any_forward_iterator&&) = default;


		any_forward_iterator&
			operator++()
		{
			base::operator++();
			return *this;
		}

		friend bool
			operator==(const any_forward_iterator& x, const any_forward_iterator& y)
		{
			return static_cast<const base&>(x) == static_cast<const base&>(y);
		}
	};

	using forward_monomorphic_iterator
		= any_forward_iterator<void_ref, ptrdiff_t, void*, void_ref>;

	template<typename _type, typename _tDifference = ptrdiff_t,
		typename _tPointer = _type*, typename _tReference = _type&>
		//动态泛型双向迭代器
		class any_bidirectional_iterator
		: public any_forward_iterator<_type,
		_tDifference, _tPointer, _tReference>, public bidirectional_iteratable<
		any_bidirectional_iterator<_type, _tDifference, _tPointer, _tReference>,
		_tReference>
	{
	protected:
		using base = any_forward_iterator<_type,
			_tDifference, _tPointer, _tReference>;

	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using pointer = _tPointer;
		using reference = _tReference;

		any_bidirectional_iterator() = default;
		template<typename _tIter,
			limpl(typename = exclude_self_t<any_bidirectional_iterator, _tIter>)>
			any_bidirectional_iterator(_tIter&& i)
			: any_forward_iterator<_type, _tPointer, _tReference>(lforward(i))
		{}

	protected:
		template<typename _tIter, typename _tHandler>
		any_bidirectional_iterator(any_ops::with_handler_t<_tHandler> t, _tIter&& i)
			: base(t, lforward(i))
		{}

	public:
		any_bidirectional_iterator(const any_bidirectional_iterator&) = default;
		any_bidirectional_iterator(any_bidirectional_iterator&&) = default;

		any_bidirectional_iterator&
			operator=(const any_bidirectional_iterator&) = default;
		any_bidirectional_iterator&

			operator=(any_bidirectional_iterator&&) = default;


		any_bidirectional_iterator&
			operator++()
		{
			base::operator++();
			return *this;
		}

		any_bidirectional_iterator&
			operator--()
		{
			this->call(this->get_storage(), any_ops::decrease);
			return *this;
		}

		friend bool
			operator==(const any_bidirectional_iterator& x,
				const any_bidirectional_iterator& y)
		{
			return static_cast<const base&>(x) == static_cast<const base&>(y);
		}
	};

	using bidirectional_monomorphic_iterator
		= any_bidirectional_iterator<void_ref, ptrdiff_t, void*, void_ref>;
}

#endif