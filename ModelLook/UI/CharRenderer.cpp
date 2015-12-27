#include "CharRenderer.h"
#include "Blend.h"

LEO_DRAW_BEGIN

namespace
{

	const AlphaType BLT_TEXT_ALPHA_THRESHOLD(16);

	/*!
	\brief 混合 Alpha 透明度扫描线。
	\warning 不检查迭代器有效性。
	*/
	struct BlitTextPoint
	{
		Pixel<> Color;

		template<typename _tOut, typename _tIn>
		void
			operator()(_tOut dst_iter, _tIn src_iter) const
		{
			if (*src_iter >= BLT_TEXT_ALPHA_THRESHOLD)
				lunseq(*get<1>(dst_iter.base()) = *src_iter,
					*dst_iter = Color);
		}
	};

	template<size_t _vN>
	struct tr_seg
	{
		static_assert(_vN < CHAR_BIT, "Specified bits should be within a byte.");

		byte v;

		const byte&
			operator()(const bitseg_iterator<_vN, true>& i) lnothrow
		{
			return v = byte(unsigned(*i << (size_t(CHAR_BIT) - _vN))
				| ((1U << _vN) - 1));
		}
	};


	//@{
	using PixelIt = pseudo_iterator<const Pixel<>>;

	//! \since build 584
	template<size_t _vBit>
	using MonoItPairN = pair_iterator<PixelIt,
		transformed_iterator<bitseg_iterator<_vBit, true>, tr_seg<_vBit>>>;

	template<unsigned char _vN>
	auto
		tr_buf(byte* p)
		-> decltype(make_transform(bitseg_iterator<_vN, true>(p), tr_seg<_vN>()))
	{
		return make_transform(bitseg_iterator<_vN, true>(p), tr_seg<_vN>());
	}
	//@}

} // unnamed namespace;

void
RenderChar(PaintContext&& pc, Color c, bool neg_pitch,
	CharBitmap::BufferType cbuf, CharBitmap::FormatType fmt, const Size& ss)
{
	LAssert(cbuf, "Invalid buffer found.");

	const Shaders::BlitAlphaPoint bp{};
	const auto dst(pc.Target.GetBufferPtr());

	switch (fmt)
	{
	case CharBitmap::Mono:
		BlitGlyphPixels(bp, dst, MonoItPairN<1>(PixelIt(c), tr_buf<1>(cbuf)),
			ss, pc, neg_pitch);
		break;
	case CharBitmap::Gray2:
		BlitGlyphPixels(bp, dst, MonoItPairN<2>(PixelIt(c), tr_buf<2>(cbuf)),
			ss, pc, neg_pitch);
		break;
	case CharBitmap::Gray4:
		BlitGlyphPixels(bp, dst, MonoItPairN<4>(PixelIt(c), tr_buf<4>(cbuf)),
			ss, pc, neg_pitch);
		break;
	case CharBitmap::Gray:
		BlitGlyphPixels(bp, dst, pair_iterator<PixelIt, const AlphaType*>(
			PixelIt(c), cbuf), ss, pc, neg_pitch);
	default:
		break;
	}
}


PutCharResult PutCharBase(TextState & ts, SDst eol, ucs4_t c)
{
	if (c == '\n')
	{
		ts.PutNewline();
		return PutCharResult::PutNewline;
	}
	if (LB_UNLIKELY(!IsPrint(c)))
		return PutCharResult::NotPrintable;

	const SPos w_adv(ts.Pen.X + ts.Font.GetAdvance(c));

	if (LB_UNLIKELY(w_adv > 0 && SDst(w_adv) > eol))
	{
		ts.PutNewline();
		return PutCharResult::NeedNewline;
	}
	return PutCharResult::Normal;
}

LEO_DRAW_END
