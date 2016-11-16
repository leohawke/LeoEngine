#ifndef LBase_set_hpp
#define LBase_set_hpp 1

#include "LBase/functor.hpp"
#include "LBase/iterator.hpp"
#include <map>

namespace leo {


	/*!
	\brief 转移集合元素。
	\post 转移后元素和参数比较相等。
	*/
	template<typename _type>
	lconstfn const _type&
		set_value_move(const _type& x) lnothrow
	{
		return x;
	}


	// XXX: G++ 5.2.0 rejects generic associative lookup in debug mode of %std::map.
	// TODO: Find precise version supporting debug mode.
#if (__cpp_lib_generic_associative_lookup >= 201304 || __cplusplus >= 201402L) \
	&& !(defined(__GLIBCXX__) && defined(_GLIBCXX_DEBUG))
#	define LB_Impl_Set_UseGenericLookup 1
#else
#	define LB_Impl_Set_UseGenericLookup 0
#endif

	namespace details
	{

		template<typename _tWrappedKey, typename _fComp,
			bool = has_mem_is_transparent<_fComp>::value>
			struct tcompare
		{
			_fComp comp;

			tcompare(_fComp c)
				: comp(std::move(c))
			{}

			bool
				operator()(const _tWrappedKey& x, const _tWrappedKey& y) const
			{
				return bool(comp(x.get(), y.get()));
			}
		};

		template<typename _tWrappedKey, typename _fComp>
		struct tcompare<_tWrappedKey, _fComp, true>
			: private tcompare<_tWrappedKey, _fComp, false>
		{
		private:
			using base = tcompare<_tWrappedKey, _fComp, false>;

		public:
			using is_transparent = limpl(void);

			using base::comp;

#if LB_IMPL_CLANGPP
			// NOTE: See https://llvm.org/bugs/show_bug.cgi?id=27504.
			using tcompare<_tWrappedKey, _fComp, false>::tcompare;
#else
			using base::tcompare;
#endif

			using base::operator();
			template<typename _tKey>
			bool
				operator()(const _tWrappedKey& x, const _tKey& y) const
			{
				return bool(comp(x.get(), y));
			}
			template<typename _tKey>
			bool
				operator()(const _tKey& x, const _tWrappedKey& y) const
			{
				return bool(comp(x, y.get()));
			}
		};


		template<typename _type>
		class wrapped_key
		{
		public:
			using key_type = _type;
			using wrapper = lref<const key_type>;

		private:
			mutable wrapper k;

		public:
			wrapped_key(const key_type& key)
				: k(key)
			{}

			void
				amend(const _type& key) const lnothrow
			{
				k = wrapper(key);
			}

			const _type&
				get() const lnothrow
			{
				return k;
			}
		};

	} // namespace details;

	  /*!
	  \brief 允许指定不同于值类型的键的集合。
	  \note 和 std::set 类似，但迭代器可修改，且插入操作要求参数满足 CopyInsertable 。
	  \see WG21 N3456 23.2.4 [associative.reqmts] 。
	  */
	template<typename _type, typename _fComp = less<_type>,
		class _tAlloc = std::allocator<_type>>
		class mapped_set
	{
	public:
		using key_type = _type;
		using value_type = _type;
		using key_compare = _fComp;
		using value_compare = _fComp;
		using allocator_type = _tAlloc;
		using reference = value_type&;
		using const_reference = const value_type&;

	private:
		using mapped_key_type = details::wrapped_key<_type>;
		using mapped_key_compare = details::tcompare<mapped_key_type, _fComp>;
		using umap_type
			= std::map<mapped_key_type, value_type, mapped_key_compare, _tAlloc>;
		using umap_pair = typename umap_type::value_type;
		// NOTE: Here %get_second cannot be used directory due to possible
		//	incomplete value type and requirement on conversion between
		//	'const_iterator' and 'iterator'.
		template<typename _tIter>
		using wrap_iter
			= transformed_iterator<_tIter, iterator_transformation::second<>>;

	public:
		using iterator = limpl(wrap_iter<typename umap_type::iterator>);
		using const_iterator = limpl(wrap_iter<typename umap_type::const_iterator>);
		using size_type = limpl(typename umap_type::size_type);
		using difference_type = limpl(typename umap_type::difference_type);
		using pointer = typename std::allocator_traits<_tAlloc>::pointer;
		using const_pointer
			= typename std::allocator_traits<_tAlloc>::const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	private:
		// XXX: It is undefined behavior when %value_type is incomplete, however
		//	some implementations actually supports this.
		umap_type m_map;

