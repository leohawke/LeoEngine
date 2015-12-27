#ifndef UI_TextRenderer_h
#define UI_TextRenderer_h

#include "TextBase.h"
#include "TextLayout.h"
#include "CharRenderer.h"

LEO_DRAW_BEGIN

using String = std::string;

/*!
\note 使用 ADL <tt>GetEndOfLineOffsetOf</tt> 判断行尾位置。
\note 使用 ADL <tt>PrintChar</tt> 或 <tt>PutChar</tt> 渲染字符。
*/
//@{
/*!
\brief 打印迭代器指定的起始字符的字符串，直至行尾或字符迭代终止。
\param rd 使用的字符渲染器。
\param s 指向字符串起始字符的输入迭代器。
\return 指向结束位置的迭代器。
\note 迭代直至字符串结束符。
*/
template<typename _tIter, class _tRenderer,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	_tIter
	PrintLine(_tRenderer& rd, _tIter s)
{
	while (*s != 0 && *s != '\n')
	{
		PrintChar(rd, *s);
		++s;
	}
	return s;
}
/*!
\brief 打印迭代器指定的起始字符的字符串，直至行尾或字符迭代终止。
\param rd 使用的字符渲染器。
\param s 指向字符串起始字符的输入迭代器。
\param g 指示迭代终止位置的输入迭代器。
\param c 指向迭代终止的字符。
\return 指向结束位置的迭代器。
\note 迭代直至 g 指定的位置或指定位置的字符为 c 。
*/
template<typename _tIter, class _tRenderer,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	_tIter
	PrintLine(_tRenderer& rd, _tIter s, _tIter g, ucs4_t c = {})
{
	while (s != g && ucs4_t(*s) != c && *s != '\n')
	{
		PrintChar(rd, *s);
		++s;
	}
	return s;
}
/*!
\brief 打印字符串，直至行尾或字符串结束。
\param rd 使用的字符渲染器。
\param str 被输出的字符串。
\return 打印字符数。
*/
template<class _tRenderer, class _tString,
	limpl(typename = enable_for_string_class_t<_tString>)>
	inline String::size_type
	PrintLine(_tRenderer& rd, const _tString& str)
{
	return String::size_type(Drawing::PrintLine(rd, &str[0]) - &str[0]);
}

/*!
\brief 打印迭代器指定的起始字符的字符串，直至行尾或字符迭代终止。
\param rd 使用的字符渲染器。
\param s 指向字符串起始字符的输入迭代器。
\return 指向结束位置的迭代器。
\note 迭代直至字符串结束符。
\note 当行内无法容纳完整字符时换行。
*/
template<typename _tIter, class _tRenderer,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	_tIter
	PutLine(_tRenderer& rd, _tIter s)
{
	const SPos seol(GetEndOfLineOffsetOf(rd));

	if (seol >= 0)
	{
		TextState& ts(rd.GetTextState());
		const SPos fpy(ts.Pen.Y);

		while (*s != 0 && fpy == ts.Pen.Y)
			if (PutChar(rd, *s, SDst(seol)) != PutCharResult::NeedNewline)
				++s;
	}
	return s;
}
/*!
\brief 打印迭代器指定的起始字符的字符串，直至行尾或字符迭代终止。
\param rd 使用的字符渲染器。
\param s 指向字符串起始字符的输入迭代器。
\param g 指示迭代终止位置的输入迭代器。
\param c 指向迭代终止的字符。
\return 指向结束位置的迭代器。
\note 迭代直至 g 指定的位置或指定位置的字符为 c 。
\note 当行内无法容纳完整字符时换行。
*/
template<typename _tIter, class _tRenderer,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	_tIter
	PutLine(_tRenderer& rd, _tIter s, _tIter g, ucs4_t c = {})
{
	const SPos seol(GetEndOfLineOffsetOf(rd));

	if (seol >= 0)
	{
		TextState& ts(rd.GetTextState());
		const SPos fpy(ts.Pen.Y);

		while (s != g && ucs4_t(*s) != c && fpy == ts.Pen.Y)
			if (PutChar(rd, *s, SDst(seol)) != PutCharResult::NeedNewline)
				++s;
	}
	return s;
}
/*!
\brief 打印字符串，直至行尾或字符串结束。
\param rd 使用的字符渲染器。
\param str 被输出的字符串。
\return 打印字符数。
\note 当行内无法容纳完整字符时换行。
*/
template<class _tRenderer, class _tString,
	limpl(typename = enable_for_string_class_t<_tString>)>
	inline String::size_type
	PutLine(_tRenderer& rd, const _tString& str)
{
	return String::size_type(Drawing::PutLine(rd, &str[0]) - &str[0]);
}

