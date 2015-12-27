#ifndef UI_CharRenderer_h
#define UI_CharRenderer_h

#include "UI.h"
#include "TextBase.h"

namespace leo
{
	/*!
	\brief 检查默认区域下指定字符是否为可打印字符。
	\note MSVCRT 的 isprint/iswprint 实现缺陷的变通。
	\sa https://connect.microsoft.com/VisualStudio/feedback/details/799287/isprint-incorrectly-classifies-t-as-printable-in-c-locale
	*/
	//@{
	inline PDefH(bool, IsPrint, char c)
		ImplRet(stdex::isprint(c))
		inline PDefH(bool, IsPrint, wchar_t c)
		ImplRet(stdex::iswprint(c))
		template<typename _tChar>
	bool
		IsPrint(_tChar c)
	{
		return leo::IsPrint(wchar_t(c));
	}
}

LEO_DRAW_BEGIN

/*!
\brief 字符按像素块传输。
\tparam _tOut 输出迭代器类型。
\tparam _tIn 输入迭代器类型。
\tparam _fPixelShader 像素着色器类型。
\param blit 像素操作。
\param src 源迭代器。
\param ss 源迭代器所在缓冲区大小。
\param pc 指定字符所在区域和渲染目标的绘制上下文，
其中 Location 为相对源的坐标。
\param neg_pitch 指定交换行渲染顺序。
\sa Drawing::Blit
\sa Drawing::BlitPixels
\since build 440
*/
template<typename _tOut, typename _tIn, typename _fPixelShader>
void
BlitGlyphPixels(_fPixelShader blit, _tOut dst, _tIn src, const Size& ss,
	const PaintContext& pc, bool neg_pitch)
{
	const auto& ds(pc.Target.GetSize());
	const Rect& bounds(pc.ClipArea);

	if (neg_pitch)
		BlitPixels<false, true>(blit, dst, src, ds, ss, bounds.GetPoint(),
			pc.Location, bounds.GetSize());
	else
		BlitPixels<false, false>(blit, dst, src, ds, ss, bounds.GetPoint(),
			pc.Location, bounds.GetSize());
}

/*!
\brief 渲染单个字符。
\param pc 指定字符所在区域和渲染目标的绘制上下文，
其中 Location 为相对源的坐标。
\pre 断言：缓冲区非空。
\note 忽略 Alpha 缓冲。
*/
LB_API void
RenderChar(PaintContext&& pc, Color, bool, CharBitmap::BufferType,
	CharBitmap::FormatType, const Size&);

/*!
\brief 取文本渲染器的行末位置（横坐标）。
*/
template<class _tRenderer>
inline SPos
GetEndOfLinePositionOf(const _tRenderer& r)
{
	return r.GetTextState().Margin.Right;
}

/*!
\brief 取文本渲染器的行末位置剩余偏移。
\note 使用 ADL <tt>GetEndOfLinePositionOf</tt> 取行末位置。
*/
template<class _tRenderer>
inline SPos
GetEndOfLineOffsetOf(const _tRenderer& r)
{
	return SPos(r.GetContext().GetWidth()) - GetEndOfLinePositionOf(r);
}


/*!
\brief 打印单个可打印字符。
\todo 行的结尾位置计算和边距解除耦合。
*/
template<class _tRenderer>
void
PrintChar(_tRenderer& r, ucs4_t c)
{
	if (LB_LIKELY(IsPrint(c)))
		r(c);
}

//@{
//! \brief 输出字符结果。
enum class PutCharResult
{
	//! \brief 行内无法容纳而换行。
	NeedNewline,
	//! \brief 输出换行符。
	PutNewline,
	//! \brief 遇到不可打印字符。
	NotPrintable,
	//! \brief 可继续在同一行输出可打印字符。
	Normal
};

//! \brief 使用指定的文本状态和行末位置按需打印换行并判断是否需要渲染单个字符。
LB_API PutCharResult
PutCharBase(TextState&, SDst, ucs4_t);

/*!
\brief 打印单个字符。
\return 遇到行内无法容纳而换行时返回非零值，否则返回 0 。
\note 处理换行符。
\note 当行内无法容纳完整字符时换行。
*/
//@{
template<class _tRenderer>
PutCharResult
PutChar(_tRenderer& r, ucs4_t c, SDst eol)
{
	const auto res(PutCharBase(r.GetTextState(), eol, c));

	if (res == PutCharResult::Normal)
		r(c);
	return res;
}
template<class _tRenderer>
PutCharResult
PutChar(_tRenderer& r, ucs4_t c)
{
	const SPos seol(GetEndOfLineOffsetOf(r));

	if (seol >= 0)
		return PutChar(r, c, SDst(seol));
	return PutCharResult::NeedNewline;
}

template<class _tRenderer, typename _tChar, typename... _tParams>
inline PutCharResult
PutChar(_tRenderer& r, _tChar c, _tParams&&... args)
{
	return PutChar(r, ucs4_t(c), lforward(args)...);
}
//@}


LEO_DRAW_END

#endif
