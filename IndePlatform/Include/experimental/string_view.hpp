// Components for manipulating non-owning sequences of characters -*- C++ -*-

// Copyright (C) 2013-2014 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file experimental/string_view
*  This is a Standard C++ Library header.
*/

//
// N3762 basic_string_view library
//

#ifndef _GLIBCXX_EXPERIMENTAL_STRING_VIEW
#define _GLIBCXX_EXPERIMENTAL_STRING_VIEW 1

#pragma GCC system_header

#if __cplusplus <= 201103L
# include <bits/c++14_warning.h>
#else

#include <string>
#include <limits>

namespace std _GLIBCXX_VISIBILITY(default)
{
	namespace experimental
	{
		_GLIBCXX_BEGIN_NAMESPACE_VERSION

			/**
			*  @class basic_string_view <string_view>
			*  @brief  A non-owning reference to a string.
			*
			*  @ingroup strings
			*  @ingroup sequences
			*
			*  @tparam _CharT  Type of character
			*  @tparam _Traits  Traits for character type, defaults to
			*                   char_traits<_CharT>.
			*
			*  A basic_string_view looks like this:
			*
			*  @code
			*    _CharT*    _M_str
			*    size_t     _M_len
			*  @endcode
			*/
			template<typename _CharT, typename _Traits = std::char_traits<_CharT>>
		class basic_string_view
		{
		public:

			// types
			using traits_type = _Traits;
			using value_type = _CharT;
			using pointer = const _CharT*;
			using const_pointer = const _CharT*;
			using reference = const _CharT&;
			using const_reference = const _CharT&;
			using const_iterator = const _CharT*;
			using iterator = const_iterator;
			using const_reverse_iterator = std::reverse_iterator<const_iterator>;
			using reverse_iterator = const_reverse_iterator;
			using size_type = size_t;
			using difference_type = ptrdiff_t;
			static constexpr size_type npos = size_type(-1);

			// [string.view.cons], construct/copy

			constexpr
				basic_string_view() noexcept
				: _M_len{ 0 }, _M_str{ nullptr }
			{ }

			constexpr basic_string_view(const basic_string_view&) noexcept = default;

			template<typename _Allocator>
			basic_string_view(const basic_string<_CharT, _Traits,
				_Allocator>& __str) noexcept
				: _M_len{ __str.length() }, _M_str{ __str.data() }
			{ }

			constexpr basic_string_view(const _CharT* __str)
				: _M_len{ __str == nullptr ? 0 : traits_type::length(__str) },
				_M_str{ __str }
			{ }

			constexpr basic_string_view(const _CharT* __str, size_type __len)
				: _M_len{ __len },
				_M_str{ __str }
			{ }

			basic_string_view&
				operator=(const basic_string_view&) noexcept = default;

			// [string.view.iterators], iterators

			constexpr const_iterator
				begin() const noexcept
			{
				return this->_M_str;
			}

			constexpr const_iterator
				end() const noexcept
			{
				return this->_M_str + this->_M_len;
			}

			constexpr const_iterator
				cbegin() const noexcept
			{
				return this->_M_str;
			}

			constexpr const_iterator
				cend() const noexcept
			{
				return this->_M_str + this->_M_len;
			}

			const_reverse_iterator
				rbegin() const noexcept
			{
				return const_reverse_iterator(this->end());
			}

			const_reverse_iterator
				rend() const noexcept
			{
				return const_reverse_iterator(this->begin());
			}

			const_reverse_iterator
				crbegin() const noexcept
			{
				return const_reverse_iterator(this->end());
			}

			const_reverse_iterator
				crend() const noexcept
			{
				return const_reverse_iterator(this->begin());
			}

			// [string.view.capacity], capacity

			constexpr size_type
				size() const noexcept
			{
				return this->_M_len;
			}

			constexpr size_type
				length() const noexcept
			{
				return _M_len;
			}

			constexpr size_type
				max_size() const noexcept
			{
				return (npos - sizeof(size_type) - sizeof(void*))
					/ sizeof(value_type) / 4;
			}

