/*! \file iterator.hpp
\ingroup LBase
\brief 迭代器相关操作。
*/

#ifndef LBase_iterator_hpp
#define LBase_iterator_hpp 1

#include "pointer.hpp" // for "iterator_op.hpp", iterator_operators_t,
//	std::iterator_traits, _t, pointer_classify, cond_t, and_,
//	exclude_self_t, true_type, enable_if_inconvertible_t, *_tag,
//	yassume, is_undereferenceable, yconstraint, random_access_iteratable;
#include "type_op.hpp" // for first_tag, second_tag, std::tuple,
//	make_index_sequence, index_sequence, std::get;
#include "ref.hpp" // for lref;

namespace leo
{
	template<typename _type, typename _tIter = _type*,
		typename _tTraits = std::iterator_traits<_tIter >>
	//伪迭代器,总返回单一值
	class pseudo_iterator
	{
	public:
		using iterator_type = _tIter;
		using traits_type = _tTraits;
		using iterator_category = typename traits_type::iterator_category;
		using value_type = typename traits_type::value_type;
		using difference_type = typename traits_type::difference_type;
		using pointer = typename traits_type::pointer;
		using reference = typename traits_type::reference;

		value_type value;

		lconstfn
			pseudo_iterator()
			: value()
		{}
		explicit lconstfn
			pseudo_iterator(value_type v)
			: value(v)
		{}
		lconstfn
			pseudo_iterator(const pseudo_iterator&) = default;
		lconstfn
			pseudo_iterator(pseudo_iterator&&) = default;


		pseudo_iterator&
			operator=(const pseudo_iterator&) = default;
		pseudo_iterator&
			operator=(pseudo_iterator&&) = default;

		pseudo_iterator&
			operator+=(difference_type)
		{
			return *this;
		}

		pseudo_iterator&
			operator-=(difference_type)
		{
			return *this;
		}

		reference
			operator*() lnothrow
		{
			return value;
		}

		lconstfn reference
			operator*() const
		{
			return value;
		}

		lconstfn pointer
			operator->() const
		{
			return this;
		}

		pseudo_iterator&
			operator++()
		{
			return *this;
		}
		pseudo_iterator
			operator++(int)
		{
			return *this;
		}

		lconstfn pseudo_iterator&
			operator--() const
		{
			return *this;
		}
		lconstfn pseudo_iterator
			operator--(int) const
		{
			return *this;
		}

		lconstfn reference
			operator[](difference_type n) const
		{
			return this[n];
		}

		lconstfn pseudo_iterator
			operator+(difference_type) const
		{
			return *this;
		}

		lconstfn pseudo_iterator
			operator-(difference_type) const
		{
			return *this;
		}
	};

	namespace details 
	{
		template<typename _tIter, typename _fTrans, typename _tReference>
		struct transit_traits
		{
			using iterator_type = _t<pointer_classify<_tIter>>;
			using iterator_category
				= typename std::iterator_traits<iterator_type>::iterator_category;
			using transformed_type = result_of_t<_fTrans&(_tIter&)>;
			using difference_type
				= typename std::iterator_traits<iterator_type>::difference_type;
			using reference
				= cond_t<is_same<_tReference, void>, transformed_type, _tReference>;
			/*!
			\todo 测试 operator-> 并支持代理指针。
			*/
			using pointer = decay_t<reference>*;
		};
	}

