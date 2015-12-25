#ifndef IndePlatform_cache_hpp
#define IndePlatform_cache_hpp

#include "LAssert.h" // for std::pair, is_undereferenceable , lassume;
#include "type_op.hpp"
#include <list> // for std::list;
#include <unordered_map> // for std::unordered_map;
#include <map> // for std::map;
#include <stdexcept> // for std::runtime_error;

namespace leo
{
	//@{
	/*!
	\brief 使用双向链表实现的最近使用列表。
	\note 保证移除项时的顺序为最远端先移除。
	*/
	template<typename _tKey, typename _tMapped,
	class _tAlloc = std::allocator<std::pair<const _tKey, _tMapped>>>
	class recent_used_list : private std::list<std::pair<const _tKey, _tMapped>>
	{
	public:
		using allocator_type = _tAlloc;
		using value_type = std::pair<const _tKey, _tMapped>;
		using list_type = std::list<value_type>;
		using size_type = typename list_type::size_type;
		using const_iterator = typename list_type::const_iterator;
		using iterator = typename list_type::iterator;

		using list_type::begin;

		using list_type::cbegin;

		using list_type::cend;

		using list_type::clear;

		template<typename... _tParams>
		iterator
			emplace(_tParams&&... args)
		{
			list_type::emplace_front(lforward(args)...);
			return begin();
		}

		using list_type::empty;

		using list_type::end;

		using list_type::front;

		iterator
			refresh(iterator i) lnothrowv
		{
			list_type::splice(begin(), *this, i);
			return i;
		}

		template<typename _tMap>
		void
			shrink(_tMap& m) lnothrowv
		{
			lassume(!empty());

			auto& val(list_type::back());

			m.erase(val.first);
			list_type::pop_back();
		}
		template<typename _tMap, typename _func>
		void
			shrink(_tMap& m, _func f)
		{
			lassume(!empty());

			auto& val(list_type::back());

			if (f)
				f(val);
			m.erase(val.first);
			list_type::pop_back();
		}
		template<typename _tMap, typename... _tParams>
		void
			shrink(_tMap& m, size_type max_use, _tParams&&... args)
		{
			while (max_use < size_type(m.size()))
			{
				lassume(!m.empty());

				shrink(m, lforward(args)...);
			}
		}

		void
			undo_emplace() lnothrow
		{
			lassume(!empty());

			list_type::pop_front();
		}
	};


	//! \brief 最近刷新策略的缓存特征。
	//@{
	template<typename _tKey, typename _tMapped, typename _fHash = std::hash<_tKey>,
	class _tAlloc = std::allocator<std::pair<const _tKey, _tMapped>>,
	class _tList = recent_used_list<_tKey, _tMapped, _tAlloc >>
	struct used_list_cache_traits
	{
		using map_type = std::unordered_map<_tKey, _tMapped, _fHash,
			std::equal_to<_tKey>, _tAlloc>;
		using used_list_type = _tList;
		using used_cache_type = std::unordered_multimap<_tKey,
			typename _tList::iterator, _fHash, typename map_type::key_equal,
			typename _tAlloc::template
			rebind<std::pair<const _tKey, typename _tList::iterator>>::other>;
	};

	template<typename _tKey, typename _tMapped, class _tAlloc, class _tList>
	struct used_list_cache_traits<_tKey, _tMapped, void, _tAlloc, _tList>
	{
		using map_type = std::map<_tKey, _tMapped, _tAlloc>;
		using used_list_type = _tList;
		using used_cache_type = std::map<_tKey, typename _tList::iterator,
			std::less<_tKey>, typename _tAlloc::template
			rebind<std::pair<const _tKey, typename _tList::iterator>>::other>;
	};
	//@}


	/*!
	\brief 按最近使用策略刷新的缓存。
	\note 默认策略替换最近最少使用的项，保留其它项。
	\todo 加入异常安全的复制构造函数和复制赋值操作符。
	\todo 支持其它刷新策略。
	*/
	template<typename _tKey, typename _tMapped, typename _fHash = void,
		typename _tTraits = used_list_cache_traits<_tKey, _tMapped, _fHash >>
	class used_list_cache
	{
	public:

