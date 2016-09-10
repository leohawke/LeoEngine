/*!	\file Convert.hpp
\ingroup CHRLib
\brief 转换模板。
\version r113
\author FrankHB <frankhb1989@gmail.com>
\since build 400
\par 创建时间:
2015-12-29 10:18:20 +0800
\par 修改时间:
2016-06-19 leohawke port to vs2015
\par 文本编码:
UTF-8
\par 模块名称:
CHRLib::Convert
*/

#ifndef LBase_CHRLib_Convert_hpp
#define LBase_CHRLib_Convert_hpp 1

#include "LBase/any_iterator.hpp"
#include "LBase/CHRLib/CharacterMapping.h"

namespace CHRLib
{

	template<typename _tIn, typename _tOut, typename _fConv>
	ConversionResult
		ConvertCharacter(_fConv f, _tOut&& obj, _tIn&& i, ConversionState&& st)
	{
		return f(lforward(obj),leo::input_monomorphic_iterator(std::ref(i)),
			std::move(st));
	}
	template<typename _tIn, typename _fConv>
	ConversionResult
		ConvertCharacter(_fConv f, _tIn&& i, ConversionState&& st)
	{
		return f(leo::input_monomorphic_iterator(std::ref(i)), std::move(st));
	}

	using leo::remove_reference_t;
	using leo::decay_t;

	/*!
	\brief 验证指定字符序列是否可转换为 UCS-2 字符串。
	\pre 指定范围内的迭代器可解引用。
	\note 使用 ADL \c MBCToUC 指定转换迭代器的例程：第一参数应迭代输入参数至下一位置。
	*/
	//@{
	template<typename _tIn>
	bool
		VerifyUC(_tIn&& first, remove_reference_t<_tIn> last, Encoding enc)
	{
		auto res(ConversionResult::OK);

		while (first != last && *first != 0
			&& (res = MBCToUC(first, enc), res == ConversionResult::OK))
			;
		return res == ConversionResult::OK || *first == char();
	}
	/*!
	\pre 迭代器为随机迭代器。
	\note 使用指定迭代器边界。
	*/
	template<typename _tIn>
	bool
		VerifyUC(_tIn&& first, typename std::iterator_traits<decay_t<_tIn>>
			::difference_type n, Encoding enc)
	{

		if (n != 0)
		{
			auto res(ConversionResult::OK);
			const auto i(first);

			while (*first != 0 && (res = MBCToUC(first, first + n, enc),
				res == ConversionResult::OK))
				;
			return res == ConversionResult::OK
				|| (res == ConversionResult::BadSource && first - i == n)
				|| *first == char();
		}
		return true;
	}
	//@}

} // namespace CHRLib;

#endif
