/*\par 修改时间 :
	2017-03-27 15:14 +0800
*/

#ifndef LScheme_LSchemeA_H
#define LScheme_LSchemeA_H 1


#include "SContext.h" 
#include <LBase/sutility.h> // for leo::derived_entity
#include "LEvent.hpp"
#include <LBase/any.h> // for leo::any

namespace scheme {

	/*!	\defgroup ThunkType Thunk Types
	\brief 中间值类型。

	标记特定求值策略，储存于 TermNode 的 Value 数据成员中不直接表示宿主语言对象的类型。
	*/


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

		class LS_API ParameterMismatch : public InvalidSyntax
		{
		public:
			using InvalidSyntax::InvalidSyntax;
			DefDeCtor(ParameterMismatch)

				//! \brief 虚析构：类定义外默认实现。
				~ParameterMismatch() override;
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


		
		//@}
		//@}


		/*!
		\brief 规约状态：一遍规约可能的中间结果。
		*/
		enum class ReductionStatus : limpl(size_t)
		{
			//@{
			//! \brief 规约成功终止且不需要保留子项。
			Clean = 0,
				//! \brief 规约成功但需要保留子项。
				Retained,
				//! \brief 需要重规约。
				Retrying
				//@}
		};


		/*!
		\ingroup ThunkType
		\brief 延迟求值项。
		\note 和被延迟求值的项及其它节点是不同的包装类型。
		\warning 非空析构。

		直接作为项的值对象包装被延迟求值的项。
		*/
		using DelayedTerm = leo::derived_entity<TermNode, LSLATag>;

		//@{
		//! \brief 上下文节点类型。
		using ContextNode = limpl(ValueNode);

		using leo::AccessChildPtr;

		//! \brief 上下文处理器类型。
		using ContextHandler = leo::GHEvent<ReductionStatus(TermNode&, ContextNode&)>;
		//! \brief 字面量处理器类型。
		using LiteralHandler = leo::GHEvent<ReductionStatus(const ContextNode&)>;

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
			enum class LexemeCategory
		{
			//! \brief 无：非字面量。
			Symbol,
			//! \brief 代码字面量。
			Code,
			//! \brief 数据字面量。
			Data,
			/*!
			\brief 扩展字面量：由派生实现定义的其它字面量。
			*/
			Extended
		};



		//! \sa LexemeCategory
		//@{
		/*!
		\pre 断言：字符串参数的数据指针非空且字符串非空。
		\return 判断的非扩展字面量分类。
		*/
		//@{
		/*!
		\brief 对排除扩展字面量的词素分类。
		\note 扩展字面量视为非字面量。
		*/
		LS_API LexemeCategory
			CategorizeBasicLexeme(string_view) lnothrowv;

		/*!
		\brief 对词素分类。
		\sa CategorizeBasicLexeme
		*/
		LS_API LexemeCategory
			CategorizeLexeme(string_view) lnothrowv;
		//@}

		/*!
		\brief 判断不是非扩展字面量的词素是否为 LSLA 扩展字面量。
		\pre 断言：字符串参数的数据指针非空且字符串非空。
		\pre 词素不是代码字面量或数据字面量。
		*/
		LS_API bool
			IsLSLAExtendedLiteral(string_view) lnothrowv;

		/*!
		\brief 判断字符是否为 LSLA 扩展字面量非数字前缀。
		*/
		lconstfn PDefH(bool, IsLSLAExtendedLiteralNonDigitPrefix, char c) lnothrow
			ImplRet(c == '#' || c == '+' || c == '-')

			//! \brief 判断字符是否为 LSLA 扩展字面量前缀。
			inline PDefH(bool, IsLSLAExtendedLiteralPrefix, char c) lnothrow
			ImplRet(std::isdigit(c) || IsLSLAExtendedLiteralNonDigitPrefix(c))

			/*!
			\brief 判断词素是否为 LSLA 符号。
			\pre 间接断言：字符串参数的数据指针非空且字符串非空。
			*/
			inline PDefH(bool, IsLSLASymbol, string_view id) lnothrowv
			ImplRet(CategorizeLexeme(id) == LexemeCategory::Symbol)
			//@}
			//@}


			/*!
			\ingroup ThunkType
			\brief 记号值。
			\note 和被求值的字符串不同的包装类型。
			\warning 非空析构。
			*/
			using TokenValue = leo::derived_entity<string, LSLATag>;


		/*!
		\brief 标记记号节点：递归变换节点，转换其中的词素为记号值。
		\note 先变换子节点。
		*/
		LS_API void
			TokenizeTerm(TermNode& term);


		/*!
		\brief 对表示值的 ValueObject 进行基于所有权的生存期检查并取表示其引用的间接值。
		\throw LSLException 检查失败：参数具有对象的唯一所有权，不能被外部引用保存。
		\throw leo::invalid_construction 参数不持有值。
		\todo 使用具体的语义错误异常类型。
		*/
		LS_API ValueObject
			ReferenceValue(const ValueObject&);

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
			return GetValueOf(scheme::LookupName(ctx, name));
		}

		template<typename _tKey>
		static observer_ptr<const ValueObject>
			FetchValuePtr(const ContextNode& ctx, const _tKey& name)
		{
			return GetValuePtrOf(scheme::LookupName(ctx, name));
		}
		//@}

