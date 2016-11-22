/*! \file ValueNode.h
\ingroup LScheme
\brief 值类型节点。

*/
#ifndef LScheme_ValueNode_H
#define LScheme_ValueNode_H 1

#include "LObject.h"

#include <LBase/set.hpp>
#include <LBase/LPath.hpp>

#include <numeric>

namespace leo {
	lconstexpr const struct ListContainerTag {} ListContainer{};

	lconstexpr const struct NoContainerTag {} NoContainer{};

	class LB_API ValueNode : private totally_ordered<ValueNode>,
		private totally_ordered<ValueNode, string>
	{
	public:
		using Container = mapped_set<ValueNode, less<>>;
		using key_type = typename Container::key_type;
		using iterator = Container::iterator;
		using const_iterator = Container::const_iterator;
		using reverse_iterator = Container::reverse_iterator;
		using const_reverse_iterator = Container::const_reverse_iterator;
	private:
		string name{};
		/*!
		\brief 子节点容器。
		*/
		Container container{};

	public:
		ValueObject Value{};

		DefDeCtor(ValueNode)
			/*!
			\brief 构造：使用容器对象。
			*/
			ValueNode(Container con)
			: container(std::move(con))
		{}
		/*!
		\brief 构造：使用字符串引用和值类型对象构造参数。
		\note 不使用容器。
		*/
		template<typename _tString, typename... _tParams>
		inline
			ValueNode(NoContainerTag, _tString&& str, _tParams&&... args)
			: name(lforward(str)), Value(lforward(args)...)
		{}
		/*!
		\brief 构造：使用容器对象、字符串引用和值类型对象构造参数。
		*/
		template<typename _tString, typename... _tParams>
		ValueNode(Container con, _tString&& str, _tParams&&... args)
			: name(lforward(str)), container(std::move(con)),
			Value(lforward(args)...)
		{}
		/*!
		\brief 构造：使用输入迭代器对。
		*/
		template<typename _tIn>
		inline
			ValueNode(const pair<_tIn, _tIn>& pr)
			: container(pr.first, pr.second)
		{}
		/*!
		\brief 构造：使用输入迭代器对、字符串引用和值参数。
		*/
		template<typename _tIn, typename _tString>
		inline
			ValueNode(const pair<_tIn, _tIn>& pr, _tString&& str)
			: name(lforward(str)), container(pr.first, pr.second)
		{}
		/*!
		\brief 原地构造：使用容器、名称和值的参数元组。
		*/
		//@{
		template<typename... _tParams1>
		inline
			ValueNode(std::tuple<_tParams1...> args1)
			: container(make_from_tuple<Container>(args1))
		{}
		template<typename... _tParams1, typename... _tParams2>
		inline
			ValueNode(std::tuple<_tParams1...> args1, std::tuple<_tParams2...> args2)
			: name(make_from_tuple<string>(args2)),
			container(make_from_tuple<Container>(args1))
		{}
		template<typename... _tParams1, typename... _tParams2,
			typename... _tParams3>
			inline
			ValueNode(std::tuple<_tParams1...> args1, std::tuple<_tParams2...> args2,
				std::tuple<_tParams3...> args3)
			: name(make_from_tuple<string>(args2)),
			container(make_from_tuple<Container>(args1)),
			Value(make_from_tuple<Container>(args3))
		{}
		//@}

		DefDeCopyMoveCtor(ValueNode)

			/*!
			\brief 合一赋值：使用值参数和交换函数进行复制或转移赋值。
			*/
			PDefHOp(ValueNode&, =, ValueNode node) lnothrow
			ImplRet(swap(node, *this), *this)

			DefBoolNeg(explicit, bool(Value) || !container.empty())

			//@{
			PDefHOp(const ValueNode&, +=, const ValueNode& node)
			ImplRet(Add(node), *this)
			PDefHOp(const ValueNode&, +=, ValueNode&& node)
			ImplRet(Add(std::move(node)), *this)

			PDefHOp(const ValueNode&, -=, const ValueNode& node)
			ImplRet(Remove(node), *this)
			PDefHOp(const ValueNode&, -=, const string& str)
			ImplRet(Remove(str), *this)
			/*!
			\brief 替换同名子节点。
			\return 自身引用。
			*/
			//@{
			PDefHOp(ValueNode&, /=, const ValueNode& node)
			ImplRet(*this %= node, *this)
			PDefHOp(ValueNode&, /=, ValueNode&& node)
			ImplRet(*this %= std::move(node), *this)
			//@}
			/*!
			\brief 替换同名子节点。
			\return 子节点引用。
			*/
			//@{
			const ValueNode&
			operator%=(const ValueNode&);
		const ValueNode&
			operator%=(const ValueNode&&);
		//@}
		//@}

		friend PDefHOp(bool, == , const ValueNode& x, const ValueNode& y) lnothrow
			ImplRet(x.name == y.name)
			friend PDefHOp(bool, == , const ValueNode& x, const string& str) lnothrow
			ImplRet(x.name == str)
			template<typename _tKey>
		friend bool
			operator==(const ValueNode& x, const _tKey& str) lnothrow
		{
			return x.name == str;
		}

		friend PDefHOp(bool, <, const ValueNode& x, const ValueNode& y) lnothrow
			ImplRet(x.name < y.name)
			friend PDefHOp(bool, <, const ValueNode& x, const string& str) lnothrow
			ImplRet(x.name < str)
			template<typename _tKey>
		friend bool
			operator<(const ValueNode& x, const _tKey& str) lnothrow
		{
			return x.name < str;
		}
		template<typename _tKey>
		friend bool
			operator<(const _tKey& str, const ValueNode& y) lnothrow
		{
			return str < y.name;
		}
		friend PDefHOp(bool, >, const ValueNode& x, const string& str) lnothrow
			ImplRet(x.name > str)
			template<typename _tKey>
		friend bool
			operator>(const ValueNode& x, const _tKey& str) lnothrow
		{
			return x.name > str;
		}
		template<typename _tKey>
		friend bool
			operator>(const _tKey& str, const ValueNode& y) lnothrow
		{
			return str > y.name;
		}

		template<typename _tString>
		ValueNode&
			operator[](_tString&& str)
		{
			return *leo::try_emplace(container, str, NoContainer, lforward(str))
				.first;
		}
		template<class _tCon>
		const ValueNode&
			operator[](const lpath<_tCon>& pth)
		{
			auto p(this);

			for (const auto& n : pth)
				p = &(*p)[n];
			return *p;
		}

		/*!
		\brief 取子节点容器引用。
		*/
		DefGetter(const lnothrow, const Container&, Container, container)
			/*!
			\brief 取子节点容器引用。
			*/
			DefGetter(lnothrow, Container&, ContainerRef, container)
			DefGetter(const lnothrow, const string&, Name, name)

			//@{
			//! \brief 设置子节点容器内容。
			PDefH(void, SetChildren, Container con)
			ImplExpr(container = std::move(con))
			/*!
			\note 设置子节点容器和值的内容。
			*/
			//@{
			void
			SetContent(Container, ValueObject) lnothrow;
		PDefH(void, SetContent, const ValueNode& node)
			ImplExpr(SetContent(node.GetContainer(), node.Value))
			PDefH(void, SetContent, ValueNode&& node)
			ImplExpr(SwapContent(node))
			//@}

			PDefH(bool, Add, const ValueNode& node)
			ImplRet(insert(node).second)
			PDefH(bool, Add, ValueNode&& node)
			ImplRet(insert(std::move(node)).second)
			template<typename _tString, typename... _tParams>
		inline bool
			Add(_tString&& str, _tParams&&... args)
		{
			return AddValueTo(container, lforward(str), lforward(args)...);
		}

		template<typename _tString, typename... _tParams>
		static bool
			AddValueTo(Container& con, _tString&& str, _tParams&&... args)
		{
			const auto pr(con.equal_range(str));

			if (pr.first == pr.second)
			{
				con.emplace_hint(pr.first, NoContainer, lforward(str),
					lforward(args)...);
				return true;
			}
			return{};
		}

		//! \note 清理容器和修改值的操作之间的顺序未指定。
		//@{
		/*!
		\brief 清除节点。
		\post <tt>!Value && empty()</tt> 。
		*/
		PDefH(void, Clear, ) lnothrow
			ImplExpr(Value.Clear(), ClearContainer())

			/*!
			\brief 清除容器并设置值。
			*/
			PDefH(void, ClearTo, ValueObject vo) lnothrow
			ImplExpr(ClearContainer(), Value = std::move(vo))
			//@}

			/*!
			\brief 清除节点容器。
			\post \c empty() 。
			*/
			PDefH(void, ClearContainer, ) lnothrow
			ImplExpr(container.clear())
			//@}

			/*!
			\brief 若指定名称子节点不存在则按指定值初始化。
			\return 按指定名称查找的指定类型的子节点的值的引用。
			*/
			template<typename _type, typename _tString, typename... _tParams>
		inline _type&
			Place(_tString&& str, _tParams&&... args)
		{
			return this->try_emplace(str, NoContainer, lforward(str),
				in_place<_type>, lforward(args)...).first->Value.template
				GetObject<_type>();
		}

		PDefH(bool, Remove, const ValueNode& node)
			ImplRet(container.erase(node) != 0)
			template<typename _tKey>
		inline bool
			Remove(const _tKey& k)
		{
			return erase_first(container, k);
		}

		/*!
		\brief 复制满足条件的子节点。
		*/
		template<typename _func>
		Container
			SelectChildren(_func f) const
		{
			Container res;

			for_each_if(begin(), end(), f, [&](const ValueNode& nd) {
				res.insert(nd);
			});
			return res;
		}

		//@{
		//! \brief 转移满足条件的子节点。
		template<typename _func>
		Container
			SplitChildren(_func f)
		{
			Container res;

			std::for_each(begin(), end(), [&](const ValueNode& nd) {
				container.emplace(NoContainer, res, nd.GetName());
			});
			for_each_if(begin(), end(), f, [&, this](const ValueNode& nd) {
				const auto& child_name(nd.GetName());

				Deref(res.find(child_name)).Value = std::move(nd.Value);
				Remove(child_name);
			});
			return res;
		}

		//! \warning 不检查容器之间的所有权，保持循环引用状态析构引起未定义行为。
		//@{
		//! \brief 交换容器。
		PDefH(void, SwapContainer, ValueNode& node) lnothrow
			ImplExpr(container.swap(node.container))

			//! \brief 交换容器和值。
			void
			SwapContent(ValueNode&) lnothrow;
		//@}
		//@}

		/*!
		\brief 抛出索引越界异常。
		\throw std::out_of_range 索引越界。
		*/
		LB_NORETURN static void
			ThrowIndexOutOfRange();

		/*!
		\brief 抛出名称错误异常。
		\throw std::out_of_range 名称错误。
		*/
		LB_NORETURN static void
			ThrowWrongNameFound();

		//@{
		PDefH(iterator, begin, )
			ImplRet(GetContainerRef().begin())
			PDefH(const_iterator, begin, ) const
			ImplRet(GetContainer().begin())

			DefFwdTmpl(const, pair<iterator LPP_Comma bool>, emplace,
				container.emplace(lforward(args)...))

			DefFwdTmpl(, iterator, emplace_hint,
				container.emplace_hint(lforward(args)...))

			PDefH(bool, empty, ) const lnothrow
			ImplRet(container.empty())

			PDefH(iterator, end, )
			ImplRet(GetContainerRef().end())
			PDefH(const_iterator, end, ) const
			ImplRet(GetContainer().end())
			//@}

			DefFwdTmpl(-> decltype(container.insert(lforward(args)...)), auto,
				insert, container.insert(lforward(args)...))

			//@{
			template<typename _tKey, class _tParam>
		limpl(enable_if_inconvertible_t)<_tKey&&, const_iterator,
			std::pair<iterator, bool>>
			insert_or_assign(_tKey&& k, _tParam&& arg)
		{
			return insert_or_assign(container, lforward(k), lforward(arg));
		}
		template<typename _tKey, class _tParam>
		iterator
			insert_or_assign(const_iterator hint, _tKey&& k, _tParam&& arg)
		{
			return insert_or_assign_hint(container, hint, lforward(k),
				lforward(arg));
		}
		//@}

		//@{
		PDefH(reverse_iterator, rbegin, )
			ImplRet(GetContainerRef().rbegin())
			PDefH(const_reverse_iterator, rbegin, ) const
			ImplRet(GetContainer().rbegin())

			PDefH(reverse_iterator, rend, )
			ImplRet(GetContainerRef().rend())
			PDefH(const_reverse_iterator, rend, ) const
			ImplRet(GetContainer().rend())
			//@}

			/*!
			\sa mapped_set
			\sa set_value_move
			*/
			friend PDefH(ValueNode, set_value_move, ValueNode& node)
			ImplRet({ std::move(node.GetContainerRef()), node.GetName(),
				std::move(node.Value) })

			PDefH(size_t, size, ) const lnothrow
			ImplRet(container.size())

			/*!
			\brief 交换。
			*/
			LB_API friend void
			swap(ValueNode&, ValueNode&) lnothrow;

		//@{
		template<typename _tKey, typename... _tParams>
		limpl(enable_if_inconvertible_t)<_tKey&&, const_iterator,
			std::pair<iterator, bool>>
			try_emplace(_tKey&& k, _tParams&&... args)
		{
			return leo::try_emplace(container, lforward(k),std::forward<_tParams>(args)...);
		}
		template<typename _tKey, typename... _tParams>
		iterator
			try_emplace(const_iterator hint, _tKey&& k, _tParams&&... args)
		{
			return try_emplace_hint(container, hint, lforward(k),
				std::forward<_tParams>(args)...);
		}
		//@}
	};

