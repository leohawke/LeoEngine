#ifndef IndePlatform_iterator_hpp
#define IndePlatform_iterator_hpp

#include "type_op.hpp" //leo::remove_reference_t,::*_tag;
#include <cassert>
#include <iterator> //std::make_move_iterator,iterator,iterator_tarits
#include <utility> //std::make_pair
#include <memory> //std::addressof

namespace std
{
	template<typename _type>
	struct atomic;
}

namespace leo
{
	//类型选择,cl编译器可能在此有错误
	template<typename _tParam, typename = limpl(std::pair<
		decltype(*std::declval<_tParam&>()), decltype(++std::declval<_tParam&>())>)>
		using enable_for_iterator_t = enable_if_t<
		is_same<decltype(++std::declval<_tParam&>()), _tParam&>::value, int>;

	/*!	\defgroup is_undereferenceable Is Undereferenceable Iterator
	\brief 判断迭代器实例是否不可解引用。
	\tparam _tIter 迭代器类型。
	\note 注意返回 \c false 不表示参数实际可解引用。
	*/
	//@{
	template<typename _tIter>
	lconstfn bool
		is_undereferenceable(const _tIter&) lnothrow
	{
		return{};
	}
	template<typename _type>
	lconstfn bool
		is_undereferenceable(_type* p) lnothrow
	{
		return !p;
	}
	template<typename _type>
	lconstfn bool
		is_undereferenceable(const std::atomic<_type>&) = delete;
	template<typename _type>
	lconstfn bool
		is_undereferenceable(const volatile std::atomic<_type>&) = delete;
	//@}

	/*
	迭代器指向的值满足条件取邻接迭代器,否则取原值
	*/
	template<typename _tIn, typename _fPred>
	_tIn
		next_if(_tIn i, _fPred f,
		typename std::iterator_traits<_tIn>::difference_type n = 1)
	{
		lconstraint(!is_undereferenceable(i));
		return f(*i) ? std::next(i, n) : i;
	}
	template<typename _tIn, typename _type>
	_tIn
		next_if_eq(_tIn i, const _type& val,
		typename std::iterator_traits<_tIn>::difference_type n = 1)
	{
		lconstraint(!is_undereferenceable(i));
		return *i == val ? std::next(i, n) : i;
	}

	/*
	迭代器指向的值满足条件时取反向邻接迭代器,否则取原值
	*/
	template<typename _tBi, typename _fPred>
	_tBi
		prev_if(_tBi i, _fPred f,
		typename std::iterator_traits<_tBi>::difference_type n = 1)
	{
		return f(*i) ? std::prev(i, n) : i;
	}
	template<typename _tBi, typename _type>
	_tBi
		prev_if_eq(_tBi i, const _type& val,
		typename std::iterator_traits<_tBi>::difference_type n = 1)
	{
		return *i == val ? std::prev(i, n) : i;
	}

	template<typename _tIterator1, typename _tIterator2>
	inline auto
		make_move_iterator_pair(_tIterator1 it1, _tIterator2 it2) -> decltype(
		std::make_pair(std::make_move_iterator(it1), std::make_move_iterator(it2)))
	{
		return std::make_pair(std::make_move_iterator(it1),
			std::make_move_iterator(it2));
	}

	template<typename _tRange>
	inline auto
		make_move_iterator_pair(_tRange& c)
		-> decltype(leo::make_move_iterator_pair(begin(c), end(c)))
	{
		return leo::make_move_iterator_pair(begin(c), end(c));
	}

	template<typename _type>
	//指针迭代器
	class pointer_iterator
	{
	public:
		using iterator_type = _type*;
		using iterator_category
			= typename std::iterator_traits<iterator_type>::iterator_category;
		using value_type = typename std::iterator_traits<iterator_type>::value_type;
		using difference_type
			= typename std::iterator_traits<iterator_type>::difference_type;
		using pointer = typename std::iterator_traits<iterator_type>::pointer;
		using reference = typename std::iterator_traits<iterator_type>::reference;

	protected:
		pointer raw;

	public:
		lconstfn
			pointer_iterator(nullptr_t = {})
			: raw()
		{}
		template<typename _tPointer>
		explicit lconstfn
			pointer_iterator(_tPointer&& ptr)
			: raw(lforward(ptr))
		{}
		inline
			pointer_iterator(const pointer_iterator&) = default;

		pointer_iterator&
			operator+=(difference_type n)
		{
			lconstraint(raw);
			raw += n;
			return *this;
		}

		pointer_iterator&
			operator-=(difference_type n)
		{
			lconstraint(raw);
			raw -= n;
			return *this;
		}
		reference
			operator*() const lnothrowv
		{
			lconstraint(raw);
			return *raw;
		}

