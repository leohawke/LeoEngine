#ifndef IndePlatform_container_hpp
#define IndePlatform_container_hpp

#include "functional.hpp"
#include "utility.hpp"
#include <array>
#include <algorithm>

namespace leo
{
	template<class _tSeqCon>
	//ref ISO C++11 23.6[container.adaptors],
	//ref 23.2.1[container.requirements.general]
	class container_adaptor : protected _tSeqCon
	{
	protected:
		using container_type = _tSeqCon;

	private:
		using base = container_type;

	public:
		using value_type = typename container_type::value_type;
		using reference = typename container_type::reference;
		using const_reference = typename container_type::const_reference;
		using iterator = typename container_type::iterator;
		using const_iterator = typename container_type::const_iterator;
		using difference_type = typename container_type::difference_type;
		using size_type = typename container_type::size_type;

		container_adaptor() = default;
		explicit
			container_adaptor(size_type n)
			: base(n)
		{}
		container_adaptor(size_type n, const value_type& value)
			: base(n, value)
		{}
		template<class _tIn>
		container_adaptor(_tIn first, _tIn last)
			: base(std::move(first), std::move(last))
		{}
		container_adaptor(const container_adaptor&) = default;
#if LB_IMPL_MSCPP
		container_adaptor(container_adaptor&& con)
			: base(static_cast<base&&>(con))
		{}
#else
		container_adaptor(container_adaptor&&) = default;
#endif
		container_adaptor(std::initializer_list<value_type> il)
			: base(il)
		{};

		container_adaptor&
			operator=(const container_adaptor&) = default;
		container_adaptor&
#if LB_IMPL_MSCPP
			operator=(container_adaptor&& con)
		{
			static_cast<base&>(*this) = static_cast<base&&>(con);
			return *this;
		}
#else
			operator=(container_adaptor&&) = default;
#endif
		container_adaptor&
			operator=(std::initializer_list<value_type> il)
		{
			base::operator=(il);
		}

		friend bool
			operator==(const container_adaptor& x, const container_adaptor& y)
		{
			return static_cast<const container_type&>(x)
				== static_cast<const container_type&>(y);
		}

		using container_type::begin;

		using container_type::end;

		using container_type::cbegin;

		using container_type::cend;

		void
			swap(container_adaptor& c) lnothrow
		{
			return base::swap(static_cast<container_type&>(c));
		}

		using base::size;

		using base::max_size;

		using base::empty;
	};

	template<class _tSeqCon>
	inline bool
		operator!=(const container_adaptor<_tSeqCon>& x,
		const container_adaptor<_tSeqCon>& y)
	{
		return !(x == y);
	}

	template<class _tSeqCon>
	void
		swap(container_adaptor<_tSeqCon>& x,
		container_adaptor<_tSeqCon>& y) lnothrow
	{
		x.swap(y);
	}

	template<class _tSeqCon>
	//ref container_adaptor
	//ref 23.2.3[sequence.reqmts]
	//序列容器适配器
	class sequence_container_adaptor : protected container_adaptor<_tSeqCon>
	{
	private:
		using base = container_adaptor<_tSeqCon>;

	public:
		using container_type = typename base::container_type;
		using value_type = typename container_type::value_type;
		using size_type = typename container_type::size_type;

		sequence_container_adaptor() = default;
		explicit
			sequence_container_adaptor(size_type n)
			: base(n)
		{}
		sequence_container_adaptor(size_type n, const value_type& value)
			: base(n, value)
		{}
		template<class _tIn>
		sequence_container_adaptor(_tIn first, _tIn last)
			: base(std::move(first), std::move(last))
		{}
		sequence_container_adaptor(const sequence_container_adaptor&) = default;
#if LB_IMPL_MSCPP
		sequence_container_adaptor(sequence_container_adaptor&& con)
			: base(static_cast<base&&>(con))
		{}
#else
		sequence_container_adaptor(sequence_container_adaptor&&) = default;
#endif
		sequence_container_adaptor(std::initializer_list<value_type> il)
			: base(il)
		{};

		sequence_container_adaptor&
			operator=(const sequence_container_adaptor&) = default;
		sequence_container_adaptor&
#if LB_IMPL_MSCPP
			operator=(sequence_container_adaptor&& con)
		{
			static_cast<base&>(*this) = static_cast<base&&>(con);
			return *this;
		}
#else
			operator=(sequence_container_adaptor&&) = default;
#endif
		sequence_container_adaptor&
			operator=(std::initializer_list<value_type> il)
		{
			base::operator=(il);
		}

		friend bool
			operator==(const sequence_container_adaptor& x,
			const sequence_container_adaptor& y)
		{
			return static_cast<const container_type&>(x)
				== static_cast<const container_type&>(y);
		}

		//	using container_type::emplace;

		using container_type::insert;

		using container_type::erase;

		using container_type::clear;

		using container_type::assign;
	};

	template<class _tSeqCon>
	inline bool
		operator!=(const sequence_container_adaptor<_tSeqCon>& x,
		const sequence_container_adaptor<_tSeqCon>& y)
	{
		return !(x == y);
	}