		/*!
		\brief 访问项的值作为名称。
		\return 通过访问项的值取得的名称，或空指针表示无法取得名称。
		*/
		LS_API observer_ptr<const string>
			TermToNamePtr(const TermNode&);

		/*!
		//@{
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

		inline PDefH(ReductionStatus, CheckNorm, const TermNode& term) lnothrow
			ImplRet(IsBranch(term) ? ReductionStatus::Retained : ReductionStatus::Clean)

		/*!
		\sa CheckReducible
		*/
		template<typename _func, typename... _tParams>
		void
			CheckedReduceWith(_func f, _tParams&&... args)
		{
			leo::retry_on_cond(CheckReducible, f, lforward(args)...);
		}


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
			ReductionStatus
				operator()(_tIn first, _tIn last) const
			{
				auto res(ReductionStatus::Clean);

				return leo::fast_any_of(first, last, [&](ReductionStatus r) lnothrow{
					if (r == ReductionStatus::Retained)
					res = r;
				return r == ReductionStatus::Retrying;
				}) ? ReductionStatus::Retrying : res;
			}
		};


		/*!
		\note 结果表示判断是否应继续规约。
		\sa PassesCombiner
		*/
		//@{
		//! \brief 一般合并遍。
		template<typename... _tParams>
		using GPasses = leo::GEvent<ReductionStatus(_tParams...),
			leo::GCombinerInvoker<ReductionStatus, PassesCombiner>>;
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


		/*!
		\brief 调用处理遍：从指定名称的节点中访问指定类型的遍并以指定上下文调用。
		//@{
		*/
		template<class _tPasses, typename... _tParams>
		typename _tPasses::result_type
			InvokePasses(observer_ptr<const ValueNode> p, TermNode& term, ContextNode& ctx,
				_tParams&&... args)
		{
			return leo::call_value_or([&](const _tPasses& passes) {
				// XXX: Blocked. 'yforward' cause G++ 5.3 crash: internal compiler
				//	error: Segmentation fault.
				return passes(term, ctx, std::forward<_tParams>(args)...);
			}, leo::AccessPtr<const _tPasses>(p));
		}
		template<class _tPasses, typename... _tParams>
		inline typename _tPasses::result_type
			InvokePasses(const string& name, TermNode& term, ContextNode& ctx,
				_tParams&&... args)
		{
			return scheme::InvokePasses<_tPasses>(leo::AccessNodePtr(
				leo::as_const(ctx), name), term, ctx, lforward(args)...);
		}
		//@}

		/*!
		\brief 调整指定值对象的指针为 TermNode 类型的值。
		\return 若成功则为调整后的指针，否则为指向原值对象的指针。
		*/
		//@{
		inline PDefH(observer_ptr<const ValueObject>, AdjustTermValuePtr,
			observer_ptr<const TermNode> p_term)
			ImplRet(leo::nonnull_or(GetValuePtrOf(p_term)))
			inline PDefH(observer_ptr<const ValueObject>, AdjustTermValuePtr,
				observer_ptr<const ValueObject> p_vo)
			ImplRet(leo::nonnull_or(
				AdjustTermValuePtr(leo::AccessPtr<const TermNode>(p_vo)), p_vo))
			template<typename _tKey>
		observer_ptr<const ValueObject>
			AdjustTermValuePtr(const ContextNode& ctx, const _tKey& name)
		{
			return AdjustTermValuePtr(scheme::FetchValuePtr(ctx, name));
		}
		//@}

		//! \brief 使用第二个参数指定的项的内容替换第一个项的内容。
		//@{
		inline PDefH(void, LiftTerm, TermNode& term, TermNode& tm)
			ImplExpr(term.SetContent(std::move(tm)))

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
			//@}
			//@}

			/*!
			\brief 提升延迟项。
			*/
			inline PDefH(void, LiftDelayed, TermNode& term, DelayedTerm& tm)
			ImplExpr(LiftTermRef(term, tm))

			//! \pre 断言：参数指定的项是枝节点。
			//@{
			/*!
			\brief 提升首项：使用首个子项替换项的内容。
			*/
			inline PDefH(void, LiftFirst, TermNode& term)
			ImplExpr(IsBranch(term), LiftTerm(term, Deref(term.begin())))

			/*!
			\brief 提升末项：使用最后一个子项替换项的内容。
			*/
			inline PDefH(void, LiftLast, TermNode& term)
			ImplExpr(IsBranch(term), LiftTerm(term, Deref(term.rbegin())))
			//@}

			/*!
			\sa RemoveHead
			\note 使用 ADL RemoveHead 。
			*/
			//@{
			/*!
			\brief 规约第一个非结尾空列表子项。
			\return ReductionStatus::Clean 。

			若项具有不少于一个子项且第一个子项是空列表则移除。
			允许空列表作为第一个子项以标记没有操作数的函数应用。
			*/
			LS_API ReductionStatus
			ReduceHeadEmptyList(TermNode&) lnothrow;

		/*!
		\brief 规约为列表：对枝节点移除第一个子项，保留余下的子项作为列表。
		\return 若成功移除项 ReductionStatus::Retained ，否则为 ReductionStatus::Clean。
		*/
		LS_API ReductionStatus
			ReduceToList(TermNode&) lnothrow;
		//@}


} // namespace scheme;

#endif
