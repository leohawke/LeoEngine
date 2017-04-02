/*\par 修改时间 :
	2016-11-13 22:15 + 0800
*/

#ifndef LScheme_LSchemeA_H
#define LScheme_LSchemeA_H 1


#include "SContext.h"
#include "LEvent.hpp"
#include <LBase/any.h>

namespace scheme {
		using leo::pair;
		using leo::to_string;
		using leo::MakeIndex;
		using leo::NodeSequence;
		using leo::NodeLiteral;


		/*!
		\brief LA 元标签。
		\note LA 是 LScheme 的抽象实现。
		*/
		struct LS_API LSLATag : LTag
		{};


		/*!
		\brief 插入语法节点。

		在指定的节点插入以节点大小字符串为名称的节点，可用于语法分析树。
		*/
		//@{
		template<class _tNodeOrCon, typename... _tParams>
		ValueNode::iterator
			InsertSyntaxNode(_tNodeOrCon&& node_or_con,
				std::initializer_list<ValueNode> il, _tParams&&... args)
		{
			return node_or_con.emplace_hint(node_or_con.end(), ValueNode::Container(il),
				MakeIndex(node_or_con), lforward(args)...);
		}
		template<class _tNodeOrCon, typename _type, typename... _tParams>
		ValueNode::iterator
			InsertSyntaxNode(_tNodeOrCon&& node_or_con, _type&& arg, _tParams&&... args)
		{
			return node_or_con.emplace_hint(node_or_con.end(), lforward(arg),
				MakeIndex(node_or_con), lforward(args)...);
		}
		//@}


		/*!
		\brief 节点映射操作类型：变换节点为另一个节点。
		*/
		using NodeMapper = std::function<ValueNode(const TermNode&)>;

		//@{
		using NodeToString = std::function<string(const ValueNode&)>;

		template<class _tCon>
		using GNodeInserter = std::function<void(TermNode&&, _tCon&)>;

		using NodeInserter = GNodeInserter<TermNode::Container&>;

		using NodeSequenceInserter = GNodeInserter<NodeSequence>;
		//@}


		//! \return 创建的新节点。
		//@{
		/*!
		\brief 映射 LSLA 叶节点。
		\sa ParseLSLANodeString

		创建新节点。若参数为空则返回值为空串的新节点；否则值以 ParseLSLANodeString 取得。
		*/
		LS_API ValueNode
			MapLSLALeafNode(const TermNode&);

		/*!
		\brief 变换节点为语法分析树叶节点。
		\note 可选参数指定结果名称。
		*/
		LS_API ValueNode
			TransformToSyntaxNode(const ValueNode&, const string& = {});
		//@}

		/*!
		\brief 转义 LSLA 节点字面量。
		\return 调用 EscapeLiteral 转义访问字符串的结果。
		\exception leo::bad_any_cast 异常中立：由 Access 抛出。
		*/
		LS_API string
			EscapeNodeLiteral(const ValueNode&);

		/*!
		\brief 转义 LSLA 节点字面量。
		\return 参数为空节点则空串，否则调用 Literalize 字面量化 EscapeNodeLiteral 的结果。
		\exception leo::bad_any_cast 异常中立：由 EscapeNodeLiteral 抛出。
		\sa EscapeNodeLiteral
		*/
		LS_API string
			LiteralizeEscapeNodeLiteral(const ValueNode&);

		/*!
		\brief 解析 LSLA 节点字符串。

		以 string 类型访问节点，若失败则结果为空串。
		*/
		LS_API string
			ParseLSLANodeString(const ValueNode&);


		/*!
		\brief 插入语法子节点。

		在指定的节点插入以节点大小字符串为名称的节点，可用于语法分析树。
		*/
		//@{
		template<class _tNodeOrCon>
		ValueNode::iterator
			InsertChildSyntaxNode(_tNodeOrCon&& node_or_con, ValueNode& child)
		{
			return InsertSyntaxNode(lforward(node_or_con), child.GetContainerRef());
		}
		template<class _tNodeOrCon>
		ValueNode::iterator
			InsertChildSyntaxNode(_tNodeOrCon&& node_or_con, ValueNode&& child)
		{
			return InsertSyntaxNode(lforward(node_or_con),
				std::move(child.GetContainerRef()));
		}
		template<class _tNodeOrCon>
		ValueNode::iterator
			InsertChildSyntaxNode(_tNodeOrCon&& node_or_con, const NodeLiteral& nl)
		{
			return
				InsertChildSyntaxNode(lforward(node_or_con), TransformToSyntaxNode(nl));
		}
		//@}


		//! \brief 生成前缀缩进的函数类型。
		using IndentGenerator = std::function<string(size_t)>;