	//变换迭代器
	template<typename _tIter, typename _fTrans, typename _tReference = void>
	class transformed_iterator : public iterator_operators_t<transformed_iterator<
		_tIter, _fTrans, _tReference>, details::transit_traits<_tIter, _fTrans,
		_tReference>>, private totally_ordered<transformed_iterator<_tIter, _fTrans,
		_tReference>, typename
		details::transit_traits<_tIter, _fTrans, _tReference>::iterator_type>
	{
		static_assert(is_decayed<_tIter>(), "Invalid type found.");
		static_assert(is_decayed<_fTrans>(), "Invalid type found.");

		template<typename, typename, typename>
		friend class transformed_iterator;

	private:
		using impl_traits
			= details::transit_traits<_tIter, _fTrans, _tReference>;

	public:
		/*!
		\brief 原迭代器类型。
		*/
		using iterator_type = typename impl_traits::iterator_type;
		using transformer_type = _fTrans;
		//@{
		using iterator_category = typename impl_traits::iterator_category;
		using value_type = typename impl_traits::transformed_type;
		//@}
		using transformed_type = typename impl_traits::transformed_type;
		using difference_type = typename impl_traits::difference_type;
		using pointer = typename impl_traits::pointer;
		/*!
		\note 仅当参数 _tReference 为引用时可符合 ISO C++ 对 ForwardIterator 的要求。
		*/
		using reference = typename impl_traits::reference;

	protected:
		//! \note 当为空类时作为第一个成员可启用空基类优化。
		mutable transformer_type transformer;

	private:
		iterator_type transformed;

	public:
		transformed_iterator() = default;
		// XXX: This is not actually related to substitution order. For other cases,
		//	use of multiple template parameters to simplify after CWG 1227 resolved
		//	by C++14 may be necessary.
		template<typename _tIter2,
			limpl(typename = exclude_self_t<transformed_iterator, _tIter2>,
				typename = enable_if_convertible_t<_tIter2&&, iterator_type>)>
			explicit lconstfn
			transformed_iterator(_tIter2&& i)
			: transformer(), transformed(lforward(i))
		{}
		template<typename _tIter2, typename _fTrans2 = _fTrans,
			limpl(typename = exclude_self_t<transformed_iterator, _tIter2>)>
			explicit lconstfn
			transformed_iterator(_tIter2&& i, _fTrans2 f)
			: transformer(f), transformed(lforward(i))
		{}
		template<typename _tIter2, typename _fTrans2, typename _tRef2,
			limpl(typename = enable_if_t<and_<not_<is_convertible<const
				transformed_iterator<_tIter2, _fTrans2, _tRef2>&, iterator_type>>,
				is_constructible<_tIter, const _tIter2&>,
				is_constructible<_fTrans, const _fTrans2&>>::value>)>
			lconstfn
			transformed_iterator(
				const transformed_iterator<_tIter2, _fTrans2, _tRef2>& i)
			: transformer(i.transformer), transformed(i.get())
		{}
		template<typename _tIter2, typename _fTrans2, typename _tRef2,
			limpl(typename = enable_if_t<and_<not_<is_convertible<
				transformed_iterator<_tIter2, _fTrans2, _tRef2>&&, iterator_type>>,
				is_constructible<_tIter, _tIter2&&>,
				is_constructible<_fTrans, _fTrans2&&>>::value>)>
			lconstfn
			transformed_iterator(transformed_iterator<_tIter2, _fTrans2, _tRef2>&& i)
			: transformer(std::move(i.transformer)), transformed(i.get())
		{}
		//@{
		transformed_iterator(const transformed_iterator&) = default;
#if LB_IMPL_MSCPP
		//! \since build 503 as workaround for Visual C++ 2013
		transformed_iterator(transformed_iterator&& i)
			: transformer(std::move(i.transformer))
		{}
#else
		transformed_iterator(transformed_iterator&&) = default;
#endif
		//@}

		transformed_iterator&
			operator=(const transformed_iterator&) = default;
		transformed_iterator&
			operator=(transformed_iterator&&) = default;

		friend transformed_iterator&
			operator+=(transformed_iterator& i, difference_type n)
			lnoexcept(noexcept(decltype(i)(i)) && noexcept(i.get() += n))
		{
			i.get() += n;
			return i;
		}

		friend transformed_iterator&
			operator-=(transformed_iterator& i, difference_type n)
			lnoexcept(noexcept(decltype(i)(i)) && noexcept(i.get() -= n))
		{
			i.get() -= n;
			return i;
		}

		reference
			operator*() const
			lnoexcept_spec(reference(std::declval<transformed_iterator&>().
				transformer(std::declval<transformed_iterator&>().get())))
		{
			return transformer(get());
		}

		transformed_iterator&
			operator++()
		{
			++get();
			return *this;
		}

		transformed_iterator&
			operator--()
		{
			--get();
			return *this;
		}

		//@{
		template<limpl(typename = void)>
		friend lconstfn bool
			operator==(const transformed_iterator& x, const transformed_iterator& y)
			lnoexcept_spec(bool(x.get() == y.get()))
		{
			return x.get() == y.get();
		}
		template<limpl(typename = void)>
		friend lconstfn bool
			operator==(const transformed_iterator& x, const iterator_type& y)
			lnoexcept_spec(bool(x.get() == y))
		{
			return x.get() == y;
		}

		template<limpl(typename = void)>
		friend lconstfn bool
			operator<(const transformed_iterator& x, const transformed_iterator& y)
			lnoexcept_spec(bool(x.get() < y.get()))
		{
			return bool(x.get() < y.get());
		}
		template<limpl(typename = void)>
		friend lconstfn bool
			operator<(const transformed_iterator& x, const iterator_type& y)
			lnoexcept_spec(bool(x.get() < y))
		{
			return bool(x.get() < y);
		}

		template<limpl(typename = void)>
		friend lconstfn difference_type
			operator-(const transformed_iterator& x, const transformed_iterator& y)
			lnoexcept_spec(difference_type(x.get() - y.get()))
		{
			return difference_type(x.get() - y.get());
		}
		template<limpl(typename = void)>
		friend lconstfn difference_type
			operator-(const transformed_iterator& x, const iterator_type& y)
			lnoexcept_spec(difference_type(x.get() - y))
		{
			return difference_type(x.get() - y);
		}
		template<limpl(typename = void)>
		friend lconstfn difference_type
			operator-(const iterator_type& x, const transformed_iterator& y)
			lnoexcept_spec(difference_type(x - y.get()))
		{
			return difference_type(x - y.get());
		}
		//@}

		//@{
		//! \brief 取原迭代器引用。
		lconstfn_relaxed iterator_type&
			get() lnothrow
		{
			return transformed;
		}

		//! \brief 取原迭代器 const 引用。
		lconstfn const iterator_type&
			get() const lnothrow
		{
			return transformed;
		}
		//@}

		//@{
		lconstfn_relaxed transformer_type&
			get_transformer() lnothrow
		{
			return transformer;
		}
		lconstfn const transformer_type&
			get_transformer() const lnothrow
		{
			return transformer;
		}
		//@}
	};