			constexpr bool
				empty() const noexcept
			{
				return this->_M_len == 0;
			}

			// [string.view.access], element access

			constexpr const _CharT&
				operator[](size_type __pos) const
			{
				// TODO: Assert to restore in a way compatible with the constexpr.
				// _GLIBCXX_DEBUG_ASSERT(__pos < this->_M_len);
				return *(this->_M_str + __pos);
			}

			constexpr const _CharT&
				at(size_type __pos) const
			{
				return __pos < this->_M_len
					? *(this->_M_str + __pos)
					: (__throw_out_of_range_fmt(__N("basic_string_view::at: __pos "
					"(which is %zu) >= this->size() "
					"(which is %zu)"),
					__pos, this->size()),
					*this->_M_str);
			}

			constexpr const _CharT&
				front() const
			{
				// TODO: Assert to restore in a way compatible with the constexpr.
				// _GLIBCXX_DEBUG_ASSERT(this->_M_len > 0);
				return *this->_M_str;
			}

			constexpr const _CharT&
				back() const
			{
				// TODO: Assert to restore in a way compatible with the constexpr.
				// _GLIBCXX_DEBUG_ASSERT(this->_M_len > 0);
				return *(this->_M_str + this->_M_len - 1);
			}

			constexpr const _CharT*
				data() const noexcept
			{
				return this->_M_str;
			}

			// [string.view.modifiers], modifiers:

			void
				clear() noexcept
			{
				this->_M_len = 0;
				this->_M_str = nullptr;
			}

			void
				remove_prefix(size_type __n)
			{
				_GLIBCXX_DEBUG_ASSERT(this->_M_len >= __n);
				this->_M_str += __n;
				this->_M_len -= __n;
			}

			void
				remove_suffix(size_type __n)
			{
				this->_M_len -= __n;
			}

			void
				swap(basic_string_view& __sv) noexcept
			{
				std::swap(this->_M_len, __sv._M_len);
				std::swap(this->_M_str, __sv._M_str);
			}


			// [string.view.ops], string operations:

			template<typename _Allocator>
			explicit operator basic_string<_CharT, _Traits, _Allocator>() const
			{
				return{ this->_M_str, this->_M_len };
			}

			template<typename _Allocator = std::allocator<_CharT>>
			basic_string<_CharT, _Traits, _Allocator>
				to_string(const _Allocator& __alloc = _Allocator()) const
			{
				return{ this->_M_str, this->_M_len, __alloc };
			}

			size_type
				copy(_CharT* __str, size_type __n, size_type __pos = 0) const
			{
				__glibcxx_requires_string_len(__str, __n);
				if (__pos > this->_M_len)
					__throw_out_of_range_fmt(__N("basic_string_view::copy: __pos "
					"(which is %zu) > this->size() "
					"(which is %zu)"),
					__pos, this->size());
				size_type __rlen{ std::min(__n, size_type{ this->_M_len - __pos }) };
				for (auto __begin = this->_M_str + __pos,
					__end = __begin + __rlen; __begin != __end;)
					*__str++ = *__begin++;
				return __rlen;
			}


			// [string.view.ops], string operations:

			constexpr basic_string_view
				substr(size_type __pos, size_type __n = npos) const
			{
				return __pos <= this->_M_len
					? basic_string_view{ this->_M_str + __pos,
					std::min(__n, size_type{ this->_M_len - __pos }) }
					: (__throw_out_of_range_fmt(__N("basic_string_view::substr: __pos "
					"(which is %zu) > this->size() "
					"(which is %zu)"),
					__pos, this->size()), basic_string_view{});
			}

			int
				compare(basic_string_view __str) const noexcept
			{
				int __ret = traits_type::compare(this->_M_str, __str._M_str,
					std::min(this->_M_len, __str._M_len));
				if (__ret == 0)
					__ret = _S_compare(this->_M_len, __str._M_len);
				return __ret;
			}