	/*!
	\relates ValueNode
	*/
	//@{
	/*!
	\brief 访问节点的指定类型对象。
	\exception std::bad_cast 空实例或类型检查失败 。
	*/
	//@{
	template<typename _type>
	inline _type&
		Access(ValueNode& node)
	{
		return node.Value.Access<_type>();
	}
	template<typename _type>
	inline const _type&
		Access(const ValueNode& node)
	{
		return node.Value.Access<_type>();
	}
	//@}

	//@{
	//! \brief 访问节点的指定类型对象指针。
	//@{
	template<typename _type>
	inline observer_ptr<_type>
		AccessPtr(ValueNode& node) lnothrow
	{
		return node.Value.AccessPtr<_type>();
	}
	template<typename _type>
	inline observer_ptr<const _type>
		AccessPtr(const ValueNode& node) lnothrow
	{
		return node.Value.AccessPtr<_type>();
	}
	//@}
	//! \brief 访问节点的指定类型对象指针。
	//@{
	template<typename _type>
	inline observer_ptr<_type>
		AccessPtr(observer_ptr<ValueNode> p_node) lnothrow
	{
		return p_node ? AccessPtr<_type>(*p_node) : nullptr;
	}
	template<typename _type>
	inline observer_ptr<const _type>
		AccessPtr(observer_ptr<const ValueNode> p_node) lnothrow
	{
		return p_node ? AccessPtr<_type>(*p_node) : nullptr;
	}
	//@}
	//@}
	//@}


