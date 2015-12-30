#include "Label.h"
#include "HUDImpl.h"
#include "../UI/TextLayout.h"
#include "../UI/TextRenderer.h"

LEO_BEGIN
HUD_BEGIN

using Drawing::SDst;
using Drawing::SPos;

MLabel::MLabel(const Drawing::Font & fnt,Drawing::Color c,TextAlignment a)
	:ForeColor(c), Font(fnt),HorizontalAlignment(a)
{
}

void MLabel::operator()(PaintEventArgs && e) const
{
	DrawText(GetSizeOf(e.GetSender()), e);
}

Point MLabel::GetAlignedPenOffset(const Size & s) const
{
	Point pt;

	//if()
	{
		switch (HorizontalAlignment)
		{
		case leo::HUD::TextAlignment::Center:
		case leo::HUD::TextAlignment::Right:
			{
				SPos horizontal_offset(SPos(s.Width - GetHorizontalOf(Margin)
					- FetchStringWidth(Font, Text)));
				if (horizontal_offset > 0)
				{
					if (HorizontalAlignment == TextAlignment::Center)
						horizontal_offset /= 2;
					pt.X += horizontal_offset;
				}

			}
		case leo::HUD::TextAlignment::Left:
		default:
			break;
		}
		switch (VerticalAlignment)
		{
		case TextAlignment::Center:
		case TextAlignment::Down:
		{
			SPos vertical_offset(SPos(s.Height - GetVerticalOf(Margin)
				- Font.GetHeight()));

			if (vertical_offset > 0)
			{
				if (VerticalAlignment == TextAlignment::Center)
					vertical_offset /= 2;
				pt.Y += vertical_offset;
			}
		}
		case TextAlignment::Up:
		default:
			break;
		}
	}

	return pt;
}

void MLabel::DrawText(const Size & s, const PaintContext & pc) const
{
	const auto r((Rect(pc.Location, s) + Margin));
	Drawing::TextState ts(Font, Drawing::FetchMargin(r, pc.Target.GetSize()));
	ts.Color = ForeColor;
	ts.Pen += GetAlignedPenOffset(s);

	UpdateClippedText({ pc.Target, pc.Location, pc.ClipArea & r }, ts, Text,
		AutoWrapLine);
}

void
MLabel::DefaultUpdateClippedText(const PaintContext& pc,Drawing::TextState& ts,
	const String& text, bool auto_wrap_line)
{
	DrawClippedText(pc.Target, pc.ClipArea, ts, text, auto_wrap_line);
}

Rect Label::CalculateBounds(const String & text, Rect r, const Drawing::Font & fnt,
	const Drawing::Padding& m)
{
	if (r.GetSize() == Size::Invalid)
	{
		r.GetSizeRef() = { FetchStringWidth(fnt, text),fnt.GetHeight()};
		r = r -m;
	}
	return r;
}

void Label::Refresh(PaintEventArgs && e)
{
	(*this)(std::move(e));
}

LB_API std::unique_ptr<Label> MakeLabel(const String &text, const Rect & r, const Drawing::Font & fnt,const Drawing::Padding& m)
{
	auto p(std::make_unique<Label>(Label::CalculateBounds(text, r, fnt,m), fnt));

	lunseq(p->Margin = m, p->Text = text);
	return p;
}

HUD_END
LEO_END