/*!
\brief 打印迭代器指定的起始字符的字符串，直至区域末尾或字符迭代终止。
\param rd 使用的字符渲染器。
\param s 指向字符串起始字符的输入迭代器。
\return 指向结束位置的迭代器。
\note 迭代直至字符串结束符。
*/
template<typename _tIter, class _tRenderer,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	_tIter
	PrintString(_tRenderer& rd, _tIter s)
{
	while (*s != 0 && *s != '\n')
		PrintChar(rd, *s++);
	return s;
}
/*!
\brief 打印迭代器指定的起始字符的字符串，直至区域末尾或字符迭代终止。
\param rd 使用的字符渲染器。
\param s 指向字符串起始字符的输入迭代器。
\param g 指示迭代终止位置的输入迭代器。
\param c 指向迭代终止的字符。
\return 指向结束位置的迭代器。
\note 迭代直至 g 指定的位置或指定位置的字符为 c 。
*/
template<typename _tIter, class _tRenderer,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	_tIter
	PrintString(_tRenderer& rd, _tIter s, _tIter g, ucs4_t c = {})
{
	while (s != g && ucs4_t(*s) != c && *s != '\n')
		PrintChar(rd, *s++);
	return s;
}
/*!
\brief 打印字符串，直至区域末尾或字符串结束。
\param rd 使用的字符渲染器。
\param str 被输出的字符串。
\return 打印字符数。
*/
template<class _tRenderer, class _tString,
	limpl(typename = enable_for_string_class_t<_tString>)>
	inline String::size_type
	PrintString(_tRenderer& rd, const _tString& str)
{
	return String::size_type(Drawing::PrintString(rd, &str[0]) - &str[0]);
}

/*!
\brief 打印迭代器指定的起始字符的字符串，直至区域末尾或字符迭代终止。
\param rd 使用的字符渲染器。
\param s 指向字符串起始字符的输入迭代器。
\return 指向结束位置的迭代器。
\note 迭代直至字符串结束符。
\note 当行内无法容纳完整字符时换行。
*/
template<typename _tIter, class _tRenderer,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	_tIter
	PutString(_tRenderer& rd, _tIter s)
{
	const SPos seol(GetEndOfLineOffsetOf(rd));

	if (seol >= 0)
	{
		TextState& ts(rd.GetTextState());
		const SPos mpy(FetchLastLineBasePosition(ts, rd.GetHeight()));

		while (*s != 0 && ts.Pen.Y <= mpy)
			if (PutChar(rd, *s, SDst(seol)) != PutCharResult::NeedNewline)
				++s;
	}
	return s;
}
/*!
\brief 打印迭代器指定的起始字符的字符串，直至区域末尾或字符迭代终止。
\param rd 使用的字符渲染器。
\param s 指向字符串起始字符的输入迭代器。
\param g 指示迭代终止位置的输入迭代器。
\param c 指向迭代终止的字符。
\return 指向结束位置的迭代器。
\note 迭代直至 g 指定的位置或指定位置的字符为 c 。
\note 当行内无法容纳完整字符时换行。
*/
template<typename _tIter, class _tRenderer,
	limpl(typename = enable_for_iterator_t<_tIter>)>
	_tIter
	PutString(_tRenderer& rd, _tIter s, _tIter g, ucs4_t c = {})
{
	const SPos seol(GetEndOfLineOffsetOf(rd));

	if (seol >= 0)
	{
		TextState& ts(rd.GetTextState());
		const SPos mpy(FetchLastLineBasePosition(ts, rd.GetHeight()));

		while (s != g && ucs4_t(*s) != c && ts.Pen.Y <= mpy)
			if (PutChar(rd, *s, SDst(seol)) != PutCharResult::NeedNewline)
				++s;
	}
	return s;
}
/*!
\brief 打印字符串，直至区域末尾或字符串结束。
\param rd 使用的字符渲染器。
\param str 被输出的字符串。
\return 打印字符数。
\note 当行内无法容纳完整字符时换行。
*/
template<class _tRenderer, class _tString,
	limpl(typename = enable_for_string_class_t<_tString>)>
	inline String::size_type
	PutString(_tRenderer& rd, const _tString& str)
{
	return String::size_type(Drawing::PutString(rd, &str[0]) - &str[0]);
}
//@}