	template<typename _tIter, typename _fTransformer>
	inline transformed_iterator<decay_t<_tIter>,
		_fTransformer>
		make_transform(_tIter&& i, _fTransformer f)
	{
		return transformed_iterator<decay_t<_tIter>,
			_fTransformer>(lforward(i), f);
	}

	namespace iterator_transformation
	{

		//! \since build 1.4
		//@{
		template<typename _tIter = void>
		struct first
		{
			lconstfn auto
				operator()(const _tIter& i) const -> decltype((i->first))
			{
				return i->first;
			}
		};

		template<>
		struct first<void>
		{
			template<typename _tIter>
			lconstfn auto
				operator()(const _tIter& i) const -> decltype((i->first))
			{
				return i->first;
			}
		};


		template<typename _tIter = void>
		struct get
		{
			lconstfn auto
				operator()(const _tIter& i) const -> decltype((*i).get())
			{
				return (*i).get();
			}
		};

		template<>
		struct get<void>
		{
			template<typename _tIter>
			lconstfn auto
				operator()(const _tIter& i) const -> decltype((*i).get())
			{
				return (*i).get();
			}
		};


		template<typename _tIter = void>
		struct indirect
		{
			lconstfn auto
				operator()(const _tIter& i) const -> decltype(**i)
			{
				return **i;
			}
		};

		template<>
		struct indirect<void>
		{
			template<typename _tIter>
			lconstfn auto
				operator()(const _tIter& i) const -> decltype(**i)
			{
				return **i;
			}
		};


		template<typename _tIter = void>
		struct second
		{
			lconstfn auto
				operator()(const _tIter& i) const -> decltype((i->second))
			{
				return i->second;
			}
		};

		template<>
		struct second<void>
		{
			template<typename _tIter>
			lconstfn auto
				operator()(const _tIter& i) const -> decltype((i->second))
			{
				return i->second;
			}
		};
		//@}
	} // namespace iterator_transformation;

	lconstexpr first_tag get_first{}, get_key{};
	lconstexpr second_tag get_second{}, get_value{};
	lconstexpr struct indirect_tag_t{} get_indirect{};
	lconstexpr const struct get_tag_t {} get_get{};

	template<typename _tIter>
	inline auto
		operator|(_tIter&& i, first_tag)
		-> decltype(make_transform(lforward(i), 
			iterator_transformation::first<>()))
	{
		return make_transform(lforward(i), iterator_transformation::first<>());
	}
	template<typename _tIter>
	inline auto
		operator|(_tIter&& i, second_tag)
		-> decltype(make_transform(lforward(i),
			iterator_transformation::second<>()))
	{
		return make_transform(lforward(i), iterator_transformation::second<>());
	}
	template<typename _tIter>
	inline auto
		operator|(_tIter&& i, indirect_tag_t)
		-> decltype(make_transform(lforward(i), 
			iterator_transformation::indirect<>()))
	{
		return make_transform(lforward(i), iterator_transformation::indirect<>());
	}
	template<typename _tIter>
	inline auto
		operator|(_tIter&& i, get_tag_t)
		-> decltype(make_transform(lforward(i), 
			iterator_transformation::get<>()))
	{
		return make_transform(lforward(i), iterator_transformation::get<>());
	}

