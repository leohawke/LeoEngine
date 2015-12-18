#include "HUDContainer.h"
#include <container.hpp>
#include <iterator.hpp>
using namespace leo;
using namespace HUD;

bool
MUIContainer::operator-=(IWidget& wgt)
{
	auto t(mWidgets.size());

	erase_all_if(mWidgets, mWidgets.cbegin(), mWidgets.cend(),
		[&](decltype(*mWidgets.cend()) pr) {
		return is_equal()(pr.second, wgt);
	});
	t -= mWidgets.size();
	LAssert(t <= 1, "Duplicate widget references found.");
	return t != 0;
}

void
MUIContainer::Add(IWidget& wgt, ZOrder z)
{
	if (!Contains(wgt))
		mWidgets.emplace(z, ref(wgt));
}

bool
MUIContainer::Contains(IWidget& wgt)
{
	return std::count_if(mWidgets.cbegin(), mWidgets.cend(),
		[&](decltype(*mWidgets.cend()) pr) lnothrow{
		return is_equal()(pr.second, wgt);
	}) != 0;
}

void
MUIContainer::PaintVisibleChildren(PaintEventArgs& e)
{
	std::for_each(mWidgets.cbegin() | get_value, mWidgets.cend() | get_value,
		[&](const ItemType& wgt_ref) {
		PaintVisibleChildAndCommit(wgt_ref, e);
	});
}

ZOrder
MUIContainer::QueryZ(IWidget& wgt) const
{
	for (auto& pr : mWidgets)
		if (is_equal()(pr.second, wgt))
			return pr.first;
	throw std::out_of_range("Widget not found.");
}

MUIContainer::iterator
MUIContainer::begin()
{
	return mWidgets.rbegin() | get_value | get_get;
}

MUIContainer::iterator
MUIContainer::end()
{
	return mWidgets.rend() | get_value | get_get;
}

bool leo::HUD::RemoveFrom(IWidget & wgt, IWidget & con)
{
	//if (FetchContainerPtr(wgt) == &con)
	{
		//SetContainerPtrOf(wgt);
		//if (FetchFocusingPtr(con) == &wgt)
			//con.GetView().FocusingPtr = {};
		return true;
	}
	return{};
}