		//! \brief 生成水平制表符为单位的缩进。
		LS_API string
			DefaultGenerateIndent(size_t);

		/*!
		\brief 打印缩进。
		\note 若最后一个参数等于零则无副作用。
		*/
		LS_API void
			PrintIndent(std::ostream&, IndentGenerator = DefaultGenerateIndent, size_t = 1);

		/*!
		\brief 打印 LSLA 节点。
		\sa PrintIdent
		\sa PrintNodeChild
		\sa PrintNodeString

		调用第四参数输出最后一个参数决定的缩进作为前缀和一个空格，然后打印节点内容：
		先尝试调用 PrintNodeString 打印节点字符串，若成功直接返回；
		否则打印换行，然后尝试调用 PrintNodeChild 打印 NodeSequence ；
		再次失败则调用 PrintNodeChild 打印子节点。
		调用 PrintNodeChild 打印后输出回车。
		*/
		LS_API void
			PrintNode(std::ostream&, const ValueNode&, NodeToString = EscapeNodeLiteral,
				IndentGenerator = DefaultGenerateIndent, size_t = 0);

		/*!
		\brief 打印作为子节点的 LSLA 节点。
		\sa IsPrefixedIndex
		\sa PrintIdent
		\sa PrintNodeString

		调用第四参数输出最后一个参数决定的缩进作为前缀，然后打印子节点内容。
		对满足 IsPrefixedIndex 的节点调用 PrintNodeString 作为节点字符串打印；
		否则，调用 PrintNode 递归打印子节点，忽略此过程中的 std::out_of_range 异常。
		*/
		LS_API void
			PrintNodeChild(std::ostream&, const ValueNode&, NodeToString
				= EscapeNodeLiteral, IndentGenerator = DefaultGenerateIndent, size_t = 0);

		/*!
		\brief 打印节点字符串。
		\return 是否成功访问节点字符串并输出。
		\note leo::bad_any_cast 外异常中立。
		\sa PrintNode

		使用最后一个参数指定的访问节点，打印得到的字符串和换行符。
		忽略 leo::bad_any_cast 。
		*/
		LS_API bool
			PrintNodeString(std::ostream&, const ValueNode&,
				NodeToString = EscapeNodeLiteral);


		namespace sxml
		{

			//@{
			/*!
			\brief 转换 sxml 节点为 XML 属性字符串。
			\throw LoggedEvent 没有子节点。
			\note 当前不支持 annotation ，在超过 2 个子节点时使用 TraceDe 警告。
			*/
			LS_API string
				ConvertAttributeNodeString(const TermNode&);

			/*!
			\brief 转换 sxml 文档节点为 XML 字符串。
			\throw LoggedEvent 不符合最后一个参数约定的内容被解析。
			\throw leo::unimplemented 指定 ParseOption::Strict 时解析未实现内容。
			\sa ConvertStringNode
			\see http://okmij.org/ftp/Scheme/sxml.html#Annotations 。
			\todo 支持 *ENTITY* 和 *NAMESPACES* 标签。

			转换 sxml 文档节点为 XML 。
			尝试使用 ConvertStringNode 转换字符串节点，若失败作为非叶子节点递归转换。
			因为当前 sxml 规范未指定注解(annotation) ，所以直接忽略。
			*/
			LS_API string
				ConvertDocumentNode(const TermNode&, IndentGenerator = DefaultGenerateIndent,
					size_t = 0, ParseOption = ParseOption::Normal);

			/*!
			\brief 转换 sxml 节点为被转义的 XML 字符串。
			\sa EscapeXML
			*/
			LS_API string
				ConvertStringNode(const TermNode&);

			/*!
			\brief 打印 SContext::Analyze 分析取得的 sxml 语法树节点并刷新流。
			\see ConvertDocumentNode
			\see SContext::Analyze
			\see Session

			参数节点中取第一个节点作为 sxml 文档节点调用 ConvertStringNode 输出并刷新流。
			*/
			LS_API void
				PrintSyntaxNode(std::ostream& os, const TermNode&,
					IndentGenerator = DefaultGenerateIndent, size_t = 0);
			//@}


			//@{
			//! \brief 构造 sxml 文档顶级节点。
			//@{
			template<typename... _tParams>
			ValueNode
				MakeTop(const string& name, _tParams&&... args)
			{
				return leo::AsNodeLiteral(name,
				{ { MakeIndex(0), "*TOP*" }, NodeLiteral(lforward(args))... });
			}
			inline PDefH(ValueNode, MakeTop, )
				ImplRet(MakeTop({}))
				//@}