	//@{
	template<typename _tKey>
	observer_ptr<ValueNode>
		AccessNodePtr(ValueNode::Container*, const _tKey&) lnothrow;
	template<typename _tKey>
	observer_ptr<const ValueNode>
		AccessNodePtr(const ValueNode::Container*, const _tKey&) lnothrow;

	/*!
	\brief 访问节点。
	\throw std::out_of_range 未找到对应节点。
	*/
	//@{
	LB_API ValueNode&
		AccessNode(ValueNode::Container*, const string&);
	//! \since build 670
	LB_API const ValueNode&
		AccessNode(const ValueNode::Container*, const string&);
	template<typename _tKey>
	ValueNode&
		AccessNode(ValueNode::Container* p_con, const _tKey& name)
	{
		if (const auto p = AccessNodePtr(p_con, name))
			return *p;
		ValueNode::ThrowWrongNameFound();
	}
	template<typename _tKey>
	const ValueNode&
		AccessNode(const ValueNode::Container* p_con, const _tKey& name)
	{
		if (const auto p = AccessNodePtr(p_con, name))
			return *p;
		ValueNode::ThrowWrongNameFound();
	}
	template<typename _tKey>
	inline ValueNode&
		AccessNode(observer_ptr<ValueNode::Container> p_con, const _tKey& name)
	{
		return AccessNode(p_con.get(), name);
	}
	template<typename _tKey>
	inline const ValueNode&
		AccessNode(observer_ptr<const ValueNode::Container> p_con, const _tKey& name)
	{
		return AccessNode(p_con.get(), name);
	}
	template<typename _tKey>
	inline ValueNode&
		AccessNode(ValueNode::Container& con, const _tKey& name)
	{
		return AccessNode(&con, name);
	}
	template<typename _tKey>
	inline const ValueNode&
		AccessNode(const ValueNode::Container& con, const _tKey& name)
	{
		return AccessNode(&con, name);
	}
	/*!
	\note 时间复杂度 O(n) 。
	*/
	//@{
	LB_API ValueNode&
		AccessNode(ValueNode&, size_t);
	LB_API const ValueNode&
		AccessNode(const ValueNode&, size_t);
	//@}
	template<typename _tKey, limpl(typename = typename enable_if_t<
		or_<std::is_constructible<const _tKey&, const string&>,
		std::is_constructible<const string&, const _tKey&>>::value>)>
		inline ValueNode&
		AccessNode(ValueNode& node, const _tKey& name)
	{
		return AccessNode(node.GetContainerRef(), name);
	}
	template<typename _tKey, limpl(typename = typename enable_if_t<
		or_<std::is_constructible<const _tKey&, const string&>,
		std::is_constructible<const string&, const _tKey&>>::value>)>
		inline const ValueNode&
		AccessNode(const ValueNode& node, const _tKey& name)
	{
		return AccessNode(node.GetContainer(), name);
	}
	//@{
	//! \note 使用 ADL \c AccessNode 。
	template<class _tNode, typename _tIn>
	_tNode&&
		AccessNode(_tNode&& node, _tIn first, _tIn last)
	{
		return std::accumulate(first, last, ref(node),
			[](_tNode&& nd, decltype(*first) c) {
			return ref(AccessNode(nd, c));
		});
	}
	//! \note 使用 ADL \c begin 和 \c end 指定范围迭代器。
	template<class _tNode, typename _tRange,
		limpl(typename = typename enable_if_t<
			!std::is_constructible<const string&, const _tRange&>::value>)>
		inline auto
		AccessNode(_tNode&& node, const _tRange& c)
		-> decltype(AccessNode(lforward(node), begin(c), end(c)))
	{
		return AccessNode(lforward(node), begin(c), end(c));
	}
	//@}
	//@}

