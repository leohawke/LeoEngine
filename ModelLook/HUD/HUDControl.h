#ifndef HUD_Control_H
#define HUD_Control_H

#include "WidgetEvent.h"
#include <type_traits>

LEO_BEGIN

HUD_BEGIN

/*!
\brief 调用部件事件，并忽略 BadEvent 异常。
\pre 事件参数需可转换为 EventTypeMapping 的 EventType 。
\note 若控件事件不存在则忽略。
*/
template<VisualEvent _vID, typename _tEventArgs>
inline size_t
CallEvent(IWidget& wgt, _tEventArgs&& e)
{
	using HandlerType = typename EventTypeMapping<_vID>::HandlerType;
	static_assert(std::is_convertible<std::remove_reference_t<_tEventArgs>,
		std::remove_reference_t<typename EventArgsHead<typename
		HandlerType::TupleType>::type >>::value,
		"Invalid event argument type found @ CallEvent;");

	TryRet(DoEvent<HandlerType>(wgt, _vID, std::move(e)))
		CatchIgnore(BadEvent&)
		return 0;
}

HUD_END

LEO_END

#endif
