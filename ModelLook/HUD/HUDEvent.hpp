//  File name:   HUD/Label.hpp
//  Version:     v1.00
//  Created:     12/1/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: HUD事件
#ifndef HUD_EVENT_HPP
#define HUD_EVENT_HPP

#include <ldef.h>
#include <BaseMacro.h>
#include <functional.hpp>
#include <ref.hpp>
#include <map>
LEO_BEGIN

namespace HUD
{
	/*!
	\brief 事件处理器接口模板。
	*/
	template<typename... _tParams>
	DeclDerivedI(,GIHEvent,cloneable)
		DeclIEntry(size_t operator()(_tParams...) const)
		DeclIEntry(GIHEvent* clone() const ImplI(cloneable))
	EndDecl

		template<typename... _tParams>
	GIHEvent<_tParams...>::DefDeDtor(GIHEvent)


	/*!
	\brief 标准事件处理器模板。
	*/
	//@{
	template<typename>
	class GHEvent;

	template<typename _tRet, typename... _tParams>
	class GHEvent<_tRet(_tParams...)>
		: protected std::function<_tRet(_tParams...)>
	{
	public:
		using TupleType = std::tuple<_tParams...>;
		using FuncType = _tRet(_tParams...);
		using BaseType = std::function<FuncType>;

	private:
		//! \brief 比较函数类型。
		using Comparer = bool(*)(const GHEvent&, const GHEvent&);
		template<class _tFunctor>
		struct GEquality
		{
			//! \since build v1.3
			//@{
			using decayed_type = std::decay_t<_tFunctor>;

			static bool
				AreEqual(const GHEvent& x, const GHEvent& y)
				lnoexcept_spec(std::declval<const decayed_type>()
					== std::declval<const decayed_type>())
			{
				const auto p(x.template target<decayed_type>());

				if (const auto q = y.template target<decayed_type>())
					return p == q || *p == *q;
				else
					return !p;
				return{};
			}
			//@}
		};

		Comparer comp_eq; //!< 比较函数：相等关系。

	public:
		/*!
		\brief 构造：使用函数指针。
		*/
		lconstfn
			GHEvent(FuncType* f = {})
			: BaseType(f), comp_eq(GEquality<FuncType>::AreEqual)
		{}
		/*!
		\brief 使用函数对象。
		*/
		template<class _fCallable>
		lconstfn
			GHEvent(_fCallable f, std::enable_if_t<
				std::is_constructible<BaseType, _fCallable>::value, int> = 0)
			: BaseType(f), comp_eq(GetComparer(f, f))
		{}
		/*!
		\brief 使用扩展函数对象。
		\todo 推断比较相等操作。
		*/
		template<class _fCallable>
		lconstfn
			GHEvent(_fCallable&& f, std::enable_if_t<
				!std::is_constructible<BaseType, _fCallable>::value, int> = 0)
			: BaseType(make_expanded<_tRet(_tParams...)>(lforward(f))),
			comp_eq(GHEvent::AreAlwaysEqual)
		{}
		/*!
		\brief 构造：使用对象引用和成员函数指针。
		\warning 使用空成员指针构造的函数对象调用引起未定义行为。
		*/
		template<class _type>
		lconstfn
			GHEvent(_type& obj, _tRet(_type::*pm)(_tParams...))
			: GHEvent([&, pm](_tParams... args) lnoexcept(
				noexcept((obj.*pm)(lforward(args)...))
				&& std::is_nothrow_copy_constructible<_tRet>::value) {
			return (obj.*pm)(lforward(args)...);
		})
		{}
		DefDeCopyMoveCtorAssignment(GHEvent)

			//! \since build 520
			lconstfn friend bool
			operator==(const GHEvent& x, const GHEvent& y)
		{
			return
#if defined(LB_DLL) || defined(LB_BUILD_DLL)
				x.BaseType::target_type() == y.BaseType::target_type()
#else
				x.comp_eq == y.comp_eq
#endif
				&& (x.comp_eq(x, y));
		}

		/*!
		\brief 调用。
		*/
		using BaseType::operator();

		//! \since build 516
		using BaseType::operator bool;

	private:
		//! \since build 319
		//@{
		template<typename _type>
		static lconstfn Comparer
			GetComparer(_type& x, _type& y, decltype(x == y) = {}) lnothrow
		{
			return GEquality<_type>::AreEqual;
		}
		template<typename _type, typename _tUnused>
		static lconstfn Comparer
			GetComparer(_type&, _tUnused&) lnothrow
		{
			return GHEvent::AreAlwaysEqual;
		}

		static lconstfn bool
			AreAlwaysEqual(const GHEvent&, const GHEvent&) lnothrow
		{
			return true;
		}
		//@}
	};
	//@}