	//! \brief 访问节点指针。
	//@{
	LB_API observer_ptr<ValueNode>
		AccessNodePtr(ValueNode::Container&, const string&) lnothrow;
	LB_API observer_ptr<const ValueNode>
		AccessNodePtr(const ValueNode::Container&, const string&) lnothrow;
	template<typename _tKey>
	observer_ptr<ValueNode>
		AccessNodePtr(ValueNode::Container& con, const _tKey& name) lnothrow
	{
		return make_observer(call_value_or<ValueNode*>(addrof<>(),
			con.find(name), {}, end(con)));
	}
	template<typename _tKey>
	observer_ptr<const ValueNode>
		AccessNodePtr(const ValueNode::Container& con, const _tKey& name) lnothrow
	{
		return make_observer(call_value_or<const ValueNode*>(
			addrof<>(), con.find(name), {}, end(con)));
	}
	template<typename _tKey>
	inline observer_ptr<ValueNode>
		AccessNodePtr(ValueNode::Container* p_con, const _tKey& name) lnothrow
	{
		return p_con ? AccessNodePtr(*p_con, name) : nullptr;
	}
	template<typename _tKey>
	inline observer_ptr<const ValueNode>
		AccessNodePtr(const ValueNode::Container* p_con, const _tKey& name) lnothrow
	{
		return p_con ? AccessNodePtr(*p_con, name) : nullptr;
	}
	template<typename _tKey>
	inline observer_ptr<ValueNode>
		AccessNodePtr(observer_ptr<ValueNode::Container> p_con, const _tKey& name)
		lnothrow
	{
		return p_con ? AccessNodePtr(*p_con, name) : nullptr;
	}
	template<typename _tKey>
	inline observer_ptr<const ValueNode>
		AccessNodePtr(observer_ptr<const ValueNode::Container> p_con, const _tKey& name)
		lnothrow
	{
		return p_con ? AccessNodePtr(*p_con, name) : nullptr;
	}
	/*!
	\note 时间复杂度 O(n) 。
	*/
	//@{
	LB_API observer_ptr<ValueNode>
		AccessNodePtr(ValueNode&, size_t);
	LB_API observer_ptr<const ValueNode>
		AccessNodePtr(const ValueNode&, size_t);
	//@}
	template<typename _tKey, limpl(typename = typename enable_if_t<
		or_<std::is_constructible<const _tKey&, const string&>,
		std::is_constructible<const string&, const _tKey&>>::value>)>
		inline observer_ptr<ValueNode>
		AccessNodePtr(ValueNode& node, const _tKey& name)
	{
		return AccessNodePtr(node.GetContainerRef(), name);
	}
	template<typename _tKey, limpl(typename = typename enable_if_t<
		or_<std::is_constructible<const _tKey&, const string&>,
		std::is_constructible<const string&, const _tKey&>>::value>)>
		inline observer_ptr<const ValueNode>
		AccessNodePtr(const ValueNode& node, const _tKey& name)
	{
		return AccessNodePtr(node.GetContainer(), name);
	}
	//@{
	//! \note 使用 ADL \c AccessNodePtr 。
	template<class _tNode, typename _tIn>
	auto
		AccessNodePtr(_tNode&& node, _tIn first, _tIn last)
		-> decltype(make_obsrever(std::addressof(node)))
	{
		// TODO: Simplified using algorithm template?
		for (auto p(make_observer(std::addressof(node))); p && first != last;
			++first)
			p = AccessNodePtr(*p, *first);
		return first;
	}
	//! \note 使用 ADL \c begin 和 \c end 指定范围迭代器。
	template<class _tNode, typename _tRange,
		limpl(typename = typename enable_if_t<
			!std::is_constructible<const string&, const _tRange&>::value>)>
		inline auto
		AccessNodePtr(_tNode&& node, const _tRange& c)
		-> decltype(AccessNodePtr(lforward(node), begin(c), end(c)))
	{
		return AccessNodePtr(lforward(node), begin(c), end(c));
	}
	//@}
	//@}
	//@}