	public:
		mapped_set()
			: mapped_set(_fComp())
		{}
		explicit
			mapped_set(const _fComp& comp, const _tAlloc& a = _tAlloc())
			: m_map(comp, a)
		{}
		template<typename _tIn>
		mapped_set(_tIn first, _tIn last, const _fComp& comp = _fComp(),
			const _tAlloc& a = _tAlloc())
			: m_map(make_transform(first, iter_trans<_tIn>),
				make_transform(last, iter_trans<_tIn>), comp, a)
		{
			amend_all();
		}
		mapped_set(const mapped_set& s)
			: m_map(s.m_map)
		{
			amend_all();
		}
		mapped_set(mapped_set&& s)
			: m_map(std::move(s.m_map))
		{
			amend_all();
		}
		explicit
			mapped_set(const _tAlloc& a)
			: m_map(a)
		{}
		mapped_set(const mapped_set& s, const _tAlloc& a)
			: m_map(s.m_map, a)
		{
			amend_all();
		}
		mapped_set(mapped_set&& s, const _tAlloc& a)
			: m_map(std::move(s), a)
		{
			amend_all();
		}
		mapped_set(std::initializer_list<value_type> il,
			const _fComp& comp = _fComp(), const _tAlloc& a = _tAlloc())
			: mapped_set(il.begin(), il.end(), comp, a)
		{}
		template<typename _tIn>
		mapped_set(_tIn first, _tIn last, const _tAlloc& a)
			: mapped_set(first, last, _fComp(), a)
		{}
		mapped_set(std::initializer_list<value_type> il, const _tAlloc& a)
			: mapped_set(il, _fComp(), a)
		{}
		~mapped_set() limpl(= default);

		mapped_set&
			operator=(const mapped_set& s)
		{
			mapped_set(s).swap(*this);
			return *this;
		}
		mapped_set&
			operator=(mapped_set&& s)
#if false
			// TODO: Blocked. Apply exception specification after published ISO C++17.
			lnoexcept(std::allocator_traits<_tAlloc>::is_always_equal()
				&& is_nothrow_move_assignable<_fComp>())
#endif
		{
			s.swap(*this);
			return *this;
		}
		mapped_set&
			operator=(std::initializer_list<value_type> il)
		{
			mapped_set(il).swap(*this);
			return *this;
		}

		allocator_type
			get_allocator() const lnothrow
		{
			return m_map.get_allocator();
		}

		iterator
			begin() lnothrow
		{
			return iterator(m_map.begin());
		}
		const_iterator
			begin() const lnothrow
		{
			return const_iterator(m_map.begin());
		}

		iterator
			end() lnothrow
		{
			return iterator(m_map.end());
		}
		const_iterator
			end() const lnothrow
		{
			return const_iterator(m_map.end());
		}

		reverse_iterator
			rbegin() lnothrow
		{
			return reverse_iterator(end());
		}
		const_reverse_iterator
			rbegin() const lnothrow
		{
			return const_reverse_iterator(end());
		}

		reverse_iterator
			rend() lnothrow
		{
			return reverse_iterator(begin());
		}
		const_reverse_iterator
			rend() const lnothrow
		{
			return const_reverse_iterator(begin());
		}

		const_iterator
			cbegin() const lnothrow
		{
			return const_iterator(m_map.cbegin());
		}

		const_iterator
			cend() const lnothrow
		{
			return const_iterator(m_map.cend());
		}

		const_reverse_iterator
			crbegin() const lnothrow
		{
			return const_reverse_iterator(end());
		}

		const_reverse_iterator
			crend() const lnothrow
		{
			return const_reverse_iterator(begin());
		}

		bool
			empty() const lnothrow
		{
			return m_map.empty();
		}

		size_type
			size() const lnothrow
		{
			return m_map.size();
		}

		size_type
			max_size() const lnothrow
		{
			return m_map.max_size();
		}

		template<typename... _tParams>
		std::pair<iterator, bool>
			emplace(_tParams&&... args)
		{
			// XXX: %value_type needs to be MoveConstructible, not
			//	EmplaceInsertable.
			return insert(value_type(lforward(args)...));
		}

