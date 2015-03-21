#ifndef IndePlatforma_path_hpp
#define IndePlatforma_path_hpp

#include "string.hpp"
#include "memory.hpp"

#include <string>
#include <algorithm>
#include <typeinfo>

namespace leo
{
	template<typename _type>
	class path_norm : public cloneable
	{
	public:
		using value_type = _type;

		virtual
			~path_norm() = default;

		virtual bool
			is_compatible_with(const path_norm&) const lnothrow
		{
			return true;
		}

		virtual bool
			is_delimiter(const value_type&) = 0;

		virtual bool
			is_parent(const value_type&) lnothrow = 0;

		virtual bool
			is_root(const value_type&) lnothrow = 0;

		virtual bool
			is_self(const value_type&) lnothrow = 0;

		//	virtual bool
		//	is_wildcard(const value_type&) ynothrow = 0;

		virtual path_norm*
			clone() const override = 0;
	};

	template<typename _type>
	class file_path_norm;

	template<typename _tChar, class _tAlloc>
	class file_path_norm<std::basic_string<_tChar, _tAlloc>>
		: public path_norm<std::string>
	{
	public:
		using value_type = std::basic_string<_tChar, _tAlloc>;

		bool
			is_delimiter(const value_type& str) override
		{
			return str.length() == 1 && str[0] == '/';
		}

		bool
			is_parent(const value_type& str) lnothrow override
		{
			return str.length() == 2 && str[0] == '.' && str[1] == '.';
		}

			bool
			is_root(const value_type& str) lnothrow override
		{
			return str.empty();
		}

			bool
			is_self(const value_type& str) lnothrow override
		{
			return str.length() == 1 && str[0] == '.';
		}

			file_path_norm*
			clone() const override
		{
			return new file_path_norm(*this);
		}
	};

	template<class _tSeqCon,
	class _tNorm = leo::path_norm<typename _tSeqCon::value_type >>
	class path : private sequence_container_adaptor<_tSeqCon>
	{
	private:
		using base = sequence_container_adaptor<_tSeqCon>;

	public:
		using value_type = typename _tSeqCon::value_type;
		using norm = _tNorm;
		using default_norm = leo::conditional_t<std::is_default_constructible<
			norm>::value, norm, file_path_norm<value_type >> ;
		using reference = typename _tSeqCon::reference;
		using const_reference = typename _tSeqCon::const_reference;
		using size_type = typename base::size_type;
		using difference_type = typename base::difference_type;
		using iterator = typename base::iterator;
		using const_iterator = typename base::const_iterator;
		using reverse_iterator = typename base::container_type::reverse_iterator;
		using const_reverse_iterator
			= typename base::container_type::const_reverse_iterator;

		std::unique_ptr<norm> p_norm;

	public:
		path(std::unique_ptr<norm> p = std::unique_ptr<norm>())
			: path(base(), std::move(p))
		{}
		explicit
			path(base&& b, std::unique_ptr<norm> p = std::unique_ptr<norm>())
			: base(std::move(b)), p_norm(unique_norm(p))
		{}
		explicit
			path(size_type n, std::unique_ptr<norm> p = std::unique_ptr<norm>())
			: path(base(n), std::move(p))
		{}
		path(size_type n, value_type root,
			std::unique_ptr<norm> p = std::unique_ptr<norm>())
			: path(n != 0 ? n : 1, std::move(p))
		{
			this->begin() = std::move(root);
		}
		template<typename _tIn>
		path(_tIn first, _tIn last,
			std::unique_ptr<norm> p = std::unique_ptr<norm>())
			: path(base(first, last), std::move(p))
		{}
		template<typename... _tParams>
		path(std::unique_ptr<norm> p, _tParams&&... args)
			: path(base(lforward(args)...), std::move(p))
		{}
		path(const path& pth)
			: base(pth), p_norm(pth.get_norm().clone())
		{}
		path(path&& pth) lnothrow
			: path()
		{
			pth.swap(*this);
		}
		path(std::initializer_list<value_type> il,
			std::unique_ptr<norm> p = std::unique_ptr<norm>())
			: path(base(il), std::move(p))
		{}

		path&
			operator=(const path& pth)
		{
			return *this = path(pth);
		}
		path&
			operator=(path&& pth) lnothrow
		{
			pth.swap(*this);
			return *this;
		}
			path&
			operator=(std::initializer_list<value_type> il)
		{
			return *this = path(il);
		}

		using base::begin;

		using base::end;

		using base::cbegin;

		using base::cend;

		using base::container_type::rbegin;

		using base::container_type::rend;

		using base::container_type::crbegin;

		using base::container_type::crend;

		using base::size;

		using base::max_size;

		using base::empty;

		using base::back;

		//	using base::emplace;

		//	using base::emplace_back;

