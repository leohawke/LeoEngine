#ifndef IndePlatform_string_hpp
#define IndePlatform_string_hpp

#include "container.hpp"
#include <string>
#include <cstdio>
#include <cstdarg> //std::vsnprintf

namespace leo
{
	template<typename _tString>
	//字符串特征
	struct string_traits
	{
		using string_type = decay_t < _tString > ;
		using value_type = remove_rcv_t < decltype(std::declval<string_type>()[0]) > ;
		using traits_type = typename std::char_traits < value_type > ;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using initializer = std::initializer_list < value_type > ;
	};

	template<typename _tParam,
		typename = limpl(decltype(std::declval<_tParam>()[0]))>
	//选择字符串类类型的特定重载避免和其他非字符串类型冲突
		using enable_for_string_class_t
		= enable_if_t<is_class<decay_t<_tParam>>::value, int>;

	template<typename _tChar>
	size_t
		//字符串长度
		string_length(const _tChar* str)
	{
		return std::char_traits<_tChar>::length(str);
	}
	template<class _tString, typename = enable_if_t<is_class<_tString>::value, int>>
	size_t
		//字符串长度
		string_length(const _tString& str)
	{
		return str.size();
	}

	namespace details
	{
		template<typename _tFwd1, typename _tFwd2, typename _fPred>
		bool
			//支持 std::forward_iterator_tag 重载
			ends_with_iter_dispatch(_tFwd1 b, _tFwd1 e, _tFwd2 bt, _tFwd2 et,
			_fPred comp, std::bidirectional_iterator_tag)
		{
			auto i(e);
			auto it(et);

			while (i != b && it != bt)
				if (!comp(*--i, *--it))
					return false;
			return it == bt;
		}

	} // namespace details;

	template<typename _tRange1, typename _tRange2, typename _fPred>
	bool
		//判断第一个参数指定的串是否以第二个参数起始
		starts_with(const _tRange1& input, const _tRange2& test, _fPred comp)
	{
		using std::begin;
		using std::end;
		const auto e(end(input));
		const auto et(end(test));
		auto i(begin(input));
		auto it(begin(test));

		for (; i != e && it != et; lunseq(++i, ++it))
			if (!comp(*i, *it))
				return false;
		return it == et;
	}

	template<typename _tRange1, typename _tRange2>
	inline bool
		//判断第一个参数指定的串是否以第二个参数起始
		starts_with(const _tRange1& input, const _tRange2& test)
	{
		return leo::starts_with(input, test, is_equal());
	}

	template<typename _tRange1, typename _tRange2, typename _fPred>
	inline bool
		//判断第一个参数指定的串是否以第二个参数结束
		ends_with(const _tRange1& input, const _tRange2& test, _fPred comp)
	{
		using std::begin;
		using std::end;

		return details::ends_with_iter_dispatch(begin(input), end(input),
			begin(test), end(test), comp, typename std::iterator_traits<
			remove_reference_t<decltype(begin(input)) >> ::iterator_category());
	}
	template<typename _tRange1, typename _tRange2>
	inline bool
		//判断第一个参数指定的串是否以第二个参数结束
		ends_with(const _tRange1& input, const _tRange2& test)
	{
		return leo::ends_with(input, test, is_equal());
	}

	template<class _tString>
	_tString
		//取字母表：有序的字符串的不重复序列
		alph(_tString& str)
	{
		_tString res(str);

		leo::sort_unique(res);
		return res;
	}
		template<class _tString>
		void
		concat(_tString& str, size_t n)
		{
			lconstraint(n != 0);
			if (1 < n)
			{
				const auto len(str.length());

				//俄罗斯农民算法
				leo::concat(str, n / 2);
				str.append(&str[0], str.length());
				if (n % 2 != 0)
					str.append(&str[0], len);
			}
		}

		template<class _tString>
		inline _tString&&
			//删除字符串中指定的连续前缀字符
			ltrim(_tString&& str, typename string_traits<_tString>::const_pointer t
			= &to_array<typename string_traits<_tString>::value_type>("\n\r\t\v ")[0])
		{
			return static_cast<_tString&&>(str.erase(0, str.find_first_not_of(t)));
		}