			int
				compare(size_type __pos1, size_type __n1, basic_string_view __str) const
			{
				return this->substr(__pos1, __n1).compare(__str);
			}

			int
				compare(size_type __pos1, size_type __n1,
				basic_string_view __str, size_type __pos2, size_type __n2) const
			{
				return this->substr(__pos1, __n1).compare(__str.substr(__pos2, __n2));
			}

			int
				compare(const _CharT* __str) const noexcept
			{
				return this->compare(basic_string_view{ __str });
			}

			int
				compare(size_type __pos1, size_type __n1, const _CharT* __str) const
			{
				return this->substr(__pos1, __n1).compare(basic_string_view{ __str });
			}

			int
				compare(size_type __pos1, size_type __n1,
				const _CharT* __str, size_type __n2) const
			{
				return this->substr(__pos1, __n1)
					.compare(basic_string_view(__str, __n2));
			}

			size_type
				find(basic_string_view __str, size_type __pos = 0) const noexcept
			{
				return this->find(__str._M_str, __pos, __str._M_len);
			}

			size_type
				find(_CharT __c, size_type __pos = 0) const noexcept;

			size_type
				find(const _CharT* __str, size_type __pos, size_type __n) const noexcept;

			size_type
				find(const _CharT* __str, size_type __pos = 0) const noexcept
			{
				return this->find(__str, __pos, traits_type::length(__str));
			}

			size_type
				rfind(basic_string_view __str, size_type __pos = npos) const noexcept
			{
				return this->rfind(__str._M_str, __pos, __str._M_len);
			}

			size_type
				rfind(_CharT __c, size_type __pos = npos) const noexcept;

			size_type
				rfind(const _CharT* __str, size_type __pos, size_type __n) const noexcept;

			size_type
				rfind(const _CharT* __str, size_type __pos = npos) const noexcept
			{
				return this->rfind(__str, __pos, traits_type::length(__str));
			}

			size_type
				find_first_of(basic_string_view __str, size_type __pos = 0) const noexcept
			{
				return this->find_first_of(__str._M_str, __pos, __str._M_len);
			}

			size_type
				find_first_of(_CharT __c, size_type __pos = 0) const noexcept
			{
				return this->find(__c, __pos);
			}

			size_type
				find_first_of(const _CharT* __str, size_type __pos, size_type __n) const;

			size_type
				find_first_of(const _CharT* __str, size_type __pos = 0) const noexcept
			{
				return this->find_first_of(__str, __pos, traits_type::length(__str));
			}

			size_type
				find_last_of(basic_string_view __str,
				size_type __pos = npos) const noexcept
			{
				return this->find_last_of(__str._M_str, __pos, __str._M_len);
			}

			size_type
				find_last_of(_CharT __c, size_type __pos = npos) const noexcept
			{
				return this->rfind(__c, __pos);
			}

			size_type
				find_last_of(const _CharT* __str, size_type __pos, size_type __n) const;

			size_type
				find_last_of(const _CharT* __str, size_type __pos = npos) const noexcept
			{
				return this->find_last_of(__str, __pos, traits_type::length(__str));
			}

			size_type
				find_first_not_of(basic_string_view __str,
				size_type __pos = 0) const noexcept
			{
				return this->find_first_not_of(__str._M_str, __pos, __str._M_len);
			}

			size_type
				find_first_not_of(_CharT __c, size_type __pos = 0) const noexcept;

			size_type
				find_first_not_of(const _CharT* __str,
				size_type __pos, size_type __n) const;

			size_type
				find_first_not_of(const _CharT* __str, size_type __pos = 0) const noexcept
			{
				return this->find_first_not_of(__str, __pos,
					traits_type::length(__str));
			}

			size_type
				find_last_not_of(basic_string_view __str,
				size_type __pos = npos) const noexcept
			{
				return this->find_last_not_of(__str._M_str, __pos, __str._M_len);
			}

			size_type
				find_last_not_of(_CharT __c, size_type __pos = npos) const noexcept;