	template<class _tSeqCon>
	void
		swap(sequence_container_adaptor<_tSeqCon>& x,
		sequence_container_adaptor<_tSeqCon>& y) lnothrow
	{
		x.swap(y);
	}

	template<class _tCon, typename... _tParams>
	inline void
		//插入参数指定的元素到容器
		assign(_tCon& c, _tParams&&... args)
	{
		c.assign(lforward(args)...);
	}
	template<class _tCon, typename _type, size_t _vN>
	inline void
		assign(_tCon& c, const _type(&arr)[_vN])
	{
		c.assign(arr, arr + _vN);
	}

	template<typename _tCon>
	//容器插入函数对象
	class container_inserter
	{
	public:
		using container_type = _tCon;

	protected:
		_tCon* container;

	public:
		container_inserter(_tCon& c)
			: container(&c)
		{}

		template<typename... _tParams>
		auto
			operator()(_tParams&&... args)
			-> decltype(container->insert(std::forward<_tParams>(args)...))
			// NOTE: Nested %decltype could cause crashing of devkitPro G++ 4.7.1.
		{
			lassume(container);
			return container->insert(lforward(args)...);
		}
	};

	template<typename _tCon, typename... _tParams>
	inline void
		//顺序插入值至指定容器
		seq_insert(_tCon& c, _tParams&&... args)
	{
		leo::seq_apply(container_inserter<_tCon>(c), lforward(args)...);
	}

	namespace details
	{

		template<class _tCon, typename _tKey>
		bool
			exists(const _tCon& con, const _tKey& key,
			decltype(std::declval<_tCon>().count()) = 1U)
		{
			return con.count(key) != 0;
		}
		template<class _tCon, typename _tKey>
		bool
			exists(const _tCon& con, const _tKey& key, ...)
		{
			return con.find(key) != end(con);
		}

	} // namespace details;

	template<class _tCon, typename _tKey>
	inline bool
		//判断指定容器中存在指定的键
		//当容器右值可使用以整数初始化的类型的成员 count()时使用count,否则
		//使用begin,end和成员find实现
		exists(const _tCon& con, const _tKey& key)
	{
		return details::exists(con, key);
	}

	template<typename _tRange>
	void
		erase_all(_tRange& c, const typename _tRange::value_type& val)
	{
		//remove不会移除后续元素
		c.erase(std::remove(begin(c), end(c), val), end(c));
	}

	template<typename _tCon, typename _tFwd, typename _tValue>
	void
		erase_all(_tCon& c, _tFwd first, _tFwd last, const _tValue& value)
	{
		while (first != last)
			if (*first == value)
				c.erase(first++);
			else
				++first;
	}

	template<typename _tRange, typename _fPred>
	void
		erase_all_if(_tRange& c, _fPred pred)
	{
		c.erase(std::remove_if(begin(c), end(c), pred), end(c));
	}

	template<typename _tCon, typename _tFwd, typename _fPred>
	void
		erase_all_if(_tCon& c, _tFwd first, _tFwd last, _fPred pred)
	{
		while (first != last)
			if (pred(*first))
				c.erase(first++);
			else
				++first;
	}

	template<typename _tRandom>
	inline _tRandom
		//return std::unique(std::sort)
		sort_unique(_tRandom first, _tRandom last)
	{
		std::sort(first, last);
		return std::unique(first, last);
	}

	template<class _tCon>
	void
		sort_unique(_tCon& c)
	{
		leo::sort_unique(begin(c), last(c));
		c.erase(leo::sort_unique(begin(c), last(c)), end(c));
	}

	template<class _tMap>
	std::pair<typename _tMap::iterator, bool>
		//first 迭代器
		//second 是否不存在
		//like std::map::operator
		search_map(_tMap& m, const typename _tMap::key_type& k)
	{
		const auto i(m.lower_bound(k));

		return{ i, (i == m.end() || m.key_comp()(k, i->first)) };
	}

	template<typename _type, size_t _vN, typename _tSrc>
	lconstfn std::array<_type, _vN>
		to_array(const _tSrc& src)
	{
		return std::array<_type, _vN>(src);
	}
	template<typename _type, size_t _vN>
	lconstfn std::array<_type, _vN>
		to_array(const std::array<_type, _vN>& src)
	{
		return src;
	}
	template<typename _type, size_t _vN, typename _tSrcElement>
	inline std::array<_type, _vN>
		to_array(const _tSrcElement(&src)[_vN])
	{
		std::array<_type, _vN> arr;

		std::copy_n(std::addressof(src[0]), _vN, std::addressof(arr[0]));
		return arr;
	}
	template<typename _type, size_t _vN, typename _tSrcElement>
	inline std::array<_type, _vN>
		to_array(_tSrcElement(&&src)[_vN])
	{
		std::array<_type, _vN> arr;

		std::copy_n(std::make_move_iterator(std::addressof(src[0])), _vN,
			std::addressof(arr[0]));
		return arr;
	}
}


#endif