#include "Widget.h"
#include "WidgetEvent.h"

#include <container.hpp>

LEO_BEGIN

HUD_BEGIN

UIEventArgs::~UIEventArgs()
{}

HUD::PaintEventArgs::PaintEventArgs(IWidget & wgt)
	:UIEventArgs(wgt),PaintContext()
{}
PaintEventArgs::PaintEventArgs(IWidget & wgt, const PaintContext & pc)
	:UIEventArgs(wgt),PaintContext(pc)
{}
ImplDeDtor(PaintEventArgs)

ImplDeDtor(BadEvent)


EventMapping::ItemType&
GetEvent(EventMapping::MapType& m, VisualEvent id,
	EventMapping::MappedType(&f)())
{
	auto pr(search_map(m, id));

	if (pr.second)
		pr.first = m.emplace_hint(pr.first, EventMapping::PairType(id, f()));
	return pr.first->second;
}

HUD_END

LEO_END


