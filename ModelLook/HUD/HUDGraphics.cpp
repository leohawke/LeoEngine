#include "RenderSystem/TextureX.hpp"
#include "HUDImpl.h"

namespace leo {
	namespace HUD {
		ImplDeDtor(IImage)

		using Drawing::SPos;
		using Drawing::SDst;

		Point
		ClipMargin(PaintContext& pc, const Drawing::Padding& m, const Size& ss)
		{
			const Size& ds(pc.Target.GetSize());

			if (GetHorizontalOf(m) < ds.Width && GetVerticalOf(m) < ds.Height)
			{
				const auto& pt(pc.Location);
				const Point dp(std::max<int>(m.Left, pt.X), std::max<int>(m.Top, pt.Y));
				const Point sp(dp - pt);
				// XXX: Conversion to 'SPos' might be implementation-defined.
				const SPos scx(std::min<SPos>(SPos(ss.Width), SPos(ds.Width) - m.Right
					- dp.X) - sp.X), scy(std::min<SPos>(SPos(ss.Height), SPos(ds.Height)
						- m.Bottom - dp.Y) - sp.Y);

				if (scx > 0 && scy > 0)
				{
					pc.ClipArea &= Rect(dp, SDst(scx), SDst(scy));
					return pc.ClipArea.GetPoint() - pt;
				}
			}
			pc.ClipArea.GetSizeRef() = {};
			return{};
		}
	}

	namespace X {
		std::shared_ptr<HUD::IImage>  MakeIImage(HUD::Size s) {
			return std::make_shared<HUD::details::hud_tex_wrapper>(s);
		}
	}
}