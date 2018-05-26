/*! \file ValueNode.h
\ingroup LFrameWork/Core
\brief ֵ���ͽڵ㡣
\par �޸�ʱ��:
2017-03-24 09:50 +0800
*/
#ifndef LFramework_Core_ValueNode_H
#define LFramework_Core_ValueNode_H 1

#include <LBase/set.hpp>
#include <LBase/path.hpp>
#include <numeric>
#include <LFramework/Core/LObject.h>

namespace leo {
	lconstexpr const struct ListContainerTag {} ListContainer{};

	lconstexpr const struct NoContainerTag {} NoContainer{};

	class LF_API ValueNode : private totally_ordered<ValueNode>,
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
		\brief �ӽڵ�������
		*/
		Container container{};

	public:
		ValueObject Value{};

		DefDeCtor(ValueNode)
			/*!
			\brief ���죺ʹ����������
			*/
			ValueNode(Container con)
			: container(std::move(con))
		{}
		/*!
		\brief ���죺ʹ���ַ������ú�ֵ���Ͷ����������
		\note ��ʹ��������
		*/
		template<typename _tString, typename... _tParams>
		inline
			ValueNode(NoContainerTag, _tString&& str, _tParams&&... args)
			: name(lforward(str)), Value(lforward(args)...)
		{}
		/*!
		\brief ���죺ʹ�����������ַ������ú�ֵ���Ͷ����������
		*/
		template<typename _tString, typename... _tParams>
		ValueNode(Container con, _tString&& str, _tParams&&... args)
			: name(lforward(str)), container(std::move(con)),
			Value(lforward(args)...)
		{}
		/*!
		\brief ���죺ʹ������������ԡ�
		*/
		template<typename _tIn>
		inline
			ValueNode(const pair<_tIn, _tIn>& pr)
			: container(pr.first, pr.second)
		{}
		/*!
		\brief ���죺ʹ������������ԡ��ַ������ú�ֵ������
		*/
		template<typename _tIn, typename _tString>
		inline
			ValueNode(const pair<_tIn, _tIn>& pr, _tString&& str)
			: name(lforward(str)), container(pr.first, pr.second)
		{}
		/*!
		\brief ԭ�ع��죺ʹ�����������ƺ�ֵ�Ĳ���Ԫ�顣
		*/
		//@{
		template<typename... _tParams1>
		inline
			ValueNode(std::tuple<_tParams1...> args1)
			: container(leo::make_from_tuple<Container>(args1))
		{}
		template<typename... _tParams1, typename... _tParams2>
		inline
			ValueNode(std::tuple<_tParams1...> args1, std::tuple<_tParams2...> args2)
			: name(leo::make_from_tuple<string>(args2)),
			container(leo::make_from_tuple<Container>(args1))
		{}
		template<typename... _tParams1, typename... _tParams2,
			typename... _tParams3>
			inline
			ValueNode(std::tuple<_tParams1...> args1, std::tuple<_tParams2...> args2,
				std::tuple<_tParams3...> args3)
			: name(leo::make_from_tuple<string>(args2)),
			container(leo::make_from_tuple<Container>(args1)),
			Value(leo::make_from_tuple<ValueObject>(args3))
		{}
		//@}

		DefDeCopyMoveCtor(ValueNode)

			/*!
			\brief ��һ��ֵ��ʹ��ֵ�����ͽ����������и��ƻ�ת�Ƹ�ֵ��
			*/
			PDefHOp(ValueNode&, =, ValueNode node) lnothrow
			ImplRet(leo::copy_and_swap(*this,node))

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
			\brief �滻ͬ���ӽڵ㡣
			\return �������á�
			*/
			//@{
			PDefHOp(ValueNode&, /=, const ValueNode& node)
			ImplRet(*this %= node, *this)
			PDefHOp(ValueNode&, /=, ValueNode&& node)
			ImplRet(*this %= std::move(node), *this)
			//@}
			/*!
			\brief �滻ͬ���ӽڵ㡣
			\return �ӽڵ����á�
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
		\brief ȡ�ӽڵ��������á�
		*/
		DefGetter(const lnothrow, const Container&, Container, container)
			/*!
			\brief ȡ�ӽڵ��������á�
			*/
			DefGetter(lnothrow, Container&, ContainerRef, container)
			DefGetter(const lnothrow, const string&, Name, name)