	template<typename _tMaster, typename _tSlave,
	class _tTraits = std::iterator_traits<_tMaster >>
	//成对迭代器
	class pair_iterator : private std::pair<_tMaster, _tSlave>
	{
	public:
		using pair_type = std::pair<_tMaster, _tSlave>;
		using iterator_type = _tMaster;
		using traits_type = _tTraits;
		using iterator_category = typename traits_type::iterator_category;
		using value_type = typename traits_type::value_type;
		using difference_type = typename traits_type::difference_type;
		using pointer = typename traits_type::pointer;
		using reference = typename traits_type::reference;

		lconstfn
			pair_iterator()
			: std::pair<_tMaster, _tSlave>(_tMaster(), _tSlave())
		{}
		explicit lconstfn
			pair_iterator(const _tMaster& _i)
			: std::pair<_tMaster, _tSlave>(_i, _tSlave())
		{}
		lconstfn
			pair_iterator(const _tMaster& _i, const _tSlave& _s)
			: std::pair<_tMaster, _tSlave>(_i, _s)
		{}
		lconstfn
			pair_iterator(const pair_iterator&) = default;
		lconstfn
			pair_iterator(pair_iterator&& _r)
			: std::pair<_tMaster, _tSlave>(std::move(_r))
		{}

		inline pair_iterator&
			operator=(const pair_iterator&) = default;
		inline pair_iterator&
#if LB_IMPL_MSCPP
			operator=(pair_iterator&& i)
		{
			static_cast<std::pair<_tMaster, _tSlave>&>(*this)
				= static_cast<std::pair<_tMaster, _tSlave>&&>(i);
			return *this;
		}
#else
			operator=(pair_iterator&&) = default;
#endif

		pair_iterator&
			operator+=(difference_type n)
		{
			lunseq(this->first += n, this->second += n);
			return *this;
		}

		pair_iterator&
			operator-=(difference_type n)
		{
			lunseq(this->first -= n, this->second -= n);
			return *this;
		}

		lconstfn reference
			operator*() const
		{
			return *this->first;
		}

		lconstfn pointer
			operator->() const
		{
			return this->first;
		}

		pair_iterator&
			operator++()
		{
			lunseq(++this->first, ++this->second);
			return *this;
		}
		pair_iterator
			operator++(int)
		{
			auto i(*this);

			++*this;
			return i;
		}

		pair_iterator&
			operator--()
		{
			lunseq(--this->first, --this->second);
			return *this;
		}
		pair_iterator
			operator--(int)
		{
			auto i(*this);

			--*this;
			return i;
		}

		lconstfn reference
			operator[](difference_type n) const
		{
			return this->first[n];
		}

		pair_iterator
			operator+(difference_type n) const
		{
			auto i(*this);

			lunseq(i.first += n, i.second += n);
			return i;
		}

		pair_iterator
			operator-(difference_type n) const
		{
			auto i(*this);

			lunseq(i.first -= n, i.second -= n);
			return i;
		}

		template<typename _tFirst, typename _tSecond,
			typename = enable_if_t<is_convertible<_tMaster, _tFirst>::value
			&& is_convertible<_tSlave, _tSecond>::value, int >>
			operator std::pair<_tFirst, _tSecond>()
		{
			return std::pair<_tFirst, _tSecond>(this->first, this->second);
		}

		lconstfn const pair_type&
			base() const
		{
			return *this;
		}
	};

	template<typename _tMaster, typename _tSlave>
	bool
		operator==(const pair_iterator<_tMaster, _tSlave>& x,
		const pair_iterator<_tMaster, _tSlave>& y)
	{
		return x.base().first == y.base().first
			&& x.base().second == y.base().second();
	}

	template<typename _tMaster, typename _tSlave>
	inline bool
		operator!=(const pair_iterator<_tMaster, _tSlave>& x,
		const pair_iterator<_tMaster, _tSlave>& y)
	{
		return !(x == y);
	}

	template<typename _tIter>
	//间接输入迭代器
	class indirect_input_iterator
	{
	public:
		using iterator_type = _tIter;
		using iterator_category = std::input_iterator_tag;
		using value_type = typename std::iterator_traits<iterator_type>::value_type;
		using difference_type
			= typename std::iterator_traits<iterator_type>::difference_type;
		using pointer = typename std::iterator_traits<iterator_type>::pointer;
		using reference = typename std::iterator_traits<iterator_type>::reference;

	private:
		iterator_type iter;

	public:
		indirect_input_iterator()
			: iter()
		{}
		indirect_input_iterator(iterator_type i)
			: iter(i)
		{
			++*this;
		}
		indirect_input_iterator(const indirect_input_iterator&) = default;
		indirect_input_iterator(indirect_input_iterator&& i) lnothrow
			: iter()
		{
			using std::swap;

			swap(iter, i.iter);
		}

