#include "HUDPanel.h"

using namespace leo::HUD;

Panel::Panel(const Rect& r)
	: Control(r, MakeAlphaBrush()), MUIContainer()
{}

void
Panel::operator+=(IWidget& wgt)
{
	MUIContainer::operator+=(wgt);
	//SetContainerPtrOf(wgt, this);
}

bool
Panel::operator-=(IWidget& wgt)
{
	return RemoveFrom(wgt, *this) ? MUIContainer::operator-=(wgt) : false;
}

void
Panel::Add(IWidget& wgt, ZOrder z)
{
	MUIContainer::Add(wgt, z);
	//SetContainerPtrOf(wgt, this);
}

void
Panel::ClearContents()
{
	//ClearFocusingOf(*this);
	mWidgets.clear();
	SetInvalidationOf(*this);
}

bool
Panel::MoveToFront(IWidget& wgt)
{
	const auto i(std::find_if(mWidgets.cbegin(), mWidgets.cend(),
		[&](decltype(*mWidgets.cend()) pr) {
		return is_equal()(pr.second, wgt);
	}));

	if (i != mWidgets.cend())
	{
		const auto z(i->first);

		mWidgets.erase(i);
		mWidgets.emplace(z, ref(wgt));
		Invalidate(wgt);
		return true;
	}
	return{};
}

void
Panel::Refresh(PaintEventArgs&& e)
{
	if (!e.ClipArea.IsUnStrictlyEmpty())
		PaintVisibleChildren(e);
}