			//@{
			//! \brief �����ӽڵ��������ݡ�
			PDefH(void, SetChildren, const Container& con)
			ImplExpr(container = con)
			PDefH(void, SetChildren, Container&& con)
			ImplExpr(container = std::move(con))
			PDefH(void, SetChildren, ValueNode&& node)
			ImplExpr(container = std::move(node.container))
			/*!
			\note �����ӽڵ�������ֵ�����ݡ�
			*/
			//@{
			template<class _tCon, class _tValue>
			limpl(leo::enable_if_t)<
			leo::and_<std::is_assignable<Container, _tCon&&>,
			std::is_assignable<ValueObject, _tValue&&>>::value>
			SetContent(_tCon&& con, _tValue&& val) lnoexcept(leo::and_<
				std::is_nothrow_assignable<Container, _tCon&&>,
				std::is_nothrow_assignable<ValueObject, _tValue&&>>())
			{
				lunseq(container = lforward(con), Value = lforward(val));
			}
			PDefH(void, SetContent, const ValueNode& node)
			ImplExpr(SetContent(node.GetContainer(), node.Value))
			PDefH(void, SetContent, ValueNode&& node)
			ImplExpr(SwapContent(node))
			//@}

			void
			SetContentIndirect(Container, const ValueObject&) lnothrow;
		PDefH(void, SetContentIndirect, const ValueNode& node)
			ImplExpr(SetContentIndirect(node.GetContainer(), node.Value))

			PDefH(bool, Add, const ValueNode& node)
			ImplRet(insert(node).second)
			PDefH(bool, Add, ValueNode&& node)
			ImplRet(insert(std::move(node)).second)

			//@{
			template<typename _tKey>
		bool
			AddChild(_tKey&& k, const ValueNode& node)
		{
			return emplace(node.GetContainer(), lforward(k), node.Value).second;
		}
		template<typename _tKey>
		bool
			AddChild(_tKey&& k, ValueNode&& node)
		{
			return emplace(std::move(node.GetContainerRef()), lforward(k),
				std::move(node.Value)).second;
		}
		template<typename _tKey>
		void
			AddChild(const_iterator hint, _tKey&& k, const ValueNode& node)
		{
			return emplace_hint(hint, node.GetContainer(), lforward(k), node.Value);
		}
		template<typename _tKey>
		void
			AddChild(const_iterator hint, _tKey&& k, ValueNode&& node)
		{
			return emplace_hint(hint, std::move(node.GetContainerRef()),
				lforward(k), std::move(node.Value));
		}
		//@}

			template<typename _tString, typename... _tParams>
		inline bool
			AddValue(_tString&& str, _tParams&&... args)
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

		//! \note �����������޸�ֵ�Ĳ���֮���˳��δָ����
		//@{
		/*!
		\brief ����ڵ㡣
		\post <tt>!Value && empty()</tt> ��
		*/
		PDefH(void, Clear, ) lnothrow
			ImplExpr(Value.Clear(), ClearContainer())

		/*!
		\brief �������������ֵ��
		*/
		PDefH(void, ClearTo, ValueObject vo) lnothrow
		ImplExpr(ClearContainer(), Value = std::move(vo))
		//@}

		/*!
		\brief ����ڵ�������
		\post \c empty() ��
		*/
		PDefH(void, ClearContainer, ) lnothrow
		ImplExpr(container.clear())
		//@}

		/*!
		\brief �ݹ鴴������������
		*/
		//@{
		static Container
		CreateRecursively(const Container&, IValueHolder::Creation);
		template<typename _fCallable>
		static Container
			CreateRecursively(Container& con, _fCallable f)
		{
			Container res;

			for (auto& tm : con)
				res.emplace(CreateRecursively(tm.GetContainerRef(), f),
					tm.GetName(), leo::invoke(f, tm.Value));
			return res;
		}
		template<typename _fCallable>
		static Container
			CreateRecursively(const Container& con, _fCallable f)
		{
			Container res;

			for (auto& tm : con)
				res.emplace(CreateRecursively(tm.GetContainer(), f), tm.GetName(),
					leo::invoke(f, tm.Value));
			return res;
		}

		PDefH(Container, CreateWith, IValueHolder::Creation c) const
		ImplRet(CreateRecursively(container, c))