				/*!
				\brief 构造 sxml 文档 XML 声明节点。
				\note 第一个参数指定节点名称，其余参数指定节点中 XML 声明的值：版本、编码和独立性。
				\note 最后两个参数可选为空值，此时结果不包括对应的属性。
				\warning 不对参数合规性进行检查。
				*/
				LS_API ValueNode
				MakeXMLDecl(const string& = {}, const string& = "1.0",
					const string& = "UTF-8", const string& = {});

			/*!
			\brief 构造包含 XML 声明的 sxml 文档节点。
			\sa MakeTop
			\sa MakeXMLDecl
			*/
			LS_API ValueNode
				MakeXMLDoc(const string& = {}, const string& = "1.0",
					const string& = "UTF-8", const string& = {});

			//! \brief 构造 sxml 属性标记字面量。
			//@{
			inline PDefH(NodeLiteral, MakeAttributeTagLiteral,
				std::initializer_list<NodeLiteral> il)
				ImplRet({ "@", il })
				template<typename... _tParams>
			NodeLiteral
				MakeAttributeTagLiteral(_tParams&&... args)
			{
				return sxml::MakeAttributeTagLiteral({ NodeLiteral(lforward(args)...) });
			}
			//@}

			//! \brief 构造 XML 属性字面量。
			//@{
			inline PDefH(NodeLiteral, MakeAttributeLiteral, const string& name,
				std::initializer_list<NodeLiteral> il)
				ImplRet({ name,{ MakeAttributeTagLiteral(il) } })
				template<typename... _tParams>
			NodeLiteral
				MakeAttributeLiteral(const string& name, _tParams&&... args)
			{
				return{ name,{ sxml::MakeAttributeTagLiteral(lforward(args)...) } };
			}
			//@}

			//! \brief 插入只有 XML 属性节点到语法节点。
			//@{
			template<class _tNodeOrCon>
			inline void
				InsertAttributeNode(_tNodeOrCon&& node_or_con, const string& name,
					std::initializer_list<NodeLiteral> il)
			{
				InsertChildSyntaxNode(node_or_con, MakeAttributeLiteral(name, il));
			}
			template<class _tNodeOrCon, typename... _tParams>
			inline void
				InsertAttributeNode(_tNodeOrCon&& node_or_con, const string& name,
					_tParams&&... args)
			{
				InsertChildSyntaxNode(node_or_con,
					sxml::MakeAttributeLiteral(name, lforward(args)...));
			}
			//@}
			//@}

		} // namespace sxml;
		  //@}


		  //@{
		  //! \ingroup exceptions
		  //@{
		  //! \brief LSL 异常基类。
		class LS_API LSLException : public LoggedEvent
		{
		public:
			LB_NONNULL(2)
				LSLException(const char* str, leo::RecordLevel lv = leo::Err)
				: LoggedEvent(str, lv)
			{}
			LSLException(const leo::string_view sv,
				leo::RecordLevel lv = leo::Err)
				: LoggedEvent(sv, lv)
			{}
			DefDeCtor(LSLException)

				//! \brief 虚析构：类定义外默认实现。
			virtual	~LSLException() override;
		};


		/*!
		\brief 列表规约失败。
		\todo 捕获并保存上下文信息。
		*/
		class LS_API ListReductionFailure : public LSLException
		{
		public:
			using LSLException::LSLException;
			DefDeCtor(ListReductionFailure)

				//! \brief 虚析构：类定义外默认实现。
				~ListReductionFailure() override;
		};


		//! \brief 语法错误。
		class LS_API InvalidSyntax : public LSLException
		{
		public:
			using LSLException::LSLException;
			DefDeCtor(InvalidSyntax)

				//! \brief 虚析构：类定义外默认实现。
				~InvalidSyntax() override;
		};


		/*!
		\brief 标识符错误。
		*/
		class LS_API BadIdentifier : public LSLException
		{
		private:
			leo::shared_ptr<string> p_identifier;

		public:
			/*!
			\brief 构造：使用作为标识符的字符串、已知实例数和和记录等级。
			\pre 间接断言：第一参数的数据指针非空。

			不检查第一参数内容作为标识符的合法性，直接视为待处理的标识符，
			初始化表示标识符错误的异常对象。
			实例数等于 0 时表示未知标识符；
			实例数等于 1 时表示非法标识符；
			实例数大于 1 时表示重复标识符。
			*/
			LB_NONNULL(2)
				BadIdentifier(const char*, size_t = 0, leo::RecordLevel = leo::Err);
			BadIdentifier(string_view, size_t = 0, leo::RecordLevel = leo::Err);
			DefDeCtor(BadIdentifier)

