//  File name:   HUD/Label.hpp
//  Version:     v1.00
//  Created:     12/1/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 标准部件事件定义

#ifndef HUD_WIDGETEVENT_HPP
#define HUD_WIDGETEVENT_HPP

#include "HUDGraphics.h"
#include "HUDEvent.hpp"

LEO_BEGIN

HUD_BEGIN
	/*!
	\brief 用户界面绘制优先级。
	*/
	//@{
	lconstexpr const EventPriority BackgroundPriority(0xC0);
	lconstexpr const EventPriority BoundaryPriority(0x60);
	lconstexpr const EventPriority ForegroundPriority(0x40);
	//@}

	/*!
	\brief 用户界面事件参数基类。
	*/
	struct LB_API UIEventArgs
	{
	private:
		IWidget* pSender;

	public:
		explicit
			UIEventArgs(IWidget& wgt)
			: pSender(&wgt)
		{}
		/*!
		\brief 复制构造：默认实现。
		\since build 295
		*/
		DefDeCopyCtor(UIEventArgs)
			/*!
			\brief 虚析构：类定义外默认实现。
			\since build 423
			*/
			virtual ~UIEventArgs();

		/*!
		\brief 复制赋值：默认实现。
		\since build 295
		*/
		DefDeCopyAssignment(UIEventArgs)

			DefGetter(const lnothrow, IWidget&, Sender, *pSender)
			PDefH(void, SetSender, IWidget& wgt)
			ImplExpr(pSender = &wgt)
	};

	/*!
	\brief 部件绘制参数。
	*/
	struct LB_API PaintEventArgs : public UIEventArgs,public PaintContext
	{
		PaintEventArgs(IWidget&);
		PaintEventArgs(IWidget&, const PaintContext&);
		DefDeCopyCtor(PaintEventArgs)
			/*!
			\brief 虚析构：类定义外默认实现。
			*/
			~PaintEventArgs() override;
	};

	using HPaintEvent = GHEvent<void(PaintEventArgs&&)>;

#define DefEventTypeMapping(_name, _tEventHandler) \
	template<> \
	struct EventTypeMapping<_name> \
	{ \
		using HandlerType = _tEventHandler; \
	};

	/*!
	\brief 标准控件事件空间。
	*/
	enum class VisualEvent:std::size_t {
		Move,
		ReSize,
		Paint,

		MaxEvent,
	};

	template<VisualEvent>
	struct EventTypeMapping
	{
		//定义 HandlerType 的默认值可能会导致运行期 dynamic_cast 失败。
		//	using HandlerType = HEvent;
	};

	DefEventTypeMapping(VisualEvent::Paint, HPaintEvent)

	/*!
	\brief 事件映射命名空间。
	*/
	namespace EventMapping
	{
		using MappedType = GEventPointerWrapper<UIEventArgs&&>; //!< 映射项类型。
		using ItemType = GIHEvent<UIEventArgs&&>;
		using PairType = std::pair<VisualEvent, MappedType>;
		using MapType = std::map<VisualEvent, MappedType>; //!< 映射表类型。
		using SearchResult = std::pair<typename MapType::iterator, bool>; //!< 搜索表结果类型。
	}

	using VisualEventMap = EventMapping::MapType;

	/*!
	\ingroup exception_types
	\brief 错误或不存在的部件事件异常。
	*/
	class LB_API BadEvent : public logged_event
	{
	public:
		using logged_event::logged_event;

		//@{
		DefDeCtor(BadEvent)
			DefDeCopyCtor(BadEvent)
			//! \brief 虚析构：类定义外默认实现。
			~BadEvent() override;
		//@}
	};


	using HBrush = std::function<void(PaintEventArgs&&)>;
	

	/*!
	\brief 构造指针指向的 VisualEvent 指定的事件对象。
	*/
	template<VisualEvent _vID>
	EventMapping::MappedType
		NewEvent()
	{
		return EventMapping::MappedType(new GEventWrapper<GEvent<typename
			EventTypeMapping<_vID>::HandlerType::FuncType>, UIEventArgs&&>());
	}

	template<VisualEvent _vID>
	GEvent<typename EventTypeMapping<_vID>::HandlerType::FuncType>&
		FetchEvent(AController& controller)
	{
		return dynamic_cast<GEvent<typename EventTypeMapping<_vID>::HandlerType
			::FuncType>&>(controller.GetItemRef(_vID, NewEvent<_vID>));
	}


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



template<class _tEventHandler>
size_t
DoEvent(AController& controller, VisualEvent id,
	typename EventArgsHead<typename _tEventHandler::TupleType>::type&& e)
{
	TryRet(dynamic_cast<GEvent<typename _tEventHandler::FuncType>&>(
		controller.GetItem(id))(std::move(e)))
		CatchIgnore(std::out_of_range&)
		CatchIgnore(std::bad_cast&)
		return 0;
}

/*!
\brief 在事件映射表中取指定 id 对应的事件。
*/
LB_API EventMapping::ItemType&
GetEvent(EventMapping::MapType&, VisualEvent, EventMapping::MappedType(&)());

HUD_END
LEO_END

#endif