		template<typename _fCallable>
		Container
			CreateWith(_fCallable f)
		{
			return CreateRecursively(container, f);
		}
		template<typename _fCallable>
		Container
			CreateWith(_fCallable f) const
		{
			return CreateRecursively(container, f);
		}
		//@}

		/*!
		\brief ��ָ�������ӽڵ㲻������ָ��ֵ��ʼ����
		\return ��ָ�����Ʋ��ҵ�ָ�����͵��ӽڵ��ֵ�����á�
		*/
		template<typename _type, typename _tString, typename... _tParams>
		inline _type&
			Place(_tString&& str, _tParams&&... args)
		{
			return this->try_emplace(str, NoContainer, lforward(str),
				leo::in_place<_type>, lforward(args)...).first->Value.template
				GetObject<_type>();
		}

		PDefH(bool, Remove, const ValueNode& node)
			ImplRet(erase(node) != 0)
			PDefH(iterator, Remove, const_iterator i)
			ImplRet(erase(i))

			template<typename _tKey, limpl(typename = leo::enable_if_t<
				leo::is_interoperable<const _tKey&, const string&>::value>)>
		inline bool
			Remove(const _tKey& k)
		{
			return leo::erase_first(container, k);
		}

		/*!
		\brief ���������������ӽڵ㡣
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
		//! \brief ת�������������ӽڵ㡣
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

		//! \warning ���������֮�������Ȩ������ѭ������״̬��������δ������Ϊ��
		//@{
		//! \brief ����������
		PDefH(void, SwapContainer, ValueNode& node) lnothrow
			ImplExpr(container.swap(node.container))

			//! \brief ����������ֵ��
			void
			SwapContent(ValueNode&) lnothrow;
		//@}
		//@}

		/*!
		\brief �׳�����Խ���쳣��
		\throw std::out_of_range ����Խ�硣
		*/
		LB_NORETURN static void
			ThrowIndexOutOfRange();

		/*!
		\brief �׳����ƴ����쳣��
		\throw std::out_of_range ���ƴ���
		*/
		LB_NORETURN static void
			ThrowWrongNameFound();

		//@{
		PDefH(iterator, begin, )
			ImplRet(GetContainerRef().begin())
			PDefH(const_iterator, begin, ) const
			ImplRet(GetContainer().begin())