				//! \brief 虚析构：类定义外默认实现。
				~BadIdentifier() override;

			DefGetter(const lnothrow, const string&, Identifier,
				leo::Deref(p_identifier))
		};


		/*!
		\brief 元数不匹配错误。
		\todo 支持范围匹配。
		*/
		class LS_API ArityMismatch : public LSLException
		{
		private:
			size_t expected;
			size_t received;

		public:
			DefDeCtor(ArityMismatch)
				/*!
				\note 前两个参数表示期望和实际的元数。
				*/
				ArityMismatch(size_t, size_t, leo::RecordLevel = leo::Err);

			//! \brief 虚析构：类定义外默认实现。
			~ArityMismatch() override;

			DefGetter(const lnothrow, size_t, Expected, expected)
				DefGetter(const lnothrow, size_t, Received, received)
		};
		//@}
		//@}


		/*!
		\brief 规约状态：一遍规约可能的中间结果。
		*/
		enum class ReductionStatus : limpl(size_t)
		{
			Success = 0,
				NeedRetry
		};


		//@{
		//! \brief 上下文节点类型。
		using ContextNode = ValueNode;

		using leo::AccessChildPtr;

		//! \brief 上下文处理器类型。
		using ContextHandler = leo::GHEvent<void(TermNode&, ContextNode&)>;
		//! \brief 字面量处理器类型。
		using LiteralHandler = leo::GHEvent<bool(const ContextNode&)>;

		//! \brief 注册上下文处理器。
		inline PDefH(void, RegisterContextHandler, ContextNode& node,
			const string& name, ContextHandler f)
			ImplExpr(node[name].Value = std::move(f))

			//! \brief 注册字面量处理器。
			inline PDefH(void, RegisterLiteralHandler, ContextNode& node,
				const string& name, LiteralHandler f)
			ImplExpr(node[name].Value = std::move(f))
			//@}


			//@{
			//! \brief 字面量类别。
			enum class LiteralCategory
		{
			//! \brief 无：非字面量。
			None,
			//! \brief 代码字面量。
			Code,
			//! \brief 数据字面量。
			Data,
			/*!
			\brief 扩展字面量：由派生实现定义的其它字面量。
			*/
			Extended
		};


		/*!
		\brief 对字面量分类。
		\pre 断言：字符串参数的数据指针非空。
		\return 判断的非扩展字面量分类。
		\note 扩展字面量视为非字面量。
		\sa LiteralCategory
		*/
		LS_API LiteralCategory
			CategorizeLiteral(string_view);
		//@}


		//@{
		//! \brief 从指定上下文查找名称对应的节点。
		template<typename _tKey>
		inline observer_ptr<ValueNode>
			LookupName(ContextNode& ctx, const _tKey& id) lnothrow
		{
			return leo::AccessNodePtr(ctx, id);
		}

		template<typename _tKey>
		inline observer_ptr<const ValueNode>
			LookupName(const ContextNode& ctx, const _tKey& id) lnothrow
		{
			return leo::AccessNodePtr(ctx, id);
		}

		//! \brief 从指定上下文取指定名称指称的值。
		template<typename _tKey>
		ValueObject
			FetchValue(const ContextNode& ctx, const _tKey& name)
		{
			return leo::call_value_or(
				std::mem_fn(&ValueNode::Value),scheme::LookupName(ctx, name));
		}

		template<typename _tKey>
		static observer_ptr<const ValueObject>
			FetchValuePtr(const ContextNode& ctx, const _tKey& name)
		{
			return leo::call_value_or(
				[](const ValueNode& node) {
				return make_observer(&node.Value);
			}, scheme::LookupName(ctx, name));
		}
		//@}

		/*!
		\brief 访问项的值作为名称。
		\return 通过访问项的值取得的名称，或空指针表示无法取得名称。
		*/
		LS_API observer_ptr<const string>
			TermToName(const TermNode&);

		/*!
		\pre 字符串参数的数据指针非空。
		\note 最后一个参数表示强制调用。
		\throw BadIdentifier 非强制调用时发现标识符不存在或冲突。
		*/
		//@{
		//! \brief 以字符串为标识符在指定上下文中定义值。
		LS_API void
			DefineValue(ContextNode&, string_view, ValueObject&&, bool);

		/*!
		\brief 以字符串为标识符在指定上下文中覆盖定义值。
		*/
		LS_API void
			RedefineValue(ContextNode&, string_view, ValueObject&&, bool);

		//! \brief 以字符串为标识符在指定上下文移除对象。
		LS_API void
			RemoveIdentifier(ContextNode&, string_view, bool);
		//@}


		/*!
		\brief 根据规约状态检查是否可继续规约。
		\see TraceDe

		只根据输入状态确定结果。当且仅当规约成功时不视为继续规约。
		若发现不支持的状态视为不成功，输出警告。
		*/
		LB_PURE LS_API bool
			CheckReducible(ReductionStatus);

		/*!
		\sa CheckReducible
		*/
		template<typename _func, typename... _tParams>
		void
			CheckedReduceWith(_func f, _tParams&&... args)
		{
			leo::retry_on_cond(CheckReducible, f, lforward(args)...);
		}

		/*!
		\brief 移除节点的空子节点，然后判断是否可继续规约。
		\return 可继续规约：第一参数指定非成功状态，且移除空子节点后的节点是枝节点。
		\see CheckReducible
		*/
		LS_API bool
			DetectReducible(ReductionStatus, TermNode&);


		//@{
		/*!
		\brief 遍合并器：逐次调用直至成功。
		\note 合并遍结果用于表示及早判断是否应继续规约，可在循环中实现再次规约一个项。
		*/
		struct PassesCombiner
		{
			/*!
			\note 对遍调用异常中立。
			*/
			template<typename _tIn>
			bool
				operator()(_tIn first, _tIn last) const
			{
				return leo::fast_any_of(first, last, leo::id<>());
			}
		};


		/*!
		\note 结果表示判断是否应继续规约。
		\sa PassesCombiner
		*/
		//@{
		//! \brief 一般合并遍。
		template<typename... _tParams>
		using GPasses = leo::GEvent<bool(_tParams...),
			leo::GCombinerInvoker<bool, PassesCombiner>>;
		//! \brief 项合并遍。
		using TermPasses = GPasses<TermNode&>;
		//! \brief 求值合并遍。
		using EvaluationPasses = GPasses<TermNode&, ContextNode&>;
		/*!
		\brief 字面量合并遍。
		\pre 字符串参数的数据指针非空。
		*/
		using LiteralPasses = GPasses<TermNode&, ContextNode&, string_view>;
		//@}


		//! \brief 作用域守护类型。
		using Guard = leo::any;
		/*!
		\brief 作用域守护遍：用于需在规约例程的入口和出口关联执行的操作。
		\todo 支持迭代使用旧值。
		*/
		using GuardPasses = leo::GEvent<Guard(TermNode&, ContextNode&),
			leo::GDefaultLastValueInvoker<Guard>>;


		//! \brief 使用第二个参数指定的项的内容替换第一个项的内容。
		inline PDefH(void, LiftTerm, TermNode& term, TermNode& tm)
			ImplExpr(term.SetContent(std::move(tm)))
			//@}

			//@{
			inline PDefH(void, LiftTerm, ValueObject& term_v, ValueObject& vo)
			ImplExpr(term_v = std::move(vo))
			inline PDefH(void, LiftTerm, TermNode& term, ValueObject& vo)
			ImplExpr(LiftTerm(term.Value, vo))
			//@}

			//@{
			inline PDefH(void, LiftTermRef, TermNode& term, TermNode& tm)
			ImplExpr(term.SetContentIndirect(tm))
			inline PDefH(void, LiftTermRef, ValueObject& term_v, const ValueObject& vo)
			ImplExpr(term_v = vo.MakeIndirect())
			inline PDefH(void, LiftTermRef, TermNode& term, const ValueObject& vo)
			ImplExpr(LiftTermRef(term.Value, vo))

			//! \pre 断言：参数指定的项是枝节点。
			//@{
			/*!
			\brief 使用首个子项替换项的内容。
			*/
			inline PDefH(void, LiftFirst, TermNode& term)
			ImplExpr(IsBranch(term), LiftTerm(term, Deref(term.begin())))

			/*!
			\brief 使用最后一个子项替换项的内容。
			*/
			inline PDefH(void, LiftLast, TermNode& term)
			ImplExpr(IsBranch(term), LiftTerm(term, Deref(term.rbegin())))
			//@}


			/*!
			\brief 调用处理遍：从指定名称的节点中访问指定类型的遍并以指定上下文调用。
			*/
			template<class _tPasses, typename... _tParams>
		typename _tPasses::result_type
			InvokePasses(const string& name, TermNode& term, ContextNode& ctx,
				_tParams&&... args)
		{
			return leo::call_value_or(
				[&](_tPasses& passes) {
				// XXX: Blocked. 'lforward' cause G++ 5.3 crash: internal compiler
				//	error: Segmentation fault.
				return passes(term, ctx, std::forward<_tParams>(args)...);
			}, AccessChildPtr<_tPasses>(ctx, name));
		}

} // namespace scheme;

#endif