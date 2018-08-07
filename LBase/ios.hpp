/*!	\file ios.hpp
\ingroup LBase
\brief ISO C++ 标准库输入/输出流基类扩展。
*/

#ifndef LBase_ios_hpp
#define LBase_ios_hpp 1


#include "LBase/ldef.h"
#include <ios> // for std::basic_ios, std::ios_base::iostate;

namespace leo
{
	/*!
	\brief 设置流状态。
	\note 无异常抛出：和直接调用 std::basic_ios 的 setstate 成员函数不同。
	\note 不复用标准库实现的内部接口。
	*/
	template<typename _tChar, class _tTraits>
	void
		setstate(std::basic_ios<_tChar, _tTraits>& ios, std::ios_base::iostate state) lnothrow
	{
		const auto except(ios.exceptions());

		ios.exceptions(std::ios_base::goodbit);
		ios.setstate(state);
		ios.exceptions(except);
	}

	/*!
	\brief 设置流状态并重新抛出当前异常。
	\note 一个主要用例为实现标准库要求的格式/非格式输入/输出函数。
	\see WG21/N4567 27.7.2.2.1[istream.formatted.reqmts] 。
	\see WG21/N4567 27.7.2.3[istream.unformatted]/1 。
	\see WG21/N4567 27.7.3.6.1[ostream.formatted.reqmts] 。
	\see WG21/N4567 27.7.3.7[ostream.unformatted]/1 。
	\see http://wg21.cmeerw.net/lwg/issue91 。
	*/
	template<typename _tChar, class _tTraits>
	LB_NORETURN
	void
		rethrow_badstate(std::basic_ios<_tChar, _tTraits>& ios,
			std::ios_base::iostate state = std::ios_base::badbit)
	{
		setstate(ios, state);
		throw;
	}

} // namespace leo;

#endif
