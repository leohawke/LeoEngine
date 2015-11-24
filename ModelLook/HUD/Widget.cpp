#include "Widget.h"

LEO_BEGIN

HUD_BEGIN

void SetBoundsOf(IWidget& w, const Rect& r)
{

}

bool Widget::IsVisible() const
{
	return mVisible;
}

bool Widget::Contains(platform::unit_type, platform::unit_type) const
{
	return false;
}

Size Widget::GetSizeOf() const {
	return mSize;
}

Box Widget::GetBox() const {
	return Box();
}

Point Widget::GetLocationOf() const {
	return mLocation;
}

void Widget::SetLocationOf(const Point& p) {
	mLocation = p;
}

void Widget::SetSizeOf(const Size& s) {
	mSize = s;
}

void Widget::SetVisible(bool b)
{
	mVisible = b;
}
HUD_END

LEO_END