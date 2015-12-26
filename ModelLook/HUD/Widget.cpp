#include "Widget.h"
#include "HUDControl.h"
#include "HUDUtilities.h"
#include "HUDBrush.h"
LEO_BEGIN

HUD_BEGIN

ImplDeDtor(IWidget)

void SetBoundsOf(IWidget& wgt, const Rect& r)
{
	SetLocationOf(wgt, r.GetPoint());
	SetSizeOf(wgt, r.GetSize());
}

void SetBox(IWidget& wgt, const Box& b) {
	SetBoundsOf(wgt, b);
}

void
SetLocationOf(IWidget& wgt, const Point& pt)
{
	wgt.SetLocationOf(pt);
	//CallEvent<Move>(wgt, UIEventArgs(wgt));
}

void
SetSizeOf(IWidget& wgt, const Size& s)
{
	wgt.GetRenderer().SetSize(s);
	wgt.SetSizeOf(s);
	//CallEvent<Resize>(wgt, UIEventArgs(wgt));
}

void
SetInvalidationOf(IWidget& wgt)
{
	wgt.GetRenderer().CommitInvalidation(Rect(GetSizeOf(wgt)));
}

void
Invalidate(IWidget& wgt, const Rect& bounds)
{
	Rect r(bounds);

	for (auto p_wgt(&wgt); p_wgt; p_wgt = nullptr/*FetchContainerPtr(*p_wgt)*/)
	{
		r = p_wgt->GetRenderer().CommitInvalidation(r);
		r.GetPointRef() += GetLocationOf(*p_wgt);
	}
}

void
PaintChild(IWidget& wgt, PaintEventArgs&& e)
{
	auto& sender(e.GetSender());

	if (Clip(e.ClipArea, Rect(e.Location += GetLocationOf(sender),
		GetSizeOf(sender))))
		wgt.GetRenderer().Paint(sender, std::move(e));
}
Rect
PaintChild(IWidget& wgt, const PaintContext& pc)
{
	PaintEventArgs e(wgt, pc);

	PaintChild(wgt, std::move(e));
	return e.ClipArea;
}

void
PaintChildAndCommit(IWidget& wgt, PaintEventArgs& e)
{
	e.ClipArea |= PaintChild(wgt, e);
}

Widget::Widget(const Rect & r)
	:renderer_ptr(new HUDRenderer()), controller_ptr(new WidgetController({})),Background()
{
	InitializeEvents();
	SetBoundsOf(*this, r);
}

Widget::Widget(const Rect& r, HBrush b)
	: IWidget(), renderer_ptr(new HUDRenderer()),
	controller_ptr(new WidgetController({})), Background(b)
{
	InitializeEvents();
	SetBoundsOf(*this, r);
}
Widget::Widget(const Widget& wgt)
	: IWidget(),
	renderer_ptr(ClonePolymorphic(wgt.renderer_ptr)),
	controller_ptr(ClonePolymorphic(wgt.controller_ptr)),
	Background(wgt.Background)
{}
Widget::~Widget()
{
	//FetchGUIState().CleanupReferences(*this);
}

void
Widget::InitializeEvents()
{
	(FetchEvent<VisualEvent::Paint>(*this).Add(std::ref(Background), BackgroundPriority))
		+= std::bind(&Widget::Refresh, this, std::placeholders::_1);
}

HBrush
Widget::MakeAlphaBrush()
{
	return SolidBlendBrush(Drawing::ColorSpace::Trans);
}


void
Widget::SetRenderer(std::shared_ptr<HUDRenderer> p)
{
	renderer_ptr = p ? std::move(p) : std::make_shared<HUDRenderer>();
	renderer_ptr->SetSize(GetSizeOf());
}

void
Widget::Refresh(PaintEventArgs&& e)
{
	if (!e.ClipArea.IsUnStrictlyEmpty())
		for (auto pr(GetChildren()); pr.first != pr.second; ++pr.first)
			PaintVisibleChildAndCommit(*pr.first, e);
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
	return Box(mLocation,mSize);
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