		using traits_type = _tTraits;
		using map_type = typename traits_type::map_type;
		using key_type = typename map_type::key_type;

		using value_type = typename map_type::value_type;
		using size_type = typename map_type::size_type;
		using used_list_type = typename traits_type::used_list_type;
		using used_cache_type = typename traits_type::used_cache_type;
		using const_iterator = typename used_list_type::const_iterator;
		using iterator = typename used_list_type::iterator;

		static_assert(and_<is_same<size_type, typename used_list_type::size_type>,
			is_same<size_type, typename used_cache_type::size_type >>::value,
			"Invalid size found.");

	private:
		/*!
		\invariant <tt>std::count(used_list.begin(), used_list.end())
		== used_cache.size()</tt> 。
		*/
		//@{
		mutable used_list_type used_list;
		used_cache_type used_cache;
		//@}
		//! \brief 保持可以再增加一个缓存项的最大容量。
		size_type max_use;

	public:
		std::function<void(value_type&)> flush{};

		explicit
			used_list_cache(size_type s = yimpl(15U))
			: used_list(), used_cache(), max_use(s)
		{}
		used_list_cache(used_list_cache&&) = default;

	private:
		void
			check_max_used() lnothrowv
		{
			used_list.shrink(used_cache, max_use, flush);
		}

	public:
		size_type
			get_max_use() const lnothrow
		{
			return max_use;
		}

		void
			set_max_use(size_type s) lnothrowv
		{
			max_use = s < 1 ? 1 : s;
			check_max_used();
		}

		//@{
		iterator
			begin()
		{
			return used_list.begin();
		}
		iterator
			begin() const
		{
			return used_list.cbegin();
		}
		//@}

		void
			clear() lnothrow
		{
			used_list.clear(),
				used_cache.clear();
		}

		template<typename... _tParams>
		std::pair<iterator, bool>
			emplace(const key_type& k, _tParams&&... args)
		{
			check_max_used();

			const auto i_cache(used_cache.find(k));

			if (i_cache == used_cache.end())
			{
				const auto i(used_list.emplace(k, lforward(args)...));

				try
				{
					used_cache.emplace(k, i);
				}
				catch (...)
				{
					used_list.undo_emplace();
					throw;
				}
				return{ i, true };
			}
			else
				return{ i_cache->second, false };
		}
		template<typename... _tParams>
		std::pair<iterator, bool>
			emplace(_tParams&&... args)
		{
			check_max_used();

			const auto i(used_list.emplace_front(lforward(args)...));

			try
			{
				const auto pr(used_cache.emplace(i->first, i));

				if (!pr.second)
				{
					used_list.undo_emplace();
					return{ pr.first, false };
				}
			}
			catch (...)
			{
				used_list.undo_emplace();
				throw;
			}
			return{ i, true };
		}

		//@{
		iterator
			end()
		{
			return used_list.end();
		}
		iterator
			end() const
		{
			return used_list.cend();
		}
		//@}

		iterator
			find(const key_type& k)
		{
			const auto i(used_cache.find(k));

			return i != used_cache.end() ? used_list.refresh(i->second) : end();
		}
		const_iterator
			find(const key_type& k) const
		{
			const auto i(used_cache.find(k));

			return i != used_cache.end() ? used_list.refresh(i->second) : end();
		}

		size_type
			size() const lnothrow
		{
			return used_cache.size();
		}
	};
	//@}


	/*!
	\brief 以指定的关键字查找作为缓存的无序关联容器，
	若没有找到使用指定的可调用对象和参数初始化内容。
	\tparam _tMap 映射类型，可以是 std::map_type 、 std::unordered_map
	或 leo::used_list_cache 等。
	*/
	template<class _tMap, typename _tKey, typename _func, typename... _tParams>
	auto
		cache_lookup(_tMap& cache, const _tKey& key, _func init, _tParams&&... args)
		-> decltype((cache.begin()->second))
	{
		auto i(cache.find(key));

		if (i == cache.end())
		{
			const auto pr(cache.emplace(key, init(lforward(args)...)));

			if (YB_UNLIKELY(!pr.second))
				throw std::runtime_error("Cache insertion failed.");
			i = pr.first;
		}
		return i->second;
	}

} // namespace leo;

#endif