	//@{
	/*!
	\exception std::bad_cast 空实例或类型检查失败 。
	\relates ValueNode
	*/
	//@{
	/*!
	\brief 访问子节点的指定类型对象。
	\note 使用 ADL \c AccessNode 。
	*/
	//@{
	template<typename _type, typename... _tParams>
	inline _type&
		AccessChild(ValueNode& node, _tParams&&... args)
	{
		return Access<_type>(AccessNode(node, lforward(args)...));
	}
	template<typename _type, typename... _tParams>
	inline const _type&
		AccessChild(const ValueNode& node, _tParams&&... args)
	{
		return Access<_type>(AccessNode(node, lforward(args)...));
	}
	//@}

	//! \brief 访问指定名称的子节点的指定类型对象的指针。
	//@{
	template<typename _type, typename... _tParams>
	inline observer_ptr<_type>
		AccessChildPtr(ValueNode& node, _tParams&&... args) lnothrow
	{
		return AccessPtr<_type>(
			AccessNodePtr(node.GetContainerRef(), lforward(args)...));
	}
	template<typename _type, typename... _tParams>
	inline observer_ptr<const _type>
		AccessChildPtr(const ValueNode& node, _tParams&&... args) lnothrow
	{
		return AccessPtr<_type>(
			AccessNodePtr(node.GetContainer(), lforward(args)...));
	}
	template<typename _type, typename... _tParams>
	inline observer_ptr<_type>
		AccessChildPtr(ValueNode* p_node, _tParams&&... args) lnothrow
	{
		return p_node ? AccessChildPtr<_type>(*p_node, lforward(args)...) : nullptr;
	}
	template<typename _type, typename... _tParams>
	inline observer_ptr<const _type>
		AccessChildPtr(const ValueNode* p_node, _tParams&&... args) lnothrow
	{
		return p_node ? AccessChildPtr<_type>(*p_node, lforward(args)...) : nullptr;
	}
	//@}
	//@}
	//@}


	//! \note 结果不含子节点。
	//@{
	inline PDefH(const ValueNode&, AsNode, const ValueNode& node)
		ImplRet(node)
		/*!
		\brief 传递指定名称和值参数构造值类型节点。
		*/
		template<typename _tString, typename... _tParams>
	inline ValueNode
		AsNode(_tString&& str, _tParams&&... args)
	{
		return{ NoContainer, lforward(str), std::forward<_tParams>(args)... };
	}

	/*!
	\brief 传递指定名称和退化值参数构造值类型节点。
	*/
	template<typename _tString, typename... _tParams>
	inline ValueNode
		MakeNode(_tString&& str, _tParams&&... args)
	{
		return{ NoContainer, lforward(str), leo::decay_copy(args)... };
	}
	//@}

	/*!
	\brief 取指定名称和转换为字符串的值类型节点。
	\note 使用非限定 to_string 转换。
	*/
	template<typename _tString, typename... _tParams>
	inline ValueNode
		StringifyToNode(_tString&& str, _tParams&&... args)
	{
		return{ NoContainer, lforward(str), to_string(lforward(args)...) };
	}

	/*!
	\brief 从引用参数取值类型节点：返回自身。
	*/
	//@{
	inline PDefH(const ValueNode&, UnpackToNode, const ValueNode& arg)
		ImplRet(arg)
		inline PDefH(ValueNode&&, UnpackToNode, ValueNode&& arg)
		ImplRet(std::move(arg))
		//@}