		indirect_input_iterator&
			operator=(const indirect_input_iterator&) = default;
		indirect_input_iterator&
#if LB_IMPL_MSCPP
			//! \since build 458 as workaround for Visual C++ 2013
			operator=(indirect_input_iterator&& i)
		{
			iter = std::move(i.iter);
			return *this;
		}
#else
			operator=(indirect_input_iterator&&) = default;
#endif

		pointer
			operator->() const
		{
			return (*iter).operator->();
		}

		template<typename = enable_if_t<is_constructible<bool,
			decltype(*std::declval<iterator_type&>())>::value, int >>
			explicit
			operator bool() const
			//	operator bool() const lnoexcept((!is_undereferenceable(std::declval<
			//		iterator_type&>()) && bool(*std::declval<iterator_type&>())))
		{
			return !is_undereferenceable(iter) && bool(*iter);
		}

		reference
			operator*() const
		{
			lconstraint(!is_undereferenceable(iter));
			return **iter;
		}

		indirect_input_iterator&
			operator++()
		{
			lconstraint(!is_undereferenceable(iter));
			++*iter;
			return *this;
		}
		indirect_input_iterator
			operator++(int)
		{
			const auto i(*this);

			++*this;
			return i;
		}

		friend bool
			operator==(const indirect_input_iterator& x, const indirect_input_iterator& y)
		{
			return (!bool(x) && !bool(y)) || x.iter == y.iter;
		}

		iterator_type&
			get() lnothrow
		{
			return iter;
		}
			const iterator_type&
			get() const lnothrow
		{
			return iter;
		}
	};

	template<typename _tIter>
	inline bool
		operator!=(const indirect_input_iterator<_tIter>& x,
		const indirect_input_iterator<_tIter>& y)
	{
		return !(x == y);
	}

	template<class _tCon, typename _type>
	//成员下标迭代器
	class subscriptive_iterator
	{
	public:
		using container_type = _tCon;
		using iterator_category = std::random_access_iterator_tag;
		using value_type = _type;
		using difference_type = ptrdiff_t;
		using pointer = _type*;
		using reference = _type&;

	protected:
		_tCon* con_ptr;
		size_t idx;

	public:
		lconstfn
			subscriptive_iterator(_tCon& c, size_t i)
			: con_ptr(std::addressof(c)), idx(i)
		{}

		subscriptive_iterator&
			operator+=(difference_type n)
		{
			idx += n;
			return *this;
		}

		subscriptive_iterator&
			operator-=(difference_type n)
		{
			lassume(!(idx < n));
			idx -= n;
			return *this;
		}

		reference
			operator*() const
		{
			return (*con_ptr)[idx];
		}

		pointer
			operator->() const
		{
			return std::addressof(**this);
		}

		subscriptive_iterator&
			operator++() lnothrow
		{
			++idx;
			return *this;
		}
			subscriptive_iterator
			operator++(int)lnothrow
		{
			auto i(*this);

			++*this;
			return i;
		}

			subscriptive_iterator
			operator--() lnothrow
		{
			--idx;
			return *this;
		}
			subscriptive_iterator
			operator--(int)lnothrow
		{
			auto i(*this);

			--*this;
			return i;
		}

			reference
			operator[](difference_type n) const
		{
			lassume(!(idx + n < 0));
			return (*con_ptr)[idx + n];
		}

		subscriptive_iterator
			operator+(difference_type n) const
		{
			lassume(!(idx + n < 0));
			return subscriptive_iterator(*con_ptr, idx + n);
		}

		subscriptive_iterator
			operator-(difference_type n) const
		{
			lassume(!(idx + n < 0));
			return subscriptive_iterator(*con_ptr, idx - n);
		}

		_tCon*
			container() const lnothrow
		{
			return con_ptr;
		}

			bool
			equals(const subscriptive_iterator<_tCon, _type>& i) const lnothrow
		{
			return con_ptr == i.con_ptr && idx == i.idx;
		}

			size_t
			index() const lnothrow
		{
			return idx;
		}
	};

	template<class _tCon, typename _type>
	bool
		operator==(const subscriptive_iterator<_tCon, _type>& x,
		const subscriptive_iterator<_tCon, _type>& y) lnothrow
	{
		return x.equals(y);
	}

		template<class _tCon, typename _type>
	bool
		operator!=(const subscriptive_iterator<_tCon, _type>& x,
		const subscriptive_iterator<_tCon, _type>& y) lnothrow
	{
		return !(x == y);
	}
}

#endif