		lconstfn pointer
			operator->() const
		{
			return raw;
		}

		inline pointer_iterator&
			operator++()
		{
			lconstraint(raw);
			++raw;
			return *this;
		}
		pointer_iterator
			operator++(int)
		{
			return raw++;
		}

		inline pointer_iterator&
			operator--()
		{
			--raw;
			return *this;
		}
		pointer_iterator
			operator--(int)
		{
			return raw--;
		}

		reference
			operator[](difference_type n) const
		{
			lconstraint(raw);

			return raw[n];
		}

		lconstfn pointer_iterator
			operator+(difference_type n) const
		{
			return pointer_iterator(raw + n);
		}

		lconstfn pointer_iterator
			operator-(difference_type n) const
		{
			return pointer_iterator(raw - n);
		}

		lconstfn
			operator pointer() const
		{
			return raw;
		}
	};

	template<class _Ty,
		size_t _Size> inline
		pointer_iterator<_Ty> begin(_Ty(&_Array)[_Size])
	{	// get beginning of array
		return (_Array);
	}

	template<class _Ty,
		size_t _Size> inline
		pointer_iterator<_Ty> end(_Ty(&_Array)[_Size])
	{	// get ending of array
		return (_Array+_Size);
	}

	template<typename _type>
	inline bool
		operator==(const pointer_iterator<_type>& x, const pointer_iterator<_type>& y)
	{
		using pointer = typename pointer_iterator<_type>::pointer;

		return pointer(x) == pointer(y);
	}

	template<typename _type>
	inline bool
		operator!=(const pointer_iterator<_type>& x, const pointer_iterator<_type>& y)
	{
		return !(x == y);
	}

	template<typename _type>
	struct pointer_classify
	{
		using type = _type;
	};

	template<typename _type>
	struct pointer_classify<_type*>
	{
		using type = pointer_iterator<_type>;
	};

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

	template<typename _tIter, typename _fTransformer>
	//转换迭代器
	class transformed_iterator
		: public pointer_classify<_tIter>::type
	{
	public:
		using iterator_type = typename
			pointer_classify<remove_reference_t<_tIter>>::type;
		using transformer_type = decay_t<_fTransformer>;
		using transformed_type = result_of_t<_fTransformer&(_tIter&)>;
		using difference_type
			= typename pointer_classify<_tIter>::type::difference_type;
		using reference = decltype(std::declval<transformed_type>());

	protected:
		//note 当为空类时作为第一个成员可启用空基类优化。
		mutable transformer_type transformer;

	public:
		transformed_iterator() = default;
		template<typename _tIterOrig, typename _tTran,
			limpl(typename = exclude_self_ctor_t<transformed_iterator, _tIterOrig>)>
			explicit lconstfn
			transformed_iterator(_tIterOrig&& i, _tTran f = {})
			: iterator_type(lforward(i)), transformer(f)
		{}
		transformed_iterator(const transformed_iterator&) = default;

		transformed_iterator(transformed_iterator&&) = default;

		inline reference
			operator*() const
		{
			return lforward(transformer(get()));
		}

		template<typename _tDiff>
		enable_if_t<is_convertible<decltype(
			std::declval<iterator_type&>()[_tDiff()]), reference>::value, reference>
			operator[](_tDiff n)
		{
			return get()[difference_type(n)];
		}

		inline
			operator iterator_type&()
		{
			return *this;
		}

		lconstfn
			operator const iterator_type&() const
		{
			return *this;
		}

		inline iterator_type&
			get()
		{
			return *this;
		}

		lconstfn const iterator_type&
			get() const
		{
			return *this;
		}

		transformer_type&
			get_transformer() lnothrow
		{
			return transformer;
		}
		lconstfn const transformer_type&
			get_transformer() const lnothrow
		{
			return transformer;
		}
	};

	template<typename _type, typename _fTransformer>
	inline bool
		operator==(const transformed_iterator<_type, _fTransformer>& x,
		const transformed_iterator<_type, _fTransformer>& y)
	{
		return x.get() == y.get();
	}

	template<typename _type, typename _fTransformer>
	inline bool
		operator!=(const transformed_iterator<_type, _fTransformer>& x,
		const transformed_iterator<_type, _fTransformer>& y)
	{
		return !(x == y);
	}

	template<typename _type, typename _fTransformer>
	inline transformed_iterator<_type, _fTransformer>&
		operator+=(transformed_iterator<_type, _fTransformer>& i,
		typename transformed_iterator<_type, _fTransformer>::difference_type n)
	{
		i.get() += n;
		return i;
	}