/*!
\brief 打印文本。
\param multi 指定是否可输出多行。
\param args 传递给 PutString 或 PutLine 的参数。
*/
template<typename... _tParams>
void
PutText(bool multi, _tParams&&... args)
{
	if (multi)
		Drawing::PutString(lforward(args)...);
	else
		Drawing::PutLine(lforward(args)...);
}


/*!	\defgroup TextRenderers Text Renderers
\brief 文本渲染器。
*/

/*!
\ingroup TextRenderers
\brief 空文本渲染器。
*/
class LB_API EmptyTextRenderer
{
public:
	TextState& State;
	SDst Height;

	EmptyTextRenderer(TextState& ts, SDst h)
		: State(ts), Height(h)
	{}

	/*!
	\brief 渲染单个字符：仅移动笔，不绘制。
	*/
	PDefHOp(void, (), ucs4_t c)
		ImplExpr(MovePen(State, c))

		DefGetter(const lnothrow, const TextState&, TextState, State)
		DefGetter(lnothrow, TextState&, TextState, State)
		DefGetter(const lnothrow, SDst, Height, Height)
};


/*!
\brief 文本渲染器静态多态基类模板。
\warning 非虚析构。
*/
template<class _type>
class GTextRendererBase
{
public:
	DeclSEntry(const TextState& GetTextState() const) //!< 取文本状态。
		DeclSEntry(TextState& GetTextState()) //!< 取文本状态。
											  /*
											  \brief 取图形接口上下文。
											  */
		DeclSEntry(Graphics GetContext() const)

#define This static_cast<_type*>(this)
#define CThis static_cast<const _type*>(this)

		/*!
		\brief 取按当前行高和行距所能显示的最大行数。
		*/
		DefGetter(const, std::uint16_t, TextLineN, FetchResizedLineN(
			CThis->GetTextState(), CThis->GetContext().GetHeight()))
		/*!
		\brief 取按当前行高和行距（行间距数小于行数 1 ）所能显示的最大行数。
		*/
		DefGetter(const, std::uint16_t, TextLineNEx,
			FetchResizedLineN(CThis->GetTextState(), CThis->GetContext().GetHeight()
				+ CThis->GetTextState().LineGap))

#undef CThis
#undef This

};


/*!
\ingroup TextRenderers
\brief 文本渲染器：简单实现。
\warning 非虚析构。
*/
class LB_API TextRenderer : public GTextRendererBase<TextRenderer>
{
public:
	TextState& State;
	const Graphics& Buffer;
	Rect ClipArea;

	TextRenderer(TextState& ts, const Graphics& g)
		: GTextRendererBase<TextRenderer>(),
		State(ts), Buffer(g), ClipArea(g.GetSize())
	{}
	/*
	\brief 构造：使用文本状态、图形接口上下文和指定区域边界。
	*/
	TextRenderer(TextState& ts, const Graphics& g, const Rect& bounds)
		: GTextRendererBase<TextRenderer>(),
		State(ts), Buffer(g), ClipArea(bounds)
	{}

	/*!
	\brief 渲染单个字符。
	*/
	void
		operator()(ucs4_t);

	ImplS(GTextRendererBase) DefGetter(const lnothrow, const TextState&,
		TextState, State)
		ImplS(GTextRendererBase) DefGetter(lnothrow, TextState&, TextState, State)
		ImplS(GTextRendererBase) DefGetter(const lnothrow, Graphics, Context,
			Buffer)
		//@{
		DefGetterMem(const lnothrow, SDst, Height, Buffer)
		DefGetterMem(const lnothrow, SDst, Width, Buffer)
		DefGetterMem(const lnothrow, const Size&, Size, Buffer)
		//@}

		/*!
		\brief 清除缓冲区第 l 行起始的 n 行像素。
		\note 图形接口上下文不可用或 l 越界时忽略。
		\note n 被限制为不越界。
		\note n 为 0 时清除之后的所有行。
		*/
		void
		ClearLine(size_t l, SDst n);
};

void
DrawClippedText(const Graphics& g, const Rect& bounds, TextState& ts,
	const String& str, bool line_wrap);

LEO_DRAW_END

#endif