	/*!
	\brief 事件优先级。
	*/
	using EventPriority = std::uint8_t;


	/*!
	\brief 默认事件优先级。
	*/
	lconstexpr const EventPriority DefaultEventPriority(0x80);


	/*!
	\brief 事件模板。
	\note 支持顺序多播。
	*/
	//@{
	template<typename>
	class GEvent;

	template<typename _tRet, typename... _tParams>
	class GEvent<_tRet(_tParams...)>
	{
	public:
		using HandlerType = GHEvent<_tRet(_tParams...)>;
		using TupleType = typename HandlerType::TupleType;
		using FuncType = typename HandlerType::FuncType;
		/*!
		\brief 容器类型。
		\since build 294
		*/
		using ContainerType
			= std::multimap<EventPriority, HandlerType, std::greater<EventPriority>>;
		//! \since build 573
		//@{
		using const_iterator = typename ContainerType::const_iterator;
		using const_reference = typename ContainerType::const_reference;
		using const_reverse_iterator
			= typename ContainerType::const_reverse_iterator;
		using iterator = typename ContainerType::iterator;
		using reference = typename ContainerType::reference;
		using reverse_iterator = typename ContainerType::reverse_iterator;
		using size_type = typename ContainerType::size_type;
		using value_type = typename ContainerType::value_type;
		//@}

	private:
		/*!
		\brief 响应列表。
		\since build 572
		*/
		ContainerType handlers;

	public:
		/*!
		\brief 无参数构造：默认实现。
		\note 得到空实例。
		*/
		lconstfn DefDeCtor(GEvent)
			/*!
			\brief 构造：添加事件处理器。
			\since build 448
			*/
			template<typename _tHandler,
			limpl(typename = exclude_self_ctor_t<GEvent, _tHandler>)>
			GEvent(_tHandler&& h)
			: handlers()
		{
			Add(lforward(h));
		}
		DefDeCopyMoveCtorAssignment(GEvent)

			/*!
			\brief 赋值：覆盖事件响应：使用单一构造参数指定的指定事件处理器。
			\since build 448
			*/
			template<typename _tHandler,
			limpl(typename = exclude_self_ctor_t<GEvent, _tHandler>)>
			inline GEvent&
			operator=(_tHandler&& _arg)
		{
			return *this = GEvent(forward(_arg));
		}

		/*!
		\brief 添加事件响应：使用 const 事件处理器和优先级。
		\note 不检查是否已经在列表中。
		*/
		inline PDefHOp(GEvent&, +=, const HandlerType& h)
			ImplRet(Add(h))
			/*!
			\brief 添加事件响应：使用事件处理器。
			\note 不检查是否已经在列表中。
			*/
			inline PDefHOp(GEvent&, +=, HandlerType&& h)
			ImplRet(Add(std::move(h)))
			/*!
			\brief 添加事件响应：目标为单一构造参数指定的指定事件处理器。
			\note 不检查是否已经在列表中。
			*/
			template<typename _type>
		inline GEvent&
			operator+=(_type&& _arg)
		{
			return Add(HandlerType(lforward(_arg)));
		}