	template<typename _type, typename _fTransformer>
	inline transformed_iterator<_type, _fTransformer>&
		operator-=(transformed_iterator<_type, _fTransformer>& i,
		typename transformed_iterator<_type, _fTransformer>::difference_type n)
	{
		i.get() -= n;
		return i;
	}

	template<typename _type, typename _fTransformer>
	transformed_iterator<_type, _fTransformer>
		operator+(const transformed_iterator<_type, _fTransformer>& i,
		typename transformed_iterator<_type, _fTransformer>::difference_type n)
	{
		auto it(i);

		it += n;
		return it;
	}

	template<typename _type, typename _fTransformer>
	transformed_iterator<_type, _fTransformer>
		operator-(const transformed_iterator<_type, _fTransformer>& i,
		typename transformed_iterator<_type, _fTransformer>::difference_type n)
	{
		auto it(i);

		it -= n;
		return it;
	}
	template<typename _type, typename _fTransformer>
	typename transformed_iterator<_type, _fTransformer>::difference_type
		operator-(const transformed_iterator<_type, _fTransformer>& x,
		const transformed_iterator<_type, _fTransformer>& y)
	{
		return x.get() - y.get();
	}

	template<typename _type, typename _fTransformer>
	inline bool
		operator<(const transformed_iterator<_type, _fTransformer>& x,
		const transformed_iterator<_type, _fTransformer>& y)
	{
		return bool(x.get() < y.get());
	}

	template<typename _type, typename _fTransformer>
	inline bool
		operator<=(const transformed_iterator<_type, _fTransformer>& x,
		const transformed_iterator<_type, _fTransformer>& y)
	{
		return !(y < x);
	}

	template<typename _type, typename _fTransformer>
	inline bool
		operator>(const transformed_iterator<_type, _fTransformer>& x,
		const transformed_iterator<_type, _fTransformer>& y)
	{
		return y < x;
	}

	template<typename _type, typename _fTransformer>
	inline bool
		operator>=(const transformed_iterator<_type, _fTransformer>& x,
		const transformed_iterator<_type, _fTransformer>& y)
	{
		return !(x < y);
	}

	template<typename _tIter, typename _fTransformer>
	inline transformed_iterator<typename array_ref_decay<_tIter>::type,
		_fTransformer>
		make_transform(_tIter&& i, _fTransformer f)
	{
		return transformed_iterator<typename array_ref_decay<_tIter>::type,
			_fTransformer>(lforward(i), f);
	}

	namespace iterator_transformation
	{
		template<typename _tIter>
		static lconstfn auto
			first(const _tIter& i) -> decltype((i->first))
		{
			return i->first;
		}
		template<typename _tIter>
		static lconstfn auto
			second(const _tIter& i) -> decltype((i->second))
		{
			return i->second;
		}

		template<typename _tIter>
		static lconstfn auto
			get(const _tIter& i) -> decltype(((*i).get()))
		{
			return (*i).get();
		}

		template<typename _tIter>
		static lconstfn auto
			indirect(const _tIter& i) -> decltype((**i))
		{
			return **i;
		}
	} // namespace iterator_transformation;

	lconstexpr first_tag get_first{}, get_key{};
	lconstexpr second_tag get_second{}, get_value{};
	lconstexpr struct indirect_tag{} get_indirect{};
	lconstexpr const struct get_tag {} get_get{};

	template<typename _tIter>
	inline auto
		operator|(_tIter&& i, first_tag)
		-> decltype(make_transform(lforward(i), iterator_transformation::first<
		typename array_ref_decay<_tIter>::type>))
	{
		return make_transform(lforward(i), iterator_transformation::first<
			typename array_ref_decay<_tIter>::type>);
	}
	template<typename _tIter>
	inline auto
		operator|(_tIter&& i, second_tag)
		-> decltype(make_transform(lforward(i), iterator_transformation::second<
		typename array_ref_decay<_tIter>::type>))
	{
		return make_transform(lforward(i), iterator_transformation::second<
			typename array_ref_decay<_tIter>::type>);
	}
	template<typename _tIter>
	inline auto
		operator|(_tIter&& i, indirect_tag)
		-> decltype(make_transform(lforward(i), iterator_transformation::indirect<
		typename array_ref_decay<_tIter>::type>))
	{
		return make_transform(lforward(i), iterator_transformation::indirect<
			typename array_ref_decay<_tIter>::type>);
	}
	template<typename _tIter>
	inline auto
		operator|(_tIter&& i, get_tag)
		-> decltype(make_transform(lforward(i), iterator_transformation::get<
			typename array_ref_decay<_tIter>::type>))
	{
		return make_transform(lforward(i), iterator_transformation::get<
			typename array_ref_decay<_tIter>::type>);
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
			//	operator bool() const ynoexcept((!is_undereferenceable(std::declval<
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