		template<class _tString>
		inline _tString&&
			//删除字符串中指定的连续后缀字符
			rtrim(_tString&& str, typename string_traits<_tString>::const_pointer t
			= &to_array<typename string_traits<_tString>::value_type>("\n\r\t\v ")[0])
		{
			return static_cast<_tString&&>(str.erase(str.find_last_not_of(t) + 1));
		}

		template<class _tString>
		inline _tString&&
			//删除字符中的指定的连续前缀与后缀字符
			trim(_tString&& str, typename string_traits<_tString>::const_pointer t
			= &to_array<typename string_traits<_tString>::value_type>("\n\r\t\v ")[0])
		{
			return static_cast<_tString&&>(leo::ltrim(leo::rtrim(str, t)));
		}

		template<typename _tString>
		inline _tString
			get_mid(const _tString& str, typename _tString::size_type l = 1)
		{
			lassume(!(str.size() < l * 2));
			return str.substr(l, str.size() - l * 2);
		}
		template<typename _tString>
		inline _tString
			get_mid(const _tString& str, typename _tString::size_type l,
			typename _tString::size_type r)
		{
			lassume(!(str.size() < l + r));
			return str.substr(l, str.size() - l - r);
		}

		template<typename _fPred, typename _fInsert, typename _tIn>
		void
			//以指定字符分割字符序列
			split(_tIn b, _tIn e, _fPred is_delim, _fInsert insert)
		{
			while (b != e)
			{
				_tIn i(std::find_if_not(b, e, is_delim));

				b = std::find_if(i, e, is_delim);
				if (i != b)
					insert(i, b);
				else
					break;
			}
		}

		template<typename _fPred, typename _fInsert, typename _tRange>
		inline void
			//以指定字符分割字符序列
			split(_tRange&& c, _fPred is_delim, _fInsert insert)
		{
			split(begin(c), end(c), is_delim, insert);
		}

		template<typename _fPred, typename _fInsert, typename _tIn>
		_tIn
			split_l(_tIn b, _tIn e, _fPred is_delim, _fInsert insert)
		{
			_tIn i(b);

			while (b != e)
			{
				if (is_delim(*b) && i != b)
				{
					insert(i, b);
					i = b;
				}
				++b;
			}
			if (i != b)
				insert(i, b);
			return i;
		}

		template<typename _fPred, typename _fInsert, typename _tRange>
		inline void
			split_l(_tRange&& c, _fPred is_delim, _fInsert insert)
		{
			split_l(begin(c), end(c), is_delim, insert);
		}

		inline std::string
			to_string(unsigned char val)
		{
			return std::to_string(unsigned(val));
		}

		inline std::string
			to_string(const std::string& str)
		{
			return str;
		}

		inline std::string
			to_string(unsigned short val)
		{
			return std::to_string(unsigned(val));
		}

		template<typename _type>
		inline std::string
			to_string(_type val, enable_if_t<is_enum<_type>::value, int> = 0)
		{
			using std::to_string;
			using leo::to_string;

			return to_string(underlying_type_t<_type>(val));
		}

		template<typename _tChar>
		std::basic_string<_tChar>
			vsfmt(const _tChar* fmt, std::va_list args)
		{
			std::string str(size_t(std::vsnprintf({}, 0, fmt, args)), _tChar());

			std::vsprintf(&str[0], fmt, args);
			va_end(args);
			return str;
		}

		template<typename _tChar>
		std::basic_string<_tChar>
			sfmt(const _tChar* fmt, ...)
		{
			std::va_list args;

			va_start(args, fmt);

			std::string str(vsfmt(fmt, args));

			va_end(args);
			return str;
		}

		template LB_ATTR(format(printf, 1, 2)) std::string
			sfmt<char>(const char*, ...);
}

namespace leo
{
	std::wstring towstring(const char* c_str, std::size_t len);
	std::wstring towstring(const std::string& string);

	std::string tostring(const wchar_t* c_str, std::size_t len);
	std::string tostring(const std::wstring& string);
}
#endif