		/*!
		\brief 移除事件响应：指定 const 事件处理器。
		*/
		GEvent&
			operator-=(const HandlerType& h)
		{
			ystdex::erase_all_if<ContainerType>(handlers, handlers.cbegin(),
				handlers.cend(), [&](decltype(*handlers.cbegin()) pr) {
				return pr.second == h;
			});
			return *this;
		}
		/*!
		\brief 移除事件响应：指定非 const 事件处理器。
		\note 防止模板 <tt>operator-=</tt> 递归。
		*/
		inline PDefHOp(GEvent&, -=, HandlerType&& h)
			ImplRet(*this -= static_cast<const HandlerType&>(h))
			/*!
			\brief 移除事件响应：目标为单一构造参数指定的指定事件处理器。
			\since build 293
			*/
			template<typename _type>
		inline GEvent&
			operator-=(_type&& _arg)
		{
			return *this -= HandlerType(forward(_arg));
		}

		/*!
		\brief 插入事件响应。
		\note 不检查是否已经在列表中。
		\sa Insert
		*/
		//@{
		/*!
		\note 使用 const 事件处理器和优先级。
		\since build 294
		*/
		inline PDefH(GEvent&, Add, const HandlerType& h,
			EventPriority prior = DefaultEventPriority)
			ImplRet(Insert(h, prior), *this)
			/*!
			\note 使用非 const 事件处理器和优先级。
			\since build 294
			*/
			inline PDefH(GEvent&, Add, HandlerType&& h,
				EventPriority prior = DefaultEventPriority)
			ImplRet(Insert(std::move(h), prior), *this)
			/*!
			\note 使用单一构造参数指定的事件处理器和优先级。
			\since build 294
			*/
			template<typename _type>
		inline GEvent&
			Add(_type&& _arg, EventPriority prior = DefaultEventPriority)
		{
			return Add(HandlerType(lforward(_arg)), prior);
		}
		/*!
		\note 使用对象引用、成员函数指针和优先级。
		\since build 413
		*/
		template<class _tObj, class _type>
		inline GEvent&
			Add(_tObj& obj, _tRet(_type::*pm)(_tParams...),
				EventPriority prior = DefaultEventPriority)
		{
			return Add(HandlerType(static_cast<_type&>(obj), std::move(pm)), prior);
		}
		//@}

		/*!
		\brief 插入事件响应。
		\note 不检查是否已经在列表中。
		\since build 572
		*/
		//@{
		//! \note 使用 const 事件处理器和优先级。
		inline PDefH(typename ContainerType::iterator, Insert, const HandlerType& h,
			EventPriority prior = DefaultEventPriority)
			ImplRet(handlers.emplace(prior, h))
			//! \note 使用非 const 事件处理器和优先级。
			inline PDefH(typename ContainerType::iterator, Insert, HandlerType&& h,
				EventPriority prior = DefaultEventPriority)
			ImplRet(handlers.emplace(prior, std::move(h)))
			//! \note 使用单一构造参数指定的事件处理器和优先级。
			template<typename _type>
		inline typename ContainerType::iterator
			Insert(_type&& _arg, EventPriority prior = DefaultEventPriority)
		{
			return Insert(HandlerType(forward(_arg)), prior);
		}
		//! \note 使用对象引用、成员函数指针和优先级。
		template<class _tObj, class _type>
		inline typename ContainerType::iterator
			Insert(_tObj& obj, _tRet(_type::*pm)(_tParams...),
				EventPriority prior = DefaultEventPriority)
		{
			return
				Insert(HandlerType(static_cast<_type&>(obj), std::move(pm)), prior);
		}
		//@}

		/*!
		\brief 移除事件响应：目标为指定对象引用和成员函数指针。
		\since build 413
		*/
		template<class _tObj, class _type>
		inline GEvent&
			Remove(_tObj& obj, _tRet(_type::*pm)(_tParams...))
		{
			return *this -= HandlerType(static_cast<_type&>(obj), std::move(pm));
		}

		/*!
		\brief 判断是否包含指定事件响应。
		*/
		bool
			Contains(const HandlerType& h) const
		{
			using get_value;

			return std::count(handlers.cbegin() | get_value,
				handlers.cend() | get_value, h) != 0;
		}
		/*!
		\brief 判断是否包含单一构造参数指定的事件响应。
		\since build 293
		*/
		template<typename _type>
		inline bool
			Contains(_type&& _arg) const
		{
			return Contains(HandlerType(lforward(_arg)));
		}

		/*!
		\brief 调用事件处理器。
		\return 成功调用的事件处理器个数。
		\exception std::bad_function_call 以外异常中立。
		\since build 573
		*/
		size_type
			operator()(_tParams... args) const
		{
			size_type n(0);

			for (const auto& pr : handlers)
			{
				TryExpr(pr.second(lforward(args)...))
					CatchIgnore(std::bad_function_call&)
					++n;
			}
			return n;
		}

		//! \since build 573
		PDefH(const_iterator, cbegin, ) const lnothrow
			ImplRet(handlers.cbegin())

			//! \since build 572
			//@{
			PDefH(iterator, begin, ) lnothrow
			ImplRet(handlers.begin())
			PDefH(iterator, begin, ) const lnothrow
			ImplRet(handlers.begin())

			//! \since build 573
			PDefH(const_iterator, cend, ) const lnothrow
			ImplRet(handlers.cend())

			//! \brief 清除：移除所有事件响应。
			PDefH(void, clear, ) lnothrow
			ImplRet(handlers.clear())

			//! \since build 573
			//@{
			PDefH(size_type, count, EventPriority prior) const lnothrow
			ImplRet(handlers.count(prior))

			PDefH(const_reverse_iterator, crbegin, ) const lnothrow
			ImplRet(handlers.crbegin())

			PDefH(const_reverse_iterator, crend, ) const lnothrow
			ImplRet(handlers.crend())
			//@}

			PDefH(bool, empty, ) const lnothrow
			ImplRet(handlers.empty())

			PDefH(iterator, end, ) lnothrow
			ImplRet(handlers.end())
			PDefH(iterator, end, ) const lnothrow
			ImplRet(handlers.end())
			//@}

			//! \since build 573
			//@{
			PDefH(reverse_iterator, rbegin, ) lnothrow
			ImplRet(handlers.rbegin())

			PDefH(reverse_iterator, rend, ) lnothrow
			ImplRet(handlers.rend())

			//! \brief 取列表中的响应数。
			PDefH(size_type, size, ) const lnothrow
			ImplRet(handlers.size())
			//@}

			/*
			\brief 交换。
			\since build 409
			*/
			PDefH(void, swap, GEvent& e) lnothrow
			ImplRet(handlers.swap(e))
	};
	//@}


	/*!
	\brief 添加单一事件响应：删除后添加。
	*/
	//@{
	template<typename _tRet, typename... _tParams>
	inline GEvent<_tRet(_tParams...)>&
		AddUnique(GEvent<_tRet(_tParams...)>& evt,
			const typename GEvent<_tRet(_tParams...)>::HandlerType& h,
			EventPriority prior = DefaultEventPriority)
	{
		return (evt -= h).Add(h, prior);
	}
	template<typename _tRet, typename... _tParams>
	inline GEvent<_tRet(_tParams...)>&
		AddUnique(GEvent<_tRet(_tParams...)>& evt, typename GEvent<_tRet(_tParams...)>
			::HandlerType&& h, EventPriority prior = DefaultEventPriority)
	{
		return (evt -= h).Add(std::move(h), prior);
	}
	template<typename _type, typename _tRet, typename... _tParams>
	inline GEvent<_tRet(_tParams...)>&
		AddUnique(GEvent<_tRet(_tParams...)>& evt, _type&& arg,
			EventPriority prior = DefaultEventPriority)
	{
		return AddUnique(evt, HandlerType(yforward(arg)), prior);
	}
	template<class _type, typename _tRet, typename... _tParams>
	inline GEvent<_tRet(_tParams...)>&
		AddUnique(GEvent<_tRet(_tParams...)>& evt, _type& obj,
			_tRet(_type::*pm)(_tParams...), EventPriority prior = DefaultEventPriority)
	{
		return AddUnique(evt, HandlerType(static_cast<_type&>(obj), std::move(pm)),
			prior);
	}
	//@}

	/*!
	\relates GEvent
	*/
	template<typename _tRet, typename... _tParams>
	inline DefSwap(lnothrow, GEvent<_tRet(_tParams...)>)


		/*!
		\brief 使用 RAII 管理的事件辅助类。
		\warning 非虚析构。
		*/
	template<typename... _tEventArgs>
	class GEventGuard
	{
	public:
		using EventType = GEvent<_tEventArgs...>;
		using HandlerType = GHEvent<_tEventArgs...>;
		//! \since build 554
		lref<EventType> Event;
		HandlerType Handler;

		template<typename _type>
		GEventGuard(EventType& evt, _type&& handler,
			EventPriority prior = DefaultEventPriority)
			: Event(evt), Handler(yforward(handler))
		{
			Event.get().Add(Handler, prior);
		}
		~GEventGuard()
		{
			Event.get() -= Handler;
		}
	};

	template<typename... _tParams>
	struct EventArgsHead
	{
		using type = std::conditional_t<sizeof...(_tParams) == 0, void,
			std::tuple_element_t<0, std::tuple<_tParams...>>>;
	};


	template<typename... _tParams>
	struct EventArgsHead<std::tuple<_tParams...>> : EventArgsHead<_tParams...>
	{};


	/*!
	\brief 事件包装类模板。
	*/
	template<class _tEvent, typename _tBaseArgs>
	class GEventWrapper : public _tEvent, implements GIHEvent<_tBaseArgs>
	{
	public:
		using EventType = _tEvent;
		using BaseArgsType = _tBaseArgs;
		using EventArgsType
			= typename EventArgsHead<typename _tEvent::TupleType>::type;

		/*!
		\brief 委托调用。
		\warning 需要确保 BaseArgsType 引用的对象能够转换至 EventArgsType 。
		*/
		size_t
			operator()(BaseArgsType e) const ImplI(GIHEvent<_tBaseArgs>)
		{
			return EventType::operator()(EventArgsType(lforward(e)));
		}

		DefClone(const ImplI(GIHEvent<_tBaseArgs>), GEventWrapper)
	};

	/*!
	\brief 事件项类型。
	\warning 非虚析构。
	*/
	template<typename _tBaseArgs>
	class GEventPointerWrapper
	{
	public:
		using ItemType = GIHEvent<_tBaseArgs>;
		using PointerType = std::unique_ptr<ItemType>;

	private:
		PointerType ptr;

	public:
		template<typename _type, limpl(
			typename = exclude_self_ctor_t<GEventPointerWrapper, _type>)>
			inline
			GEventPointerWrapper(_type&& p)
			lnoexcept(std::is_nothrow_constructible<PointerType, _type>::value)
			: ptr(Nonnull(p))
		{}
		/*!
		\brief 复制构造：深复制。
		*/
		GEventPointerWrapper(const GEventPointerWrapper& item)
			: ptr(ClonePolymorphic(item.ptr))
		{}
		DefDeMoveCtor(GEventPointerWrapper)

		lconstfn DefCvt(const lnothrow, const ItemType&, *ptr)
		lconstfn DefCvt(const lnothrow, ItemType&, *ptr)
	};
}

LEO_END

#endif