		//! \note 使用 ADL set_value_move 转移元素。
		template<typename... _tParams>
		iterator
			emplace_hint(const_iterator position, _tParams&&... args)
		{
			// XXX: %value_type needs to be MoveConstructible, not
			//	EmplaceInsertable.
			return insert(position, value_type(lforward(args)...));
		}

		std::pair<iterator, bool>
			insert(const value_type& x)
		{
			// XXX: %value_type needs to be CopyConstructible rather than
			//	CopyInsertable into %mapped_set.
			// XXX: %mapped_key_type and %value_type need to be EmplaceConstructible
			//	into %umap_type.
			const auto res(m_map.emplace(std::piecewise_construct,
				std::forward_as_tuple(mapped_key_type(x)),
				std::forward_as_tuple(x)));

			if (res.second)
				amend_pair(*res.first);
			return{ iterator(res.first), res.second };
		}
		//! \note 使用 ADL set_value_move 转移元素。
		std::pair<iterator, bool>
			insert(value_type&& x)
		{
			// XXX: %mapped_key_type and moved %value_type need to be
			//	EmplaceConstructible into %umap_type.
			const auto res(m_map.emplace(std::piecewise_construct,
				std::forward_as_tuple(mapped_key_type(x)),
				std::forward_as_tuple(set_value_move(x))));

			if (res.second)
				amend_pair(*res.first);
			return{ iterator(res.first), res.second };
		}
		iterator
			insert(const_iterator position, const value_type& x)
		{
			// XXX: %mapped_key_type and %value_type need to be EmplaceConstructible
			//	into %umap_type.
			const auto res(m_map.emplace_hint(position.get(),
				std::piecewise_construct, std::forward_as_tuple(mapped_key_type(x)),
				std::forward_as_tuple(x)));

			// TODO: Blocked. Use C++1z %try_emplace result.
			amend_pair(*res);
			return iterator(res);
		}
		//! \note 使用 ADL set_value_move 转移元素。
		iterator
			insert(const_iterator position, value_type&& x)
		{
			// XXX: %mapped_key_type and moved %value_type need to be
			//	EmplaceConstructible into %umap_type.
			const auto res(m_map.emplace_hint(position.get(),
				std::piecewise_construct, std::forward_as_tuple(mapped_key_type(x)),
				std::forward_as_tuple(set_value_move(x))));

			// TODO: Blocked. Use C++1z %try_emplace result.
			amend_pair(*res);
			return iterator(res);
		}
		template<typename _tIn>
		void
			insert(_tIn first, _tIn last)
		{
			m_map.insert(make_transform(first, iter_trans<_tIn>),
				make_transform(last, iter_trans<_tIn>));
			// XXX: Time complexity.
			amend_all();
		}
		void
			insert(std::initializer_list<value_type> il)
		{
			insert(il.begin(), il.end());
		}

		iterator
			erase(iterator position)
		{
			return iterator(m_map.erase(position.get()));
		}
		iterator
			erase(const_iterator position)
		{
			return iterator(m_map.erase(position.get()));
		}
		
		size_type
			erase(const key_type& x)
		{
			return m_map.erase(mapped_key_type(x));
		}
		iterator
			erase(const_iterator first, const_iterator last)
		{
			return iterator(m_map.erase(first.get(), last.get()));
		}

		void
			swap(mapped_set& s)
#if false
			// TODO: Blocked. Apply exception specification after published ISO C++17.
			lnoexcept(std::allocator_traits<_tAlloc>::is_always_equal()
				&& is_nothrow_swappable<_fComp>())
#endif
		{
			m_map.swap(s.m_map);
		}

		void
			clear() lnothrow
		{
			m_map.clear();
		}

		key_compare
			key_comp() const
		{
			return m_map.key_comp().comp;
		}

		value_compare
			value_comp() const
		{
			return m_map.key_comp().comp;
		}

#define LB_Impl_Set_GenericLookupHead(_n, _r) \
	template<typename _tKey, \
		limpl(typename = enable_if_transparent_t<_fComp, _tKey>)> \
	_r \
	_n(const _tKey& x)

		iterator
			find(const key_type& x)
		{
			return iterator(m_map.find(mapped_key_type(x)));
		}
		const_iterator
			find(const key_type& x) const
		{
			return const_iterator(m_map.find(mapped_key_type(x)));
		}
		LB_Impl_Set_GenericLookupHead(find, iterator)
		{
#if LB_Impl_Set_UseGenericLookup
			return iterator(m_map.find(x));
#else
			const auto i(lower_bound(x));

			return i != end() && !gcomp()(x, *i) ? i : end();
#endif
		}
		LB_Impl_Set_GenericLookupHead(find, const_iterator) const
		{
#if LB_Impl_Set_UseGenericLookup
			return const_iterator(m_map.find(x));
#else
			const auto i(lower_bound(x));

			return i != cend() && !gcomp()(x, *i) ? i : cend();
#endif
		}