			DefFwdTmpl(, pair<iterator LPP_Comma bool>, emplace,
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

			DefFwdTmpl(-> decltype(container.erase(lforward(args)...)), auto,
				erase, container.erase(lforward(args)...))

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
			\brief ������
			*/
			LF_API friend void
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
	\brief ���ʽڵ��ָ�����Ͷ���
	\exception std::bad_cast ��ʵ�������ͼ��ʧ�� ��
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
	//! \brief ���ʽڵ��ָ�����Ͷ���ָ�롣
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
	//! \brief ���ʽڵ��ָ�����Ͷ���ָ�롣
	//@{
	template<typename _type, typename _tNodeOrPointer>
	inline auto
		AccessPtr(observer_ptr<_tNodeOrPointer> p) lnothrow
		-> decltype(leo::AccessPtr<_type>(*p))
	{
		return p ? leo::AccessPtr<_type>(*p) : nullptr;
	}
	//@}
	//@}
	//@}

	//@{
	//! \brief ȡָ������ָ�Ƶ�ֵ��
	LF_API ValueObject
		GetValueOf(observer_ptr<const ValueNode>);

	//! \brief ȡָ������ָ�Ƶ�ֵ��ָ�롣
	LF_API observer_ptr<const ValueObject>
		GetValuePtrOf(observer_ptr<const ValueNode>);
	//@}

	//@{
	template<typename _tKey>
	observer_ptr<ValueNode>
		AccessNodePtr(ValueNode::Container*, const _tKey&) lnothrow;
	template<typename _tKey>
	observer_ptr<const ValueNode>
		AccessNodePtr(const ValueNode::Container*, const _tKey&) lnothrow;

	/*!
	\brief ���ʽڵ㡣
	\throw std::out_of_range δ�ҵ���Ӧ�ڵ㡣
	*/
	//@{
	LF_API ValueNode&
		AccessNode(ValueNode::Container*, const string&);
	LF_API const ValueNode&
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
	\note ʱ�临�Ӷ� O(n) ��
	*/
	//@{
	LF_API ValueNode&
		AccessNode(ValueNode&, size_t);
	LF_API const ValueNode&
		AccessNode(const ValueNode&, size_t);
	//@}
	template<typename _tKey, limpl(typename = leo::enable_if_t<
		or_<std::is_constructible<const _tKey&, const string&>,
		std::is_constructible<const string&, const _tKey&>>::value>)>
		inline ValueNode&
		AccessNode(ValueNode& node, const _tKey& name)
	{
		return AccessNode(node.GetContainerRef(), name);
	}
	template<typename _tKey, limpl(typename = leo::enable_if_t<
		or_<std::is_constructible<const _tKey&, const string&>,
		std::is_constructible<const string&, const _tKey&>>::value>)>
		inline const ValueNode&
		AccessNode(const ValueNode& node, const _tKey& name)
	{
		return AccessNode(node.GetContainer(), name);
	}
	//@{
	//! \note ʹ�� ADL \c AccessNode ��
	template<class _tNode, typename _tIn>
	_tNode&&
		AccessNode(_tNode&& node, _tIn first, _tIn last)
	{
		return std::accumulate(first, last, ref(node),
			[](_tNode&& nd, decltype(*first) c) {
			return ref(AccessNode(nd, c));
		});
	}
	//! \note ʹ�� ADL \c begin �� \c end ָ����Χ��������
	template<class _tNode, typename _tRange,
		limpl(typename = leo::enable_if_t<
			!std::is_constructible<const string&, const _tRange&>::value>)>
		inline auto
		AccessNode(_tNode&& node, const _tRange& c)
		-> decltype(AccessNode(lforward(node), begin(c), end(c)))
	{
		return AccessNode(lforward(node), begin(c), end(c));
	}
	//@}
	//@}

	//! \brief ���ʽڵ�ָ�롣
	//@{
	LF_API observer_ptr<ValueNode>
		AccessNodePtr(ValueNode::Container&, const string&) lnothrow;
	LF_API observer_ptr<const ValueNode>
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
	\note ʱ�临�Ӷ� O(n) ��
	*/
	//@{
	LF_API observer_ptr<ValueNode>
		AccessNodePtr(ValueNode&, size_t);
	LF_API observer_ptr<const ValueNode>
		AccessNodePtr(const ValueNode&, size_t);
	//@}
	template<typename _tKey, limpl(typename = leo::enable_if_t<
		or_<std::is_constructible<const _tKey&, const string&>,
		std::is_constructible<const string&, const _tKey&>>::value>)>
		inline observer_ptr<ValueNode>
		AccessNodePtr(ValueNode& node, const _tKey& name)
	{
		return AccessNodePtr(node.GetContainerRef(), name);
	}
	template<typename _tKey, limpl(typename = leo::enable_if_t<
		or_<std::is_constructible<const _tKey&, const string&>,
		std::is_constructible<const string&, const _tKey&>>::value>)>
		inline observer_ptr<const ValueNode>
		AccessNodePtr(const ValueNode& node, const _tKey& name)
	{
		return AccessNodePtr(node.GetContainer(), name);
	}
	//@{
	//! \note ʹ�� ADL \c AccessNodePtr ��
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
	//! \note ʹ�� ADL \c begin �� \c end ָ����Χ��������
	template<class _tNode, typename _tRange,
		limpl(typename = leo::enable_if_t<
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
	\exception std::bad_cast ��ʵ�������ͼ��ʧ�� ��
	\relates ValueNode
	*/
	//@{
	/*!
	\brief �����ӽڵ��ָ�����Ͷ���
	\note ʹ�� ADL \c AccessNode ��
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

	//! \brief ����ָ�����Ƶ��ӽڵ��ָ�����Ͷ����ָ�롣
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


	//! \note ��������ӽڵ㡣
	//@{
	inline PDefH(const ValueNode&, AsNode, const ValueNode& node)
		ImplRet(node)
		/*!
		\brief ����ָ�����ƺ�ֵ��������ֵ���ͽڵ㡣
		*/
		template<typename _tString, typename... _tParams>
	inline ValueNode
		AsNode(_tString&& str, _tParams&&... args)
	{
		return{ NoContainer, lforward(str), std::forward<_tParams>(args)... };
	}

	/*!
	\brief ����ָ�����ƺ��˻�ֵ��������ֵ���ͽڵ㡣
	*/
	template<typename _tString, typename... _tParams>
	inline ValueNode
		MakeNode(_tString&& str, _tParams&&... args)
	{
		return{ NoContainer, lforward(str), leo::decay_copy(args)... };
	}
	//@}

	/*!
	\brief ȡָ�����ƺ�ת��Ϊ�ַ�����ֵ���ͽڵ㡣
	\note ʹ�÷��޶� to_string ת����
	*/
	template<typename _tString, typename... _tParams>
	inline ValueNode
		StringifyToNode(_tString&& str, _tParams&&... args)
	{
		return{ NoContainer, lforward(str), to_string(lforward(args)...) };
	}

	/*!
	\brief �����ò���ȡֵ���ͽڵ㣺��������
	*/
	//@{
	inline PDefH(const ValueNode&, UnpackToNode, const ValueNode& arg)
		ImplRet(arg)
		inline PDefH(ValueNode&&, UnpackToNode, ValueNode&& arg)
		ImplRet(std::move(arg))
		//@}


		/*!
		\brief �Ӳ���ȡ��ָ������Ϊ��ʼ��������ֵ���ͽڵ㡣
		\note ȡ����ͬ std::get ����ʹ�� ADL ����ȡǰ����������
		*/
		template<class _tPack>
	inline ValueNode
		UnpackToNode(_tPack&& pk)
	{
		return{ 0, get<0>(lforward(pk)),
			ValueObject(leo::decay_copy(get<1>(lforward(pk)))) };
	}

	/*!
	\brief ȡָ��ֵ���ͽڵ�Ϊ��Ա�Ľڵ�������
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
	\brief ȡ��ָ������Ϊ������Ӧ��ʼ���õ���ֵ���ͽڵ�Ϊ�ӽڵ��ֵ���ͽڵ㡣
	*/
	template<typename _tString, typename... _tParams>
	inline ValueNode
		PackNodes(_tString&& name, _tParams&&... args)
	{
		return{ CollectNodes(UnpackToNode(lforward(args))...), lforward(name) };
	}


	//@{
	//! \brief �Ƴ����ӽڵ㡣
	LF_API void
		RemoveEmptyChildren(ValueNode::Container&) lnothrow;

	/*!
	\brief �Ƴ��׸��ӽڵ㡣
	\pre ���ԣ��ڵ�ǿա�
	*/
	//@{
	LF_API void
		RemoveHead(ValueNode::Container&) lnothrowv;
	inline PDefH(void, RemoveHead, ValueNode& term) lnothrowv
		ImplExpr(RemoveHead(term.GetContainerRef()))
		//@}
		//@}

	template<typename _tNode, typename _fCallable>
	void
		SetContentWith(ValueNode& dst, _tNode& node, _fCallable f)
	{
		lunseq(
			dst.Value = leo::invoke(f, node.Value),
			dst.GetContainerRef() = node.CreateWith(f)
		);
	}

	/*!
	\brief �ж��ַ����Ƿ���һ��ָ���ַ��ͷǸ���������ϡ�
	\pre ���ԣ��ַ�������������ָ��ǿա�
	\note �������ܱ� <tt>unsigned long</tt> ��ʾ��������
	*/
	LF_API bool
	IsPrefixedIndex(string_view, char = '$');

	/*!
	\brief ת���ڵ��СΪ�µĽڵ�����ֵ��
	\return ��֤ 4 λʮ�������ڰ��ֵ���������ַ�����
	\throw std::invalid_argument ����ֵ���󣬲����� 4 λʮ��������ʾ��
	\note �ظ�ʹ����Ϊ�½ڵ�����ƣ������ڲ��벻�ظ��ڵ㡣
	*/
	//@{
	LF_API string
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
	\brief �ڵ�����������

	��������������� std::vector ��ͬ��Ҫ���ģ���һ��ʵ����Ԫ��Ϊ ValueNode ���͡�
	*/
	using NodeSequence = limpl(leo::vector)<ValueNode>;


	/*!
	\brief ��װ�ڵ�������������
	*/
	class LF_API NodeLiteral final
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
	\brief ���ݲ�������ֵ���ͽڵ���������
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