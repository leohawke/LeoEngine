#ifndef UI_TextLayout_h
#define UI_TextLayout_h

#include "Font.hpp"
#include "TextBase.h"
#include "GUI.h"

LEO_DRAW_BEGIN

/*!
\brief 取指定文本状态和文本区域高调整的底边距。
\pre 断言： <tt>GetTextLineHeightExOf(ts) != 0</tt> 。
\post <tt>ts.Margin.Bottom</tt> 不小于原值。
\return 返回调整后的底边距值（由字体大小、行距和高决定）。
*/
LB_API SDst
FetchResizedBottomMargin(const TextState&, SDst);

/*!
\brief 取指定文本状态和文本区域高所能显示的最大文本行数。
\pre 断言： <tt>GetTextLineHeightExOf(ts) != 0</tt> 。
\return 最大能容纳的文本行数。
*/
LB_API std::uint16_t
FetchResizedLineN(const TextState& ts, SDst);

/*!
\brief 取指定文本状态在指定高的区域中表示的最底行的基线位置（纵坐标）。
\note 若不足一行则最底行视为首行。
\warning 不检查边距正确性。若顶边距正确，则返回值应小于输入的高。
*/
LB_API SPos
FetchLastLineBasePosition(const TextState&, SDst);


/*!
\brief 取指定的字符使用指定字体的显示宽度。
\note 无边界限制。
*/
LB_API SDst
FetchCharWidth(const Font&, ucs4_t);

/*!
\brief 取迭代器指定的单行字符串在字体指定、无边界限制时的显示宽度。
\note 迭代器 s 指向字符串首字符，迭代直至字符串结束符。
*/
template<typename _tIter,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	SDst
	FetchStringWidth(const Font& fnt, _tIter s)
{
	SDst w(0);

	for (; *s != char(); ++s)
	{

		LAssert(!is_undereferenceable(s), "Invalid iterator found.");
		w += FetchCharWidth(fnt, *s);
	}
	return w;
}
/*!
\brief 取迭代器指定的单行字符串在字体指定、无边界限制时的显示宽度。
\note 迭代器 s 指向字符串首字符，迭代直至 n 次或指定字符 c 。
*/
template<typename _tIter,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	SDst
	FetchStringWidth(const Font& fnt, _tIter s, size_t n, ucs4_t c = {})
{
	SDst w(0);

	for (; n-- != 0 && *s != c; ++s)
	{

		LAssert(!is_undereferenceable(s), "Invalid iterator found.");
		w += FetchCharWidth(fnt, *s);
	}
	return w;
}
/*!
\brief 取迭代器指定的单行字符串在字体指定、无边界限制时的显示宽度。
\note 迭代器 s 指向字符串首字符，迭代直至边界迭代器 g 或指定字符 c 。
*/
template<typename _tIter,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	SDst
	FetchStringWidth(const Font& fnt, _tIter s, _tIter g, ucs4_t c = {})
{
	SDst w(0);

	for (; s != g && *s != c; ++s)
	{

		LAssert(!is_undereferenceable(s), "Invalid iterator found.");
		w += FetchCharWidth(fnt, *s);
	}
	return w;
}
/*!
\brief 取单行字符串在字体指定、无边界限制时的显示宽度。
*/
template<class _tString,
	limpl(typename = enable_for_string_class_t<_tString>)>
	inline SDst
	FetchStringWidth(const Font& fnt, const _tString& str)
{
	return Drawing::FetchStringWidth(fnt, str.c_str());
}
/*!
\brief 取单行字符串前不超过 n 个字符在字体指定、无边界限制时的显示宽度。
*/
template<class _tString,
	limpl(typename = enable_for_string_class_t<_tString>)>
	inline SDst
	FetchStringWidth(const Font& fnt, const _tString& str, size_t n)
{
	return Drawing::FetchStringWidth(fnt, str.data(), n);
}
/*!
\brief 取迭代器指定的单行字符串在指定文本状态和高度限制时的显示宽度。
\note 迭代器 s 指向字符串首字符，迭代直至字符串结束符。
\note 字体由文本状态指定。
*/
template<typename _tIter,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	SDst
	FetchStringWidth(TextState& ts, SDst h, _tIter s)
{
	const SPos x(ts.Pen.X);
	EmptyTextRenderer r(ts, h);

	PrintString(r, s);
	return SDst(ts.Pen.X - x);
}
/*!
\brief 取迭代器指定的单行字符串在指定文本状态和高度限制时的显示宽度。
\note 迭代器 s 指向字符串首字符，迭代直至边界迭代器 g 或指定字符 c 。
\note 字体由文本状态指定。
*/
template<typename _tIter,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	SDst
	FetchStringWidth(TextState& ts, SDst h, _tIter s, _tIter g, ucs4_t c = {})
{
	const SPos x(ts.Pen.X);
	EmptyTextRenderer r(ts, h);

	PrintString(r, s, g, c);
	return SDst(ts.Pen.X - x);
}
/*!
\brief 取单行字符串在指定文本状态和高度限制时的显示宽度。
\note 字体由文本状态指定。
\since build 483
*/
template<class _tString,
	limpl(typename = enable_for_string_class_t<_tString>)>
	inline SDst
	FetchStringWidth(TextState& ts, SDst h, const _tString& str)
{
	return FetchStringWidth(ts, h, str.c_str());
}


LEO_DRAW_END

#endif