		/*!
		\brief 从参数取以指定分量为初始化参数的值类型节点。
		\note 取分量同 std::get ，但使用 ADL 。仅取前两个分量。
		*/
		template<class _tPack>
	inline ValueNode
		UnpackToNode(_tPack&& pk)
	{
		return{ 0, get<0>(lforward(pk)),
			ValueObject(leo::decay_copy(get<1>(lforward(pk)))) };
	}

	/*!
	\brief 取指定值类型节点为成员的节点容器。
	*/
	//@{
	template<typename _tElem>
	inline ValueNode::Container
		CollectNodes(std::initializer_list<_tElem> il)
	{
		return il;
	}
	template<typename... _tParams>
	inline ValueNode::Container
		CollectNodes(_tParams&&... args)
	{
		return{ lforward(args)... };
	}
	//@}

	/*!
	\brief 取以指定分量为参数对应初始化得到的值类型节点为子节点的值类型节点。
	*/
	template<typename _tString, typename... _tParams>
	inline ValueNode
		PackNodes(_tString&& name, _tParams&&... args)
	{
		return{ CollectNodes(UnpackToNode(lforward(args))...), lforward(name) };
	}


	//@{
	//! \brief 移除空子节点。
	LS_API void
		RemoveEmptyChildren(ValueNode::Container&) lnothrow;