		size_type
			count(const key_type& x) const
		{
			return m_map.count(mapped_key_type(x));
		}
		LB_Impl_Set_GenericLookupHead(count, size_type) const
		{
#if LB_Impl_Set_UseGenericLookup
			return m_map.count(x);
#else
			const auto pr(equal_range(x));

			return std::count(pr.first, pr.second, gcomp());
#endif
		}

		iterator
			lower_bound(const key_type& x)
		{
			return iterator(m_map.lower_bound(mapped_key_type(x)));
		}
		const_iterator
			lower_bound(const key_type& x) const
		{
			return const_iterator(m_map.lower_bound(mapped_key_type(x)));
		}
		LB_Impl_Set_GenericLookupHead(lower_bound, iterator)
		{
#if LB_Impl_Set_UseGenericLookup
			return iterator(m_map.lower_bound(x));
#else
			return std::lower_bound(begin(), end(), x, gcomp());
#endif
		}
		LB_Impl_Set_GenericLookupHead(lower_bound, const_iterator) const
		{
#if LB_Impl_Set_UseGenericLookup
			return const_iterator(m_map.lower_bound(x));
#else
			return std::lower_bound(cbegin(), cend(), x, gcomp());
#endif
		}

		iterator
			upper_bound(const key_type& x)
		{
			return iterator(m_map.upper_bound(mapped_key_type(x)));
		}
		const_iterator
			upper_bound(const key_type& x) const
		{
			return const_iterator(m_map.upper_bound(mapped_key_type(x)));
		}
		LB_Impl_Set_GenericLookupHead(upper_bound, iterator)
		{
#if LB_Impl_Set_UseGenericLookup
			return iterator(m_map.upper_bound(x));
#else
			return std::upper_bound(begin(), end(), x, gcomp());
#endif
		}
		LB_Impl_Set_GenericLookupHead(upper_bound, const_iterator) const
		{
#if LB_Impl_Set_UseGenericLookup
			return const_iterator(m_map.upper_bound(x));
#else
			return std::upper_bound(cbegin(), cend(), x, gcomp());
#endif
		}

		std::pair<iterator, iterator>
			equal_range(const key_type& x)
		{
			const auto pr(m_map.equal_range(mapped_key_type(x)));

			return{ iterator(pr.first), iterator(pr.second) };
		}
		std::pair<const_iterator, const_iterator>
			equal_range(const key_type& x) const
		{
			const auto pr(m_map.equal_range(mapped_key_type(x)));

			return{ const_iterator(pr.first), const_iterator(pr.second) };
		}
		LB_Impl_Set_GenericLookupHead(equal_range,
			std::pair<iterator LPP_Comma iterator>)
		{
#if LB_Impl_Set_UseGenericLookup
			const auto pr(m_map.equal_range(x));

			return{ iterator(pr.first), iterator(pr.second) };
#else
			return std::equal_range(begin(), end(), x, gcomp());
#endif
		}
		LB_Impl_Set_GenericLookupHead(equal_range,
			std::pair<const_iterator LPP_Comma const_iterator>) const
		{
#if LB_Impl_Set_UseGenericLookup
			const auto pr(m_map.equal_range(x));

			return{ const_iterator(pr.first), const_iterator(pr.second) };
#else
			return std::equal_range(cbegin(), cend(), x, gcomp());
#endif
		}
#undef LB_Impl_Set_GenericLookupHead

	private:
		void
			amend_all() const
		{
			for (const auto& pr : m_map)
				amend_pair(pr);
		}

		static void
			amend_pair(const umap_pair& pr) lnothrow
		{
			pr.first.amend(pr.second);
		}

#if !LB_Impl_Set_UseGenericLookup
		mapped_key_compare
			gcomp() const
		{
			return m_map.key_comp();
		}
#endif

		template<typename _tIter>
		static umap_pair
			iter_trans(const _tIter& i)
		{
			return{ mapped_key_type(*i), *i };
		}
	};

#undef LB_Impl_Set_UseGenericLookup


}

#endif