			size_type
				find_last_not_of(const _CharT* __str,
				size_type __pos, size_type __n) const;

			size_type
				find_last_not_of(const _CharT* __str,
				size_type __pos = npos) const noexcept
			{
				return this->find_last_not_of(__str, __pos,
					traits_type::length(__str));
			}

		private:

			static constexpr const int
				_S_compare(size_type __n1, size_type __n2) noexcept
			{
				return difference_type{ __n1 - __n2 } > std::numeric_limits<int>::max()
					? std::numeric_limits<int>::max()
					: difference_type{ __n1 - __n2 } < std::numeric_limits<int>::min()
					? std::numeric_limits<int>::min()
					: static_cast<int>(difference_type{ __n1 - __n2 });
			}

			size_t	    _M_len;
			const _CharT* _M_str;
		};


		// [string.view.comparison], non-member basic_string_view comparison functions

		namespace __detail
		{
			//  Identity transform to make ADL work with just one argument.
			//  See n3766.html.
			template<typename _Tp = void>
			struct __identity
			{
				typedef _Tp type;
			};

			template<>
			struct __identity<void>;

			template<typename _Tp>
			using __idt = typename __identity<_Tp>::type;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator==(basic_string_view<_CharT, _Traits> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return __x.compare(__y) == 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator==(basic_string_view<_CharT, _Traits> __x,
			__detail::__idt<basic_string_view<_CharT, _Traits>> __y) noexcept
		{
			return __x.compare(__y) == 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator==(__detail::__idt<basic_string_view<_CharT, _Traits>> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return __x.compare(__y) == 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator!=(basic_string_view<_CharT, _Traits> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return !(__x == __y);
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator!=(basic_string_view<_CharT, _Traits> __x,
			__detail::__idt<basic_string_view<_CharT, _Traits>> __y) noexcept
		{
			return !(__x == __y);
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator!=(__detail::__idt<basic_string_view<_CharT, _Traits>> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return !(__x == __y);
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator< (basic_string_view<_CharT, _Traits> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return __x.compare(__y) < 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator< (basic_string_view<_CharT, _Traits> __x,
			__detail::__idt<basic_string_view<_CharT, _Traits>> __y) noexcept
		{
			return __x.compare(__y) < 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator< (__detail::__idt<basic_string_view<_CharT, _Traits>> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return __x.compare(__y) < 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator> (basic_string_view<_CharT, _Traits> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return __x.compare(__y) > 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator> (basic_string_view<_CharT, _Traits> __x,
			__detail::__idt<basic_string_view<_CharT, _Traits>> __y) noexcept
		{
			return __x.compare(__y) > 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator> (__detail::__idt<basic_string_view<_CharT, _Traits>> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return __x.compare(__y) > 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator<=(basic_string_view<_CharT, _Traits> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return __x.compare(__y) <= 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator<=(basic_string_view<_CharT, _Traits> __x,
			__detail::__idt<basic_string_view<_CharT, _Traits>> __y) noexcept
		{
			return __x.compare(__y) <= 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator<=(__detail::__idt<basic_string_view<_CharT, _Traits>> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return __x.compare(__y) <= 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator>=(basic_string_view<_CharT, _Traits> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return __x.compare(__y) >= 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator>=(basic_string_view<_CharT, _Traits> __x,
			__detail::__idt<basic_string_view<_CharT, _Traits>> __y) noexcept
		{
			return __x.compare(__y) >= 0;
		}

		template<typename _CharT, typename _Traits>
		inline bool
			operator>=(__detail::__idt<basic_string_view<_CharT, _Traits>> __x,
			basic_string_view<_CharT, _Traits> __y) noexcept
		{
			return __x.compare(__y) >= 0;
		}

		// [string.view.io], Inserters and extractors
		template<typename _CharT, typename _Traits>
		inline basic_ostream<_CharT, _Traits>&
			operator<<(basic_ostream<_CharT, _Traits>& __os,
			basic_string_view<_CharT, _Traits> __str)
		{
			return __ostream_insert(__os, __str.data(), __str.size());
		}


		// basic_string_view typedef names

		using string_view = basic_string_view<char>;
#ifdef _GLIBCXX_USE_WCHAR_T
		using wstring_view = basic_string_view<wchar_t>;
#endif
#ifdef _GLIBCXX_USE_C99_STDINT_TR1
		using u16string_view = basic_string_view<char16_t>;
		using u32string_view = basic_string_view<char32_t>;
#endif

		_GLIBCXX_END_NAMESPACE_VERSION
	} // namespace experimental


	  // [string.view.hash], hash support:

	_GLIBCXX_BEGIN_NAMESPACE_VERSION
		template<typename _Tp>
	struct hash;

	template<>
	struct hash<experimental::string_view>
		: public __hash_base<size_t, experimental::string_view>
	{
		size_t
		operator()(const experimental::string_view& __str) const noexcept
		{
			return std::_Hash_impl::hash(__str.data(), __str.length());
		}
	};

	template<>
	struct __is_fast_hash<hash<experimental::string_view>> : std::false_type
	{};

#ifdef _GLIBCXX_USE_WCHAR_T
	template<>
	struct hash<experimental::wstring_view>
		: public __hash_base<size_t, wstring>
	{
		size_t
		operator()(const experimental::wstring_view& __s) const noexcept
		{
			return std::_Hash_impl::hash(__s.data(),
				__s.length() * sizeof(wchar_t));
		}
	};

	template<>
	struct __is_fast_hash<hash<experimental::wstring_view>> : std::false_type
	{};
#endif

#ifdef _GLIBCXX_USE_C99_STDINT_TR1
	template<>
	struct hash<experimental::u16string_view>
		: public __hash_base<size_t, experimental::u16string_view>
	{
		size_t
		operator()(const experimental::u16string_view& __s) const noexcept
		{
			return std::_Hash_impl::hash(__s.data(),
				__s.length() * sizeof(char16_t));
		}
	};

	template<>
	struct __is_fast_hash<hash<experimental::u16string_view>> : std::false_type
	{};

	template<>
	struct hash<experimental::u32string_view>
		: public __hash_base<size_t, experimental::u32string_view>
	{
		size_t
		operator()(const experimental::u32string_view& __s) const noexcept
		{
			return std::_Hash_impl::hash(__s.data(),
				__s.length() * sizeof(char32_t));
		}
	};

	template<>
	struct __is_fast_hash<hash<experimental::u32string_view>> : std::false_type
	{};
#endif
	_GLIBCXX_END_NAMESPACE_VERSION

	namespace experimental
	{
		_GLIBCXX_BEGIN_NAMESPACE_VERSION

			// I added these EMSR.
			inline namespace literals
		{
			inline namespace string_view_literals
			{

				inline basic_string_view<char>
					operator""sv(const char* __str, size_t __len)
				{
					return basic_string_view<char>{__str, __len};
				}

#ifdef _GLIBCXX_USE_WCHAR_T
				inline basic_string_view<wchar_t>
					operator""sv(const wchar_t* __str, size_t __len)
				{
					return basic_string_view<wchar_t>{__str, __len};
				}
#endif

#ifdef _GLIBCXX_USE_C99_STDINT_TR1
				inline basic_string_view<char16_t>
					operator""sv(const char16_t* __str, size_t __len)
				{
					return basic_string_view<char16_t>{__str, __len};
				}

				inline basic_string_view<char32_t>
					operator""sv(const char32_t* __str, size_t __len)
				{
					return basic_string_view<char32_t>{__str, __len};
				}
#endif

			}
		}

		_GLIBCXX_END_NAMESPACE_VERSION
	} // namespace experimental
} // namespace std

#include <experimental/string_view.tcc>

#endif // __cplusplus <= 201103L

#endif // _GLIBCXX_EXPERIMENTAL_STRING_VIEW