		using base::front;

		using base::pop_back;

		using base::push_back;

		using base::insert;

		using base::erase;

		using base::clear;

		using base::assign;

		bool
			before(const path& pth) const
		{
			return typeid(get_norm()).before(typeid(pth.get_norm()))
				&& static_cast<const base&>(*this) < static_cast<const base&>(pth);
		}

		bool
			equals(const path& pth) const
		{
			return typeid(get_norm()) == typeid(pth.get_norm())
				&& static_cast<const base&>(*this) == static_cast<const base&>(pth);
		}

		void
			filter_self()
		{
			auto& nm(get_norm());

			leo::erase_all_if(*this, [&](const value_type& s){
				return nm.is_self(s);
			});
		}

		norm&
			get_norm() const lnothrow
		{
			lconstraint(p_norm);
			return *p_norm;
		}

			value_type
			get_root() const
		{
			return is_absolute() ? front() : value_type();
		}

		bool
			is_absolute() const lnothrow
		{
			return !empty() && get_norm().is_root(front());
		}

			bool
			is_relative() const lnothrow
		{
			return !is_absolute();
		}

			void
			merge_parents()
		{
			auto& nm(get_norm());

			for (auto i(this->begin()); i != this->end();)
			{
				auto j(std::adjacent_find(i, this->end(),
					[&](const value_type& x, const value_type& y){
					return !nm.is_self(x) && !nm.is_parent(x)
						&& nm.is_parent(y);
				}));

				if (j == this->end())
					break;
				i = j++;
				lassume(j != this->end());
				i = erase(i, ++j);
			}
		}

		void
			swap(path& pth) lnothrow
		{
			base::swap(static_cast<base&>(pth)),
			p_norm.swap(pth.p_norm);
		}

	private:
		static std::unique_ptr<norm>
			unique_norm(std::unique_ptr<norm>& p)
		{
			return p ? std::move(p) : make_unique<default_norm>();
		}
	};

	template<class _tSeqCon, class _tNorm>
	inline bool
		operator==(const path<_tSeqCon, _tNorm>& x, const path<_tSeqCon, _tNorm>& y)
	{
		return x.equals(y);
	}

	template<class _tSeqCon, class _tNorm>
	bool
		operator!=(const path<_tSeqCon, _tNorm>& x, const path<_tSeqCon, _tNorm>& y)
	{
		return !(x == y);
	}

	template<class _tSeqCon, class _tNorm>
	bool
		operator<(const path<_tSeqCon, _tNorm>& x, const path<_tSeqCon, _tNorm>& y)
	{
		return x.before(y);
	}

	template<class _tSeqCon, class _tNorm>
	bool
		operator<=(const path<_tSeqCon, _tNorm>& x, const path<_tSeqCon, _tNorm>& y)
	{
		return !(y < x);
	}

	template<class _tSeqCon, class _tNorm>
	bool
		operator>(const path<_tSeqCon, _tNorm>& x, const path<_tSeqCon, _tNorm>& y)
	{
		return y < x;
	}

	template<class _tSeqCon, class _tNorm>
	bool
		operator>=(const path<_tSeqCon, _tNorm>& x, const path<_tSeqCon, _tNorm>& y)
	{
		return !(x < y);
	}

	template<class _tSeqCon, class _tNorm>
	inline void
		normalize(path<_tSeqCon, _tNorm>& pth)
	{
		pth.filter_self(), pth.merge_parents();
	}

	template<class _tSeqCon, class _tNorm>
	inline void
		swap(path<_tSeqCon, _tNorm>& x, path<_tSeqCon, _tNorm>& y)
	{
		x.swap(y);
	}

	template<class _tSeqCon, class _tNorm>
	typename _tSeqCon::value_type
		to_string(const path<_tSeqCon, _tNorm>& pth,
		const typename _tSeqCon::value_type& seperator = &to_array<
		typename string_traits<typename _tSeqCon::value_type>::value_type>("/")[0])
	{
		static_assert(is_object<typename _tSeqCon::value_type>::value,
			"Invalid type found.");

		if (pth.empty())
			return{};

		auto i(pth.begin());
		typename _tSeqCon::value_type res(*i);

		while (++i != pth.end())
		{
			res += seperator;
			res += *i;
		}
		return res;
	}

	template<class _tSeqCon, class _tNorm>
	typename _tSeqCon::value_type
		to_string_d(const path<_tSeqCon, _tNorm>& pth, typename string_traits<typename
		_tSeqCon::value_type>::value_type delimiter = '/')
	{
		static_assert(is_object<typename _tSeqCon::value_type>::value,
			"Invalid type found.");
		typename _tSeqCon::value_type res;

		for (const auto& s : pth)
		{
			res += s;
			res += delimiter;
		}
		return res;
	}
}

#endif