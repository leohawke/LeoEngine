#include "TextRenderer.h"
#include "Blit.h"

LEO_DRAW_BEGIN

namespace
{

	PaintContext
		ClipChar(const Graphics& g, const Point& pen, const CharBitmap& cbmp, Rect r)
	{
		LAssert(bool(g), "Invalid graphics context found.");

		const auto pt(ClipBounds(r, Rect(pen.X + cbmp.GetLeft(),
			pen.Y - cbmp.GetTop(), cbmp.GetWidth(), cbmp.GetHeight())));

		return{ g, pt, r };
	}

	SDst
		FetchBMPSrcWidth(const CharBitmap& cbmp)
	{
		const SDst abs_pitch(std::abs(cbmp.GetPitch()));

		switch (cbmp.GetFormat())
		{
		case CharBitmap::Mono:
			return abs_pitch * 8;
		case CharBitmap::Gray2:
			return abs_pitch * 4;
		case CharBitmap::Gray4:
			return abs_pitch * 2;
		default:
			break;
		}
		return abs_pitch;
	}

	//! \since build 368
	template<typename _tCharRenderer, _tCharRenderer& _fCharRenderer,
		typename... _tParams>
		void
		RenderCharFrom(ucs4_t c, const Graphics& g, TextState& ts, const Rect& clip,
			_tParams&&... args)
	{
		const auto cbmp(ts.Font.GetGlyph(c));

		if (LB_LIKELY(cbmp))
		{
			// TODO: Show a special glyph when no bitmap found.
			// TODO: Use fast glyph advance fetching for non-graph characters
			//	when possible.
			// TODO: Handle '\t'.
			if (stdex::iswgraph(wchar_t(c)))
				if (const auto cbuf = cbmp.GetBuffer())
				{
					auto&& pc(ClipChar(g, ts.Pen, cbmp, clip));

					// TODO: Test support for bitmaps with negative pitch.
					if (!pc.ClipArea.IsUnStrictlyEmpty())
						_fCharRenderer(std::move(pc), ts.Color, cbmp.GetPitch() < 0,
							cbuf, cbmp.GetFormat(), { FetchBMPSrcWidth(cbmp),
							cbmp.GetHeight() }, yforward(args)...);
				}
			ts.Pen.X += cbmp.GetXAdvance();
		}
	}

} // unnamed namespace;

void
TextRenderer::operator()(ucs4_t c)
{
	RenderCharFrom<decltype(RenderChar), RenderChar>(c, GetContext(), State,
		ClipArea);
}

void
TextRenderer::ClearLine(size_t l, SDst n)
{
	const auto& g(GetContext());
	const auto h(g.GetHeight());

	if (LB_LIKELY(bool(g) && l < h))
	{
		if (n == 0 || l + n > h)
			n = h - SDst(l);
		ClearPixel(g[l], size_t(g.GetWidth() * n));
	}
}

void DrawClippedText(const Graphics & g, const Rect & bounds, TextState & ts, const String & str, bool line_wrap)
{
	TextRenderer tr(ts, g, bounds);

	PutText(line_wrap, tr, str);
}

LEO_DRAW_END
