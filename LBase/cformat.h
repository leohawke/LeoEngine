#pragma once

#include "cassert.h" // for lconstraint
#include <cstdarg> // for std::va_list;
#include <stdexcept>

namespace leo
{
	/*!
	\brief 计算指定格式字符串输出的占用的字节数。
	\pre 断言：第一参数非空。
	\return 成功为字节数，否则为 size_t(-1) 。
	\note 分别使用 std::vsnprintf 和 std::vswprintf 实现。
	*/
	//@{
	LB_API LB_NONNULL(1) size_t
		vfmtlen(const char*, std::va_list) lnothrow;
	LB_API LB_NONNULL(1) size_t
		vfmtlen(const wchar_t*, std::va_list) lnothrow;
	//@}


	/*!
	\pre 间接断言：第一参数非空。
	\since build 1.4
	\bug \c char 以外的模板参数非正确实现。
	*/
	//@{
	/*!
	\brief 以 C 标准输出格式的输出 std::basic_string 实例的对象。
	\throw std::runtime_error 格式化字符串输出失败。
	\note 对 _tString 构造异常中立。
	*/
	template<typename _tChar, class _tString = std::basic_string<_tChar>>
	LB_NONNULL(1) _tString
		vsfmt(const _tChar* fmt, std::va_list args)
	{
		std::va_list ap;

		va_copy(ap, args);

		const auto l(leo::vfmtlen(fmt, ap));

		va_end(ap);
		if (l == size_t(-1))
			throw std::runtime_error("Failed to write formatted string.");

		_tString str(l, _tChar());

		if (l != 0)
		{
			lassume(str.length() > 0 && str[0] == _tChar());
			if constexpr (std::is_same_v<wchar_t, _tChar>)
				std::vswprintf(&str[0], l + 1, fmt, args);
			else
				std::vsprintf(&str[0], fmt, args);
		}
		return str;
	}

	/*!
	\brief 以 C 标准输出格式的输出 std::basic_string 实例的对象。
	\note 使用 ADL 访问可变参数。
	\note Clang++ 对模板声明 attribute 直接提示格式字符串类型错误。
	*/
	template<typename _tChar>
	LB_NONNULL(1) auto
		sfmt(const _tChar* fmt, ...)
		-> decltype(vsfmt(fmt, std::declval<std::va_list>()))
	{
		std::va_list args;

		va_start(args, fmt);
		try
		{
			auto str(vsfmt(fmt, args));

			va_end(args);
			return str;
		}
		catch (...)
		{
			va_end(args);
			throw;
		}
	}
	//@}

#if LB_IMPL_MSCPP
#else
	template LB_ATTR(format(printf, 1, 2)) std::string
		sfmt<char>(const char*, ...);
#endif
}