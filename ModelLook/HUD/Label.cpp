#include "Label.h"
#include "HUDImpl.h"

LEO_BEGIN
HUD_BEGIN

using SPos = platform::unit_type;
using SDst = platform::unitlength_type;
using std::max;

#define Margin 1
#define GetHorizontalOf(Margin) Margin
#define GetVerticalOf(Margin) Margin

#define RectAddMargin(r,m) Rect(r.X + m, r.Y + m, \
	SDst(max<SPos>(0, SPos(r.Width) - m - m)), \
	SDst(max<SPos>(0, SPos(r.Height) - m - m)))

#define RectSubMargin(r,m) RectAddMargin(r,-(m))

#define FetchStringWidth(Font,Text) (Font.GetSize()+1)*Text.size()
#define Font_GetHeight(Font) Font.GetSize()+1



MLabel::MLabel(const Drawing::Font & fnt, TextAlignment a)
	:Font(fnt),HorizontalAlignment(a)
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
				- Font_GetHeight(Font)));

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
	const auto r(RectAddMargin(Rect(pc.Location, s),Margin));

	auto offset = GetAlignedPenOffset(s);

	details::DrawText(Text, { pc.Target,pc.Location,pc.ClipArea&r }, offset);
}

Rect Label::CalculateBounds(const std::string & text, Rect r, const Drawing::Font & fnt)
{
	if (r.GetSize() == Size::Invalid)
	{
		r.GetSizeRef() = { FetchStringWidth(fnt, text),Font_GetHeight(fnt)};
		r = RectSubMargin(r,Margin);
	}
	return r;
}

void Label::Refresh(PaintEventArgs && e)
{
	(*this)(std::move(e));
}

LB_API std::unique_ptr<Label> MakeLabel(const std::string &text, const Rect & r, const Drawing::Font & fnt)
{
	auto p(std::make_unique<Label>(Label::CalculateBounds(text, r, fnt), fnt));

	lunseq(p->Text = text);
	return p;
}

HUD_END
LEO_END
