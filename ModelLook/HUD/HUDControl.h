#ifndef HUD_Control_H
#define HUD_Control_H

#include <type_traits>

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
		ImplRet(id == Paint)

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

HUD_END

LEO_END

#endif
