#ifndef HUD_Control_H
#define HUD_Control_H

#include <type_traits>
#include <bitset>
LEO_BEGIN

HUD_BEGIN

/*!
\brief 抽象控制器。
\since build 243
*/
class LB_API AController : public cloneable
{
private:
	bool enabled; //!< 控件可用性。

public:
	/*!
	\brief 构造：使用指定可用性。
	*/
	AController(bool b = true)
		: enabled(b)
	{}
	//! \since build 586
	DefDeCopyCtor(AController)
		/*!
		\brief 虚析构：类定义外默认实现。
		\since build 295
		*/
		~AController() override;

	DefPred(const lnothrow, Enabled, enabled)
		/*!
		\brief 判断指定事件是否启用。
		\note 默认实现：仅启用 Paint 事件。
		\since build 581
		*/
		virtual PDefH(bool, IsEventEnabled, VisualEvent id) const
		ImplRet(id == VisualEvent::Paint)

		/*!
		\brief 取事件项。
		\since build 581
		*/
		DeclIEntry(EventMapping::ItemType& GetItem(VisualEvent) const)
		/*!
		\brief 取事件项，若不存在则用指定函数指针添加。
		\note 派生类的实现可能抛出异常并忽略加入任何事件项。
		\since build 581
		*/
		virtual PDefH(EventMapping::ItemType&, GetItemRef, VisualEvent id,
			EventMapping::MappedType(&)()) const
		ImplRet(GetItem(id))

		DefSetter(bool, Enabled, enabled)
		/*!
		\brief 设置指定事件是否启用。
		\throw ystdex::unsupported 不支持设置事件启用操作。
		\note 默认实现：总是抛出异常。
		\since build 581
		*/
		virtual PDefH(void, SetEventEnabled, VisualEvent, bool)
		ImplThrow(unsupported("AController::SetEventEnabled"))

		/*
		\brief 复制实例。
		\since build 409
		*/
		DeclIEntry(AController* clone() const ImplI(cloneable))
};


class LB_API Controller : public AController
{
public:
	/*!
	\brief 事件映射表。
	*/
	mutable EventMapping::MapType EventMap;

private:
	/*!
	\brief 指定是否启用的掩码。
	*/
	std::bitset<static_cast<std::size_t>(VisualEvent::MaxEvent)> event_mask;

public:
	explicit
		Controller(bool b)
		: AController(b), EventMap()
	{}
	//! \since build 368
	template<typename... _tParams>
	Controller(bool b, _tParams&&... args)
		: AController(b), EventMap(lforward(args)...)
	{}

	//! \since build 581
	//@{
	PDefH(bool, IsEventEnabled, VisualEvent id) const ImplI(AController)
		ImplRet(AController::IsEnabled() && !event_mask[static_cast<std::size_t>(id)])

		PDefH(EventMapping::ItemType&, GetItem, VisualEvent id) const
		ImplI(AController)
		ImplRet(EventMap.at(id))
		EventMapping::ItemType&
		GetItemRef(VisualEvent, EventMapping::MappedType(&)()) const override;
	//@}
	//! \brief 取事件映射表。
	DefGetter(const lnothrow, EventMapping::MapType&, EventMap, EventMap)

		PDefH(void, SetEventEnabled, VisualEvent id, bool b) ImplI(AController)
		ImplExpr(event_mask[static_cast<std::size_t>(id)] = !b)

		//! \since build 409
		DefClone(const ImplI(AController), Controller)
};


/*!
\ingroup helper_functions
\brief 取部件事件。
\tparam _vID 指定事件类型。
\param wgt 指定部件。
\exception BadEvent 异常中立：由控制器抛出。
\note 需要确保 EventTypeMapping 中有对应的 EventType ，否则无法匹配此函数模板。
\note 若控件事件不存在则自动添加空事件。
*/
template<VisualEvent _vID>
inline GEvent<typename EventTypeMapping<_vID>::HandlerType::FuncType>&
FetchEvent(IWidget& wgt)
{
	return FetchEvent<_vID>(wgt.GetController());
}

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

	TryRet(DoEvent<HandlerType>(wgt.GetController(), _vID, std::move(e)))
		CatchIgnore(BadEvent&)
		return 0;
}

/*!
\brief 部件控制器。
\since build 236
*/
class LB_API WidgetController : public AController
{
public:
	//! \since build 581
	mutable GEventWrapper<GEvent<void(PaintEventArgs&&)>, UIEventArgs&&> Paint;

	/*!
	\brief 构造：使用指定可用性。
	*/
	explicit
		WidgetController(bool = {});

	EventMapping::ItemType&
		GetItem(VisualEvent) const ImplI(AController);

	DefClone(const ImplI(AController), WidgetController)
};


class LB_API Control :public Widget
{
protected:
	DefExtendEventMap(LB_API ControlEventMap,VisualEventMap)
public:
	explicit
		Control(const Rect& = {});
	/*!
	\brief 构造：使用指定边界和背景画刷。
	\sa Control::Control
	*/
	explicit
		Control(const Rect&, HBrush);
	/*!
	\brief 复制构造：除容器为空外深复制。
	*/
	Control(const Control&);
	DefDeMoveCtor(Control)
	/*!
	\brief 虚析构：类定义外默认实现。
	*/
	~Control() override;
};

HUD_END

LEO_END

#endif