	/*!
	\brief 移除首个子节点。
	\pre 断言：节点非空。
	*/
	//@{
	LS_API void
		RemoveHead(ValueNode::Container&) lnothrowv;
	inline PDefH(void, RemoveHead, ValueNode& term) lnothrowv
		ImplExpr(RemoveHead(term.GetContainerRef()))
		//@}
		//@}


		/*!
		\brief 判断字符串是否是一个指定字符和非负整数的组合。
		\pre 断言：字符串参数的数据指针非空。
		\note 仅测试能被 <tt>unsigned long</tt> 表示的整数。
		*/
		LS_API bool
		IsPrefixedIndex(string_view, char = '$');

	/*!
	\brief 转换节点大小为新的节点索引值。
	\return 保证 4 位十进制数内按字典序递增的字符串。
	\throw std::invalid_argument 索引值过大，不能以 4 位十进制数表示。
	\note 重复使用作为新节点的名称，可用于插入不重复节点。
	*/
	//@{
	LB_API string
		MakeIndex(size_t);
	inline PDefH(string, MakeIndex, const ValueNode::Container& con)
		ImplRet(MakeIndex(con.size()))
		inline PDefH(string, MakeIndex, const ValueNode& node)
		ImplRet(MakeIndex(node.GetContainer()))
	//@}

		template<typename _tParam, typename... _tParams>
	inline ValueNode
		AsIndexNode(_tParam&& arg, _tParams&&... args)
	{
		return AsNode(MakeIndex(lforward(arg)), lforward(args)...);
	}


	/*!
	\brief 节点序列容器。

	除分配器外满足和 std::vector 相同的要求的模板的一个实例，元素为 ValueNode 类型。
	*/
	using NodeSequence = limpl(leo::vector)<ValueNode>;


	/*!
	\brief 包装节点的组合字面量。
	*/
	class LS_API NodeLiteral final
	{
	private:
		ValueNode node;

	public:
		NodeLiteral(const ValueNode& nd)
			: node(nd)
		{}
		NodeLiteral(ValueNode&& nd)
			: node(std::move(nd))
		{}
		NodeLiteral(const string& str)
			: node(NoContainer, str)
		{}
		NodeLiteral(const string& str, string val)
			: node(NoContainer, str, std::move(val))
		{}
		template<typename _tLiteral = NodeLiteral>
		NodeLiteral(const string& str, std::initializer_list<_tLiteral> il)
			: node(NoContainer, str, NodeSequence(il.begin(), il.end()))
		{}
		template<typename _tLiteral = NodeLiteral, class _tString,
			typename... _tParams>
			NodeLiteral(ListContainerTag, _tString&& str,
				std::initializer_list<_tLiteral> il, _tParams&&... args)
			: node(ValueNode::Container(il.begin(), il.end()), lforward(str),
				lforward(args)...)
		{}
		DefDeCopyMoveCtorAssignment(NodeLiteral)

			DefCvt(lnothrow, ValueNode&, node)
			DefCvt(const lnothrow, const ValueNode&, node)
	};

	/*!
	\brief 传递参数构造值类型节点字面量。
	\relates NodeLiteral
	*/
	template<typename _tString, typename _tLiteral = NodeLiteral>
	inline NodeLiteral
		AsNodeLiteral(_tString&& name, std::initializer_list<_tLiteral> il)
	{
		return{ ListContainer, lforward(name), il };
	}
}

#endif