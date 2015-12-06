#include <LAssert.h>
#include "Widget.h"
#include "HUDRenderer.h"
#include "HUDControl.h"
#include "HUDUtilities.h"

LEO_BEGIN

HUD_BEGIN

Rect HUDRenderer::Paint(IWidget & wgt, PaintEventArgs && e)
{
	LAssert(&e.GetSender().GetRenderer() == this, "Invalid widget found.");
	if (LB_LIKELY(!e.ClipArea.IsUnStrictlyEmpty()))
		CallEvent<VisualEvent::Paint>(wgt, e);
	return e.ClipArea;
}

ImplDeDtor(HUDPseudoRenderer)

BufferedRenderer::BufferedRenderer(bool b, std::shared_ptr<IImage> p)
	: HUDRenderer(),
	rInvalidated(), pImageBuffer(p), IgnoreBackground(b)
{}

BufferedRenderer::BufferedRenderer(const BufferedRenderer& rd)
	: HUDRenderer(rd),
	rInvalidated(rd.rInvalidated), pImageBuffer(ClonePolymorphic(
		rd.pImageBuffer)), IgnoreBackground(rd.IgnoreBackground)
{}

bool
BufferedRenderer::RequiresRefresh() const
{
	return bool(rInvalidated);
}

void
BufferedRenderer::SetImageBuffer(std::shared_ptr<IImage> p,bool clone)
{
	pImageBuffer = clone ? std::shared_ptr<IImage>(ClonePolymorphic(p)) : p;
}

void
BufferedRenderer::SetSize(const Size& s)
{
	GetImageBuffer().SetSize(s);
	rInvalidated = { {}, s };
}

Rect
BufferedRenderer::CommitInvalidation(const Rect& r)
{
	return rInvalidated |= r;
}

Rect
BufferedRenderer::Paint(IWidget& wgt, PaintEventArgs&& e)
{
	const Rect& r(Validate(wgt, e.GetSender(), e));

	UpdateTo(e);
	return r;
}

void
BufferedRenderer::UpdateTo(const PaintContext& pc) const
{
	const auto& g(pc.Target);
	const Rect& bounds(pc.ClipArea);

	//CopyTo(g.GetBufferPtr(), GetContext(), g.GetSize(), bounds.GetPoint(),
		//bounds.GetPoint() - pc.Location, bounds.GetSize());
}

Rect
BufferedRenderer::Validate(IWidget& wgt, IWidget& sender,
	const PaintContext& pc)
{
	LAssert(&sender.GetRenderer() == this, "Invalid widget found.");
	if (RequiresRefresh())
	{
		//if (!IgnoreBackground && FetchContainerPtr(sender))
			//Invalidate(sender);

		const Rect& clip(pc.ClipArea & (rInvalidated + pc.Location));

		if (!clip.IsUnStrictlyEmpty())
		{
			const auto& g(GetContext());

			//if (!IgnoreBackground && FetchContainerPtr(sender))
			//{
			//	const auto dst(g.GetBufferPtr());
			//	const auto& src(pc.Target);

			//	if (dst != src.GetBufferPtr())
					//CopyTo(g.GetBufferPtr(), src, g.GetSize(), clip.GetPoint()
						//- pc.Location, clip.GetPoint(), clip.GetSize());
			//}

			PaintEventArgs e(sender,
			{ g, Point(), (clip - pc.Location) & Rect(g.GetSize()) });

			CallEvent<VisualEvent::Paint>(wgt, e);
			// NOTE: To keep %CommitInvalidation result correct, both
			//	components of the size shall be reset.
			rInvalidated.GetSizeRef() = {};
			return e.ClipArea;
		}
	}
	return{};
}

HUD_END
LEO_END