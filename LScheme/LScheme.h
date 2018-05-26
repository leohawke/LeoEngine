/*!	\file LScheme.h
\par �޸�ʱ��:
2017-03-27 11:05 +0800
*/

#ifndef LScheme_LScheme_H
#define LScheme_LScheme_H 1

#include "LSchemeA.h" // for LSLATag, ValueNode, TermNode, LoggedEvent,
//	leo::Access, leo::equality_comparable, leo::exclude_self_t,
//	leo::AreEqualHeld, leo::GHEvent, leo::make_function_type_t,
//	leo::make_parameter_tuple_t, leo::make_expanded,
//	leo::invoke_nonvoid, leo::make_transform, std::accumulate,
//	leo::bind1, std::placeholders::_2, leo::examiners::equal_examiner;

namespace scheme {
	namespace v1 {
		/*!
		\brief LSLV1 Ԫ��ǩ��
		\note LSLV1 �� LSLA �ľ���ʵ�֡�
		*/
		struct LS_API LSLV1Tag : LSLATag
		{};


		//! \brief ֵ�Ǻţ��ڵ��е�ֵ��ռλ����
		enum class ValueToken
		{
			Null,
			/*!
			\brief δ����ֵ��
			*/
			Undefined,
			/*!
			\brief δָ��ֵ��
			*/
			Unspecified,
			GroupingAnchor,
			OrderedAnchor
		};

		/*!
		\brief ȡֵ�Ǻŵ��ַ�����ʾ��
		\return ��ʾ��Ӧ�Ǻ�ֵ���ַ�������֤����ȵ������Ӧ����ȵĽ����
		\throw std::invalid_argument �����ֵ���ǺϷ���ֵ�Ǻš�
		\relates ValueToken
		*/
		LS_API string
			to_string(ValueToken);

		//@{
		//! \brief ���� LSLV1 �ӽڵ㡣
		//@{
		/*!
		\note �������������˳��

		��һ����ָ���ı任�������ڶ�����ָ����������
		��ӳ��������ؽڵ�����Ϊ������ݵ�ǰ�������ӽڵ�������ǰ׺ $ �����Ա����ظ���
		*/
		LS_API void
			InsertChild(TermNode&&, TermNode::Container&);

		/*!
		\note ����˳��

		ֱ�Ӳ��� LSLV1 �ӽڵ㵽��������ĩβ��
		*/
		LS_API void
			InsertSequenceChild(TermNode&&, NodeSequence&);
		//@}

		/*!
		\brief �任 LSLV1 �ڵ� S ���ʽ�����﷨��Ϊ LSLV1 ����ṹ��
		\exception std::bad_function_call �����������Ϊ�ա�
		\return �任����½ڵ㣨���ӽڵ㣩��

		��һ����ָ��Դ�ڵ㣬�������ָ�����ֱ任����
		�������õĵڶ������Ĳ������޸Ĵ���Ľڵ����ʱ�任���޸�Դ�ڵ㡣
		�������£�
		��Դ�ڵ�ΪҶ�ڵ㣬ֱ�ӷ���ʹ�õ�����������ӳ��Ľڵ㡣
		��Դ�ڵ�ֻ��һ���ӽڵ㣬ֱ�ӷ�������ӽڵ�ı任�����
		����ʹ�õ��Ĳ����ӵ�һ���ӽڵ�ȡ��Ϊ�任������Ƶ��ַ�����
			�����Ʒǿ�����Ե�һ���ӽڵ㣬ֻ�任ʣ���ӽڵ㡣
				��ʣ��һ���ӽڵ㣨��Դ�ڵ��������ӽڵ㣩ʱ��ֱ�ӵݹ�任����ڵ㲢���ء�
				���任��Ľ�����Ʒǿգ�����Ϊ�����ֵ�����򣬽����Ϊ�����ڵ�һ��ֵ��
			�����½��ڵ��������������任ʣ��Ľڵ������������������������������Ľڵ㡣
				�ڶ�����ָ����ʱ��ӳ���������Ϊ����Ĭ��ʹ�õݹ� TransformNode ���á�
				���õ����������ӳ��Ľ����������
		*/
		LS_API ValueNode
			TransformNode(const TermNode&, NodeMapper = {}, NodeMapper = MapLSLALeafNode,
				NodeToString = ParseLSLANodeString, NodeInserter = InsertChild);

		/*!
		\brief �任 LSLV1 �ڵ� S ���ʽ�����﷨��Ϊ LSLV1 ��������ṹ��
		\exception std::bad_function_call �����������Ϊ�ա�
		\return �任����½ڵ㣨���ӽڵ㣩��
		\sa TransformNode

		�� TransformNode �任������ͬ��
		��������ӽڵ��� NodeSequence ����ʽ��Ϊ�任�ڵ��ֵ�������ӽڵ㣬�ɱ���˳��
		*/
		LS_API ValueNode
			TransformNodeSequence(const TermNode&, NodeMapper = {},
				NodeMapper = MapLSLALeafNode, NodeToString = ParseLSLANodeString,
				NodeSequenceInserter = InsertSequenceChild);
		//@}


		/*!
		\brief ���� LSLV1 ���뵥Ԫ��
		\throw LoggedEvent ���棺�����������е�ʵ��ת��ʧ�ܡ�
		*/
		//@{
		template<typename _type, typename... _tParams>
		ValueNode
			LoadNode(_type&& tree, _tParams&&... args)
		{
			TryRet(v1::TransformNode(std::forward<TermNode&&>(tree),
				lforward(args)...))
				CatchThrow(leo::bad_any_cast& e, LoggedEvent(leo::sfmt(
					"Bad LSLV1 tree found: cast failed from [%s] to [%s] .", e.from(),
					e.to()), leo::Warning))
		}

		template<typename _type, typename... _tParams>
		ValueNode
			LoadNodeSequence(_type&& tree, _tParams&&... args)
		{
			TryRet(v1::TransformNodeSequence(std::forward<TermNode&&>(tree),
				lforward(args)...))
				CatchThrow(leo::bad_any_cast& e, LoggedEvent(leo::sfmt(
					"Bad LSLV1 tree found: cast failed from [%s] to [%s] .", e.from(),
					e.to()), leo::Warning))
		}
		//@}


		//@{
		//! \brief �����ػ��顣
		LS_API GuardPasses&
			AccessGuardPassesRef(ContextNode&);

		//! \brief ����Ҷ�顣
		LS_API EvaluationPasses&
			AccessLeafPassesRef(ContextNode&);

		//! \brief �����б�顣
		LS_API EvaluationPasses&
			AccessListPassesRef(ContextNode&);
		//@}

		/*!
		\brief �����������顣
		*/
		LS_API LiteralPasses&
			AccessLiteralPassesRef(ContextNode&);

		//! \sa InvokePasses
		//@{
		/*!
		\brief �����ػ��顣
		\sa GuardPasses
		*/
		LS_API Guard
			InvokeGuard(TermNode& term, ContextNode&);

		/*!
		\sa EvaluationPasses
		*/
		//@{
		//! \brief ����Ҷ�顣
		LS_API ReductionStatus
			InvokeLeaf(TermNode& term, ContextNode&);

		//! \brief �����б�顣
		LS_API ReductionStatus
			InvokeList(TermNode& term, ContextNode&);
		//@}

		/*!
		\brief �����������顣
		\pre ���ԣ��ַ�������������ָ��ǿա�
		\sa LiteralPasses
		*/
		LS_API ReductionStatus
			InvokeLiteral(TermNode&, ContextNode&, string_view);
		//@}


		/*!
		\brief LSLV1 ���ʽ�ڵ��Լ����������һ����ֵ���̹�Լ�ӱ��ʽ��
		\return ��Լ״̬��
		\note ����ʹ�����������ĵ�����ʧЧ��
		\note Ĭ�ϲ���Ҫ�ع�Լ����ɱ���ֵ��ı䡣
		\note �ɱ���ֵ�������ʵ�ֵݹ���ֵ��
		\note �쳣��ȫȡ���ڵ��ñ������쳣��ȫ��֤��
		\sa DetectReducible
		\sa InvokeGuard
		\sa InvokeLeaf
		\sa InvokeList
		\sa ValueToken
		\todo ʵ�� ValueToken ��������

		��Լ˳�����£�
		���� InvokeGuard ���б�Ҫ�����������ã�
		������Լ��ֱ������Ҫ�����ع�Լ��
		��Ӧ��ͬ�Ľڵ�μ��ṹ���࣬һ�ε���������˳���ж�ѡ�����·�֧֮һ�������Լ���
		��֦�ڵ���� InvokeList ��ֵ��
		�Կսڵ��滻Ϊ ValueToken::Null ��
		�����滻Ϊ ValueToken ��Ҷ�ڵ㱣������
		������Ҷ�ڵ���� InvokeLeaf ��ֵ��
		��һ��ֵ�Ľ����Ϊ DetectReducible �ĵڶ����������ݽ���ж��Ƿ�����ع�Լ��
		�˴�Լ���ĵ����жԽڵ�ľ���ṹ����Ĭ��Ҳ���������� LSLV1 ʵ�� API ��
		�������Ӧ����ָ����ȷ��˳��
		���������������ڵ㲻�Ǳ��ʽ����ṹ�����ǳ����﷨������ API ���� TransformNode ��
		*/
		LS_API ReductionStatus
			Reduce(TermNode&, ContextNode&);

		/*!
		\note �����Թ淶�������Լ˳��δָ����
		\note ����ʹ�����������ĵ�����ʧЧ��
		\sa Reduce
		*/
		//@{
		//! \note ���������ع�ԼҪ��
		//@{
		/*!
		\brief �������еĵڶ��ʼ�����Լ��
		\throw InvalidSyntax ����Ϊ�� ��
		\sa ReduceChildren
		*/
		//@{
		LS_API void
			ReduceArguments(TNIter, TNIter, ContextNode&);

		inline PDefH(void, ReduceArguments, TermNode::Container& con, ContextNode& ctx)
			ImplRet(ReduceArguments(con.begin(), con.end(), ctx))
			inline PDefH(void, ReduceArguments, TermNode& term, ContextNode& ctx)
			ImplRet(ReduceArguments(term.GetContainerRef(), ctx))
			//@}

		/*!
		\todo ʹ�ø�ȷ�е��쳣���͡�
		*/
		//@{
		/*!
		\brief ��Լ�����ɹ������� Reduce ���������ʧ��ʱ�׳��쳣��
		\throw LSLException Reduce ������� ReductionStatus::Success��
		\sa CheckedReduceWith
		\sa Reduce
		*/
		LS_API void
			ReduceChecked(TermNode&, ContextNode&);

		/*!
		\brief ��Լ�հ���ʹ�õ��Ĳ���ָ���ıհ����Լ���滻��ָ�����ϡ�
		\sa ReduceChecked

		�����Լ���Լ���滻����һ����ָ���
		��Լ��������ɵ��Ĳ����ıհ�ָ������������ָ���Ƿ�ͨ��ת�ƹ����������ԭ�
		��Լ��ת�Ʊհ���Լ�Ľ���������Լ����õ�ֵ��Ŀ�걻ת�Ƶ���һ����ָ�����
		����������ֵ֮�䱻ת�Ƶ����˳��δָ����
		*/
		LS_API void
			ReduceCheckedClosure(TermNode&, ContextNode&, bool, TermNode&);
		//@}

		/*!
		\brief ��Լ���
		*/
		//@{
		LS_API void
			ReduceChildren(TNIter, TNIter, ContextNode&);
		inline PDefH(void, ReduceChildren, TermNode::Container& con, ContextNode& ctx)
			ImplExpr(ReduceChildren(con.begin(), con.end(), ctx))
			inline PDefH(void, ReduceChildren, TermNode& term, ContextNode& ctx)
			ImplExpr(ReduceChildren(term.GetContainerRef(), ctx))
			//@}
			//@}

			/*!
			\brief �����Լ���
			\return ����������ʱΪ���һ������Ĺ�Լ״̬������Ϊ ReductionStatus::Clean ��
			*/
			//@{
			LS_API ReductionStatus
			ReduceChildrenOrdered(TNIter, TNIter, ContextNode&);
		inline PDefH(ReductionStatus, ReduceChildrenOrdered, TermNode::Container& con,
			ContextNode& ctx)
			ImplRet(ReduceChildrenOrdered(con.begin(), con.end(), ctx))
			inline PDefH(ReductionStatus, ReduceChildrenOrdered, TermNode& term,
				ContextNode& ctx)
			ImplRet(ReduceChildrenOrdered(term.GetContainerRef(), ctx))
			//@}

			/*!
			\brief ��Լ��һ�����
			\return ��Լ״̬��
			\sa Reduce
			\see https://en.wikipedia.org/wiki/Fexpr ��

			�����ϸ��Է�����
			��������ֵ֦�ڵ��һ���Ա����ȷ�����ƶ��ӱ��ʽ��ֵ�ĸ��Ӹ��Ӷȡ�
			*/
			LS_API ReductionStatus
			ReduceFirst(TermNode&, ContextNode&);

		/*!
		\brief ��Լ�������У�˳���Լ������Ϊ���һ������Ĺ�Լ�����
		\return ����������ʱΪ���һ������Ĺ�Լ״̬������Ϊ ReductionStatus::Clean ��
		\sa ReduceChildrenOrdered
		*/
		LS_API ReductionStatus
			ReduceOrdered(TermNode&, ContextNode&);

			/*!
			\brief �Ƴ��������ָ��������������Լ��
			*/
			LS_API ReductionStatus
			ReduceTail(TermNode&, ContextNode&, TNIter);
		//@}


			/*!
			\brief ���ø�����Ƚڵ㣺���ù�Լʱ��ʾ��Ⱥ������ĵ���Ϣ��
			\note ��Ҫ���ڵ��ԡ�
			\sa InvokeGuard
			*/
			LS_API void
			SetupTraceDepth(ContextNode& ctx, const string& name = limpl("$__depth"));


		/*!
		\note ValueObject �����ֱ�ָ���滻��ӵ�ǰ׺�ͱ��滻�ķָ�����ֵ��
		*/
		//@{
		/*!
		\note �Ƴ�������ֵ��ָ���ָ���ָ��������� AsIndexNode ���ָ��ǰ׺ֵ��Ϊ���
		\note ���һ������ָ������ֵ�����ơ�
		\sa AsIndexNode
		*/
		//@{
		//! \brief �任�ָ�����׺���ʽΪǰ׺���ʽ��
		LS_API TermNode
			TransformForSeparator(const TermNode&, const ValueObject&, const ValueObject&,
				const string& = {});

		//! \brief �ݹ�任�ָ�����׺���ʽΪǰ׺���ʽ��
		LS_API TermNode
			TransformForSeparatorRecursive(const TermNode&, const ValueObject&,
				const ValueObject&, const string& = {});
		//@}

		/*!
		\brief �������е�ָ���ָ��������ҵ����滻��Ϊȥ���ָ���������滻ǰ׺����ʽ��
		\return �Ƿ��ҵ����滻���
		\sa EvaluationPasses
		\sa TransformForSeparator
		*/
		LS_API ReductionStatus
			ReplaceSeparatedChildren(TermNode&, const ValueObject&, const ValueObject&);
		//@}


		//@{
		/*!
		\brief ��װ�����Ĵ�������
		\note ���Ա���װ�������Ĵ��������ܴ��ڵķ���ֵ������ӦĬ�Ϸ��ع�Լ�����
		*/
		template<typename _func>
		struct WrappedContextHandler
			: private leo::equality_comparable<WrappedContextHandler<_func>>
		{
			_func Handler;

			//@{
			template<typename _tParam, limpl(typename
				= leo::exclude_self_t<WrappedContextHandler, _tParam>)>
				WrappedContextHandler(_tParam&& arg)
				: Handler(lforward(arg))
			{}
			template<typename _tParam1, typename _tParam2, typename... _tParams>
			WrappedContextHandler(_tParam1&& arg1, _tParam2&& arg2, _tParams&&... args)
				: Handler(lforward(arg1), lforward(arg2), lforward(args)...)
			{}

			DefDeCopyMoveCtorAssignment(WrappedContextHandler)
				//@}

				template<typename... _tParams>
			ReductionStatus
				operator()(_tParams&&... args) const
			{
				Handler(lforward(args)...);
				return ReductionStatus::Clean;
			}

			/*!
			\brief �Ƚ������Ĵ�������ȡ�
			\note ʹ�� leo::AreEqualHeld ��
			*/
			friend PDefHOp(bool, == , const WrappedContextHandler& x,
				const WrappedContextHandler& y)
				ImplRet(leo::AreEqualHeld(x.Handler, y.Handler))
		};

		template<class _tDst, typename _func>
		inline _tDst
			WrapContextHandler(_func&& h, leo::false_)
		{
			return WrappedContextHandler<leo::GHEvent<leo::make_function_type_t<
				void, leo::make_parameter_tuple_t<typename _tDst::BaseType>>>>(
					lforward(h));
		}
		template<class, typename _func>
		inline _func
			WrapContextHandler(_func&& h, leo::true_)
		{
			return lforward(h);
		}
		template<class _tDst, typename _func>
		inline _tDst
			WrapContextHandler(_func&& h)
		{
			using BaseType = typename _tDst::BaseType;

			// XXX: It is a hack to adjust the convertible result for the expanded
			//	caller here. It should have been implemented in %GHEvent, however types
			//	those cannot convert to expanded caller cannot be SFINAE'd out,
			//	otherwise it would cause G++ 5.4 crash with internal compiler error:
			//	"error reporting routines re-entered".
			return v1::WrapContextHandler<_tDst>(lforward(h), leo::or_<
				std::is_constructible<BaseType, _func&&>,
				std::is_constructible<BaseType, leo::expanded_caller<
				typename _tDst::FuncType, leo::decay_t<_func>>>>());
		}
		//@}

		/*!
		\brief ��ʽ�����Ĵ�������
		*/
		class LS_API FormContextHandler
		{
		public:
			ContextHandler Handler;
			/*!
			\brief �������̣���֤����װ�Ĵ������ĵ��÷���ǰ��������
			*/
			std::function<bool(const TermNode&)> Check{IsBranch};

			template<typename _func,
				limpl(typename = leo::exclude_self_t<FormContextHandler, _func>)>
				FormContextHandler(_func&& f)
				: Handler(v1::WrapContextHandler<ContextHandler>(lforward(f)))
			{}
			template<typename _func, typename _fCheck>
			FormContextHandler(_func&& f, _fCheck c)
				: Handler(v1::WrapContextHandler<ContextHandler>(lforward(f))), Check(c)
			{}

			DefDeCopyMoveCtorAssignment(FormContextHandler)

			/*!
			\brief �Ƚ������Ĵ�������ȡ�
			\note ���Լ�����̵ĵȼ��ԡ�
			*/
			friend PDefHOp(bool, == , const FormContextHandler& x,
					const FormContextHandler& y)
				ImplRet(x.Handler == y.Handler)

			/*!
			\brief ����һ����ʽ��
			\exception LSLException �쳣������
			\throw LoggedEvent ���棺���Ͳ�ƥ�䣬
			�� Handler �׳��� leo::bad_any_cast ת����
			\throw LoggedEvent ������ Handler �׳��� leo::bad_any_cast ���
			std::exception ת����
			\throw std::invalid_argument ����δͨ����

			���鲻���ڻ��ڼ��ͨ���󣬶Խڵ���� Hanlder �������׳��쳣��
			*/
			ReductionStatus
				operator()(TermNode&, ContextNode&) const;
		};


		/*!
		\brief �ϸ������Ĵ�������
		*/
		class LS_API StrictContextHandler
		{
		public:
			FormContextHandler Handler;

			template<typename _func,
				limpl(typename = leo::exclude_self_t<StrictContextHandler, _func>)>
				StrictContextHandler(_func&& f)
				: Handler(lforward(f))
			{}
			template<typename _func, typename _fCheck>
			StrictContextHandler(_func&& f, _fCheck c)
				: Handler(lforward(f), c)
			{}

			DefDeCopyMoveCtorAssignment(StrictContextHandler)

			friend PDefHOp(bool, == , const StrictContextHandler& x,
					const StrictContextHandler& y)
				ImplRet(x.Handler == y.Handler)
			/*!
			\brief ��������
			\throw ListReductionFailure �б��������һ�
			\sa ReduceArguments

			��ÿһ��������ֵ��Ȼ�����������Կɵ��õ������ Hanlder �������׳��쳣��
			*/
			ReductionStatus
				operator()(TermNode&, ContextNode&) const;
		};


		//@{
		template<typename... _tParams>
		inline void
			RegisterForm(ContextNode& node, const string& name,
				_tParams&&... args)
		{
			scheme::RegisterContextHandler(node, name,
				FormContextHandler(lforward(args)...));
		}

		//! \brief ת�������Ĵ�������
		template<typename... _tParams>
		inline ContextHandler
			ToContextHandler(_tParams&&... args)
		{
			return StrictContextHandler(lforward(args)...);
		}

		/*!
		\brief ע���ϸ������Ĵ�������
		\note ʹ�� ADL ToContextHandler ��
		*/
		template<typename... _tParams>
		inline void
			RegisterStrict(ContextNode& node, const string& name, _tParams&&... args)
		{
			scheme::RegisterContextHandler(node, name,
				ToContextHandler(lforward(args)...));
		}
		//@}

		/*!
		\brief ע��ָ���ת���任�ʹ������̡�
		\sa LSL::RegisterContextHandler
		\sa ReduceChildren
		\sa ReduceOrdered
		\sa ReplaceSeparatedChildren

		�任������׺��ʽ�ķָ����Ǻŵı��ʽΪָ�����Ƶ�ǰ׺���ʽ��ȥ���ָ�����
		Ȼ��ע��ǰ׺�﷨��ʽ��
		���һ������ָ���Ƿ�����ѡ���﷨��ʽΪ ReduceOrdered �� ReduceChildren ֮һ��
		ǰ׺���Ʋ���Ҫ�ǼǺ�֧�ֵı�ʶ����
		*/
		LS_API void
			RegisterSequenceContextTransformer(EvaluationPasses&, ContextNode&,
				const string&, const ValueObject&, bool = {});


		/*!
		\brief ����֦�ڵ㡣
		\pre ���ԣ�����ָ��������֦�ڵ㡣
		*/
		inline PDefH(void, AssertBranch, const TermNode& term,
			const char* msg = "Invalid term found.") lnothrowv
			ImplExpr(lunused(msg), LAssert(IsBranch(term), msg))

		/*!
		\brief ȡ��Ĳ����������������� 1 ��
		\pre ��Ӷ��ԣ�����ָ��������֦�ڵ㡣
		\return ��Ĳ���������
		*/
		inline PDefH(size_t, FetchArgumentN, const TermNode& term) lnothrowv
		ImplRet(AssertBranch(term), term.size() - 1)

		//! \note ��һ����ָ���������� Value ָ�������ֵ��
		//@{
		//! \sa LiftDelayed
		//@{
		/*!
		\brief ��ֵ�Խڵ����ݽṹ��ӱ�ʾ���

		�� TermNode �������ֵ�����ɹ����� LiftTermRef �滻ֵ������Ҫ���ع�Լ��
		������ʶԹ�Լ����ת�ƵĿ���δ��ֵ�Ĳ������Ǳ�Ҫ�ġ�
		*/
		LS_API ReductionStatus
		EvaluateDelayed(TermNode&);
		/*!
		\brief ��ֵָ�����ӳ���ֵ�
		\return ReductionStatus::Retrying ��

		����ָ�����ӳ���ֵ���Լ��
		*/
		LS_API ReductionStatus
			EvaluateDelayed(TermNode&, DelayedTerm&);
		//@}


		/*!
		\exception BadIdentifier ��ʶ��δ������
		\note ��һ����ָ���������� Value ָ�������ֵ��
		\note Ĭ����Ϊ��Լ�ɹ��Ա�֤ǿ�淶�����ʡ�
		*/
		//@{
		//! \pre ���ԣ���������������ָ��ǿա�
		//@{
		/*!
		\brief ��ֵ��ʶ����
		\note ����֤��ʶ���Ƿ�Ϊ����������������������ʱ������Ҫ�ع�Լ��
		\sa EvaluateDelayed
		\sa LiftTermRef
		\sa LiteralHandler
		\sa ResolveName

		���ν���������ֵ������
		���� ResolveName ����ָ�����Ʋ���ֵ����ʧ���׳�δ�����쳣��
		���� LiftTermRef �� TermNode::SetContentIndirect �滻���б���б�ڵ��ֵ��
		�� LiteralHandler ���������������������ɹ����ò������������������Ĵ�������
		��δ���أ����ݽڵ��ʾ��ֵ��һ������
			�Ա�ʾ�� TokenValue ֵ��Ҷ�ڵ㣬���� EvaluateDelayed ��ֵ��
			�Ա�ʾ TokenValue ֵ��Ҷ�ڵ㣬���� ReductionStatus::Retrying �ع�Լ��
			��֦�ڵ���Ϊ�б����� ReductionStatus::Retained �����һ����ֵ��
		*/
		LS_API ReductionStatus
			EvaluateIdentifier(TermNode&, const ContextNode&, string_view);

		/*!
		\brief ��ֵҶ�ڵ�Ǻš�
		\sa CategorizeLiteral
		\sa DeliteralizeUnchecked
		\sa EvaluateIdentifier
		\sa InvokeLiteral

		����ǿ��ַ�����ʾ�Ľڵ�Ǻš�
		���ν���������ֵ������
		�Դ�����������ȥ���������߽�ָ������һ����ֵ��
		��������������ȥ���������߽�ָ�����Ϊ�ַ���ֵ��
		��������������ͨ�������������鴦��
		�����ֵ���������ı�ʶ����
		*/
		LS_API ReductionStatus
			EvaluateLeafToken(TermNode&, ContextNode&, string_view);
		//@}

		/*!
		\brief ��Լ�ϲ�������ĵ�һ���������Ϊ���������к���Ӧ�ã����淶����
		\return ��Լ״̬��
		\throw ListReductionFailure ��Լʧ�ܣ�֦�ڵ�ĵ�һ�������ʾ�����Ĵ�������
		\sa ContextHandler
		\sa Reduce

		��֦�ڵ㳢���Ե�һ������� Value ���ݳ�ԱΪ�����Ĵ����������ã��ҵ���Լ��ֹʱ�淶����
		������Ϊ��Լ�ɹ���û���������á�
		������ ContextHandler ���ã�����ǰ��ת�ƴ�������֤�����ڣ�
		�����������ڲ��Ƴ����޸�֮ǰռ�õĵ�һ������������е� Value ���ݳ�Ա����
		*/
		LS_API ReductionStatus
			ReduceCombined(TermNode&, ContextNode&);

		/*!
		\brief ��Լ��ȡ���Ƶ�Ҷ�ڵ�Ǻš�
		\sa EvaluateLeafToken
		\sa TermToNode
		*/
		LS_API ReductionStatus
			ReduceLeafToken(TermNode&, ContextNode&);
		//@}

		/*!
		\brief �������ƣ����������Ʋ��������ơ�
		\pre ���ԣ��ڶ�����������ָ��ǿա�
		*/
		LS_API observer_ptr<const ValueNode>
			ResolveName(const ContextNode&, string_view);

		/*!
		\brief ����Ĭ�Ͻ��ͣ�����ʹ�õĹ�������顣
		\note ��ǿ�쳣��ȫ������������ǰ����״̬������ʧ��ʱ�ع���
		\sa EvaluateContextFirst
		\sa ReduceFirst
		\sa ReduceLeafToken
		*/
		LS_API void
			SetupDefaultInterpretation(ContextNode&, EvaluationPasses);

		/*!
		\brief LSLV1 �﷨��ʽ��Ӧ�Ĺ���ʵ�֡�
		*/
		namespace Forms
		{
			/*!
			\brief �ж��ַ���ֵ�Ƿ�ɹ��ɷ��š�
			�ο��ķ���

			symbol? <object>
			*/
			LS_API bool
				IsSymbol(const string&) lnothrow;

			//@{
			/*!
			\brief ��������ָ���ַ���ֵ�ļǺ�ֵ��
			\note �����ֵ�Ƿ���Ϸ���Ҫ��
			*/
			LS_API TokenValue
				StringToSymbol(const string&);

			//! \brief ȡ���Ŷ�Ӧ�������ַ�����
			LS_API const string&
				SymbolToString(const TokenValue&) lnothrow;
			//@}


			/*!
			\pre ���ԣ����������Ӧ֦�ڵ㡣
			\sa AssertBranch
			*/
			//@{
			/*!
			\note ������ֵ����������;��һ�㲻��Ҫ����Ϊ�û�����ֱ��ʹ�á�

			��ʹ�� RegisterForm ע�������Ĵ��������ο��ķ���
			$retain|$retainN <expression>
			*/
			//@{
			//! \brief �����������ֵ��
			inline PDefH(ReductionStatus, Retain, const TermNode& term) lnothrowv
				ImplRet(AssertBranch(term), ReductionStatus::Retained)


				/*!
				\brief ���������ȷ������ָ�������������������ֵ��
				\return ��Ĳ���������
				\throw ArityMismatch ��Ĳ������������ڵڶ�������
				\sa FetchArgumentN
				*/
				LS_API size_t
				RetainN(const TermNode&, size_t = 1);
			//@}

			/*!
			\throw ParameterMismatch ƥ��ʧ�ܡ�
			\note ������ǿ�쳣��ȫ��֤��ƥ��ʧ��ʱ�������İ�״̬δָ����

			��ǰ��Ҫ���нṹ��ƥ���顣
			��ƥ��ʧ�ܣ����׳��쳣�������ڵ�һ����ָ���Ļ����ڰ�δ�����Ե�ƥ����
			�ǿ��б���İ�Ϊ��Ӧ����İ󶨣�
			���б���İ�Ϊ�ղ�����
			���б��������ֱ�Ӱ󶨵�����Ϊ����ֵ�Ĳ�����Ӧ�
			�󶨰�������ȵĴʷ�˳����С����Ѵ��ڰ������°󶨡�
			*/
			//@{
			/*!
			\brief ʹ�ò������ṹ��ƥ�䲢�󶨲�����
			\throw ArityMismatch ������ƥ��ʧ�ܡ�
			\sa BindParameterLeaf

			��ʽ�����Ͳ�����Ϊ��ָ���ı��ʽ����
			�ڶ�����ָ����ʽ��������������ָ����������
			����ƥ����㷨�ݹ�������ʽ�����������ƥ��Ҫ�����£�
			�����Ƿǿ��б���������Ķ�Ӧ����ӦΪ����ȷ�����������б�
			����������Ϊ���� ... ����ƥ��������н�β������������
			��������һһƥ������������
			�����ǿ��б���������Ķ�Ӧ����ӦΪ���б�
			���򣬵��� BindParameterLeaf ƥ����б��
			*/
			LS_API void
				BindParameter(ContextNode&, const TermNode&, TermNode&);

			/*!
			\brief ʹ�ò������ṹ��ƥ�䲢�󶨲��������б��
			\sa BindParameter

			��ʽ�����Ͳ�����Ϊ��ָ���ı��ʽ����
			�ڶ�����ָ�����ƣ�֮��Ĳ���ָ��ֵ��
			ƥ��Ҫ�����£�
			������ #ignore ������Բ�������Ӧ���
			�����ֵ�Ƿ��ţ���������Ķ�Ӧ����ӦΪ���б��
			*/
			//@{
			LS_API void
				BindParameterLeaf(ContextNode&, const TokenValue&, TermNode::Container&&,
					ValueObject&&);
			inline PDefH(void, BindParameterLeaf, ContextNode& e, const TokenValue& n,
				TermNode&& o)
				ImplExpr(BindParameterLeaf(e, n, std::move(o.GetContainerRef()),
					std::move(o.Value)))
				//@}
				//@}

				/*!
				\brief ��������Ƿ����Ϊ���η��ĵڶ���������������Ƴ���
				\return �Ƿ���ڲ��Ƴ������η���

				����һ����ָ�������������Ƿ���ڵڶ�����ָ�������η�Ϊ��ĵ�һ������
				����鷢�ִ������η����Ƴ���
				*/
				//@{
				LS_API bool
				ExtractModifier(TermNode::Container&, const ValueObject& = string("!"));
			inline PDefH(bool, ExtractModifier, TermNode& term,
				const ValueObject& mod = string("!"))
				ImplRet(ExtractModifier(term.GetContainerRef(), mod))
				//@}

				//! \brief ��Լ���ܴ������η����
				template<typename _func>
			void
				ReduceWithModifier(TermNode& term, ContextNode& ctx, _func f)
			{
				const bool mod(ExtractModifier(term));

				if (IsBranch(term))
					f(term, ctx, mod);
				else
					throw InvalidSyntax("Argument not found.");
			}
			//@}


			/*!
			\brief ���ʽڵ㲢����һԪ������
			\sa leo::EmplaceCallResult
			ȷ�������һ��ʵ�ʲ�����չ�����ò���ָ���ĺ�����
			�������õĺ����������ͷ� void ������ֵ��Ϊ���ֵ�����졣
			���� leo::EmplaceCallResult �� ValueObject ������ֵ����ͬ��
			�����Ժ��������͵�ֵ���Ƶķ�ʽ����װ���ڵ�һ�������й��� ValueObject ����
			*/
			//@{
			template<typename _func, typename... _tParams>
			void
				CallUnary(_func&& f, TermNode& term, _tParams&&... args)
			{
				RetainN(term);
				leo::EmplaceCallResult(term.Value, leo::invoke_nonvoid(
					leo::make_expanded<void(TermNode&, _tParams&&...)>(lforward(f)),
					Deref(std::next(term.begin())), lforward(args)...));
			}

			template<typename _type, typename _func, typename... _tParams>
			void
				CallUnaryAs(_func&& f, TermNode& term, _tParams&&... args)
			{
				Forms::CallUnary([&](TermNode& node) {
					// XXX: Blocked. 'lforward' cause G++ 5.3 crash: internal compiler
					//	error: Segmentation fault.
					return leo::make_expanded<void(_type&, _tParams&&...)>(lforward(f))(
						leo::Access<_type>(node), std::forward<_tParams&&>(args)...);
				}, term);
			}
			//@}

			/*!
			\brief ���ʽڵ㲢���ö�Ԫ������
			*/
			//@{
			template<typename _func, typename... _tParams>
			void
				CallBinary(_func&& f, TermNode& term, _tParams&&... args)
			{
				RetainN(term, 2);

				auto i(term.begin());
				auto& x(Deref(++i));

				leo::EmplaceCallResult(term.Value, leo::invoke_nonvoid(
					leo::make_expanded<void(TermNode&, TermNode&, _tParams&&...)>(
						lforward(f)), x, Deref(++i), lforward(args)...));
			}

			template<typename _type, typename _func, typename... _tParams>
			void
				CallBinaryAs(_func&& f, TermNode& term, _tParams&&... args)
			{
				RetainN(term, 2);

				auto i(term.begin());
				auto& x(leo::Access<_type>(Deref(++i)));

				leo::EmplaceCallResult(term.Value, leo::invoke_nonvoid(
					leo::make_expanded<void(_type&, _type&, _tParams&&...)>(lforward(f)),
					x, leo::Access<_type>(Deref(++i)), lforward(args)...));
			}
			//@}

			/*!
			\brief ���ʽڵ㲢��ָ���ĳ�ʼֵΪ����������ö�Ԫ������
			\note Ϊ֧�� std::bind �ƶ����ͣ������Ϻ��������β�ͬ����֧��ʡ�Բ�����
			*/
			template<typename _type, typename _func, typename... _tParams>
			void
				CallBinaryFold(_func f, _type val, TermNode& term, _tParams&&... args)
			{
				const auto n(FetchArgumentN(term));
				auto i(term.begin());
				const auto j(leo::make_transform(++i, [](TNIter it) {
					return leo::Access<_type>(Deref(it));
				}));

				leo::EmplaceCallResult(term.Value, std::accumulate(j, std::next(j,
					typename std::iterator_traits<decltype(j)>::difference_type(n)), val,
					leo::bind1(f, std::placeholders::_2, lforward(args)...)));
			}

			/*!
			\brief ���溯��չ�����õĺ�������
			\todo ʹ�� C++1y lambda ���ʽ���档
			
			������Ϊ�����Ĵ������ĳ��������ѡ�����ĺ�������
			Ϊ�ʺ���Ϊ�����Ĵ�������֧�ֵĲ����б�����ʵ�ʴ������ƣ�
			�����б��Ժ�Ԫ����ͬ�����ı���� TermNode& ���͵Ĳ���
			*/
			//@{
			//@{
			//! \sa Forms::CallUnary
			template<typename _func>
			struct UnaryExpansion
			{
				_func Function;

				friend PDefHOp(bool, == , const UnaryExpansion& x, const UnaryExpansion& y)
					ImplRet(leo::examiners::equal_examiner::are_equal(x.Function,
						y.Function))

				template<typename... _tParams>
				void
					operator()(TermNode& term, _tParams&&... args)
				{
					Forms::CallUnary(Function, term, lforward(args)...);
				}
			};


			//! \sa Forms::CallUnaryAs
			template<typename _type, typename _func>
			struct UnaryAsExpansion
			{
				_func Function;

				friend PDefHOp(bool, == , const UnaryAsExpansion& x,
					const UnaryAsExpansion& y)
					ImplRet(leo::examiners::equal_examiner::are_equal(x.Function,
						y.Function))

				template<typename... _tParams>
				void
					operator()(TermNode& term, _tParams&&... args)
				{
					Forms::CallUnaryAs<_type>(Function, term, lforward(args)...);
				}
			};
			//@}


			//@{
			//! \sa Forms::CallBinary
			template<typename _func>
			struct BinaryExpansion
			{
				_func Function;

				/*!
				\brief �Ƚϴ�������ȡ�
				*/
				friend PDefHOp(bool, == , const BinaryExpansion& x, const BinaryExpansion& y)
					ImplRet(leo::examiners::equal_examiner::are_equal(x.Function,
						y.Function))

					template<typename... _tParams>
				inline void
					operator()(_tParams&&... args) const
				{
					Forms::CallBinary(Function, lforward(args)...);
				}
			};


			//! \sa Forms::CallBinaryAs
			template<typename _type, typename _func>
			struct BinaryAsExpansion
			{
				_func Function;

				/*!
				\brief �Ƚϴ�������ȡ�
				*/
				friend PDefHOp(bool, == , const BinaryAsExpansion& x,
					const BinaryAsExpansion& y)
					ImplRet(leo::examiners::equal_examiner::are_equal(x.Function,
						y.Function))

					template<typename... _tParams>
				inline void
					operator()(_tParams&&... args) const
				{
					Forms::CallBinaryAs<_type>(Function, lforward(args)...);
				}
			};
			//@}
			//@}

			/*!
			\brief ע��һԪ�ϸ���ֵ�����Ĵ�������
			*/
			//@{
			template<typename _func>
			void
				RegisterStrictUnary(ContextNode& node, const string& name, _func f)
			{
				RegisterStrict(node, name, UnaryExpansion<_func>{f});
			}
			template<typename _type, typename _func>
			void
				RegisterStrictUnary(ContextNode& node, const string& name, _func f)
			{
				RegisterStrict(node, name, UnaryAsExpansion<_type, _func>{f});
			}
			//@}

			/*!
			\brief ע���Ԫ�ϸ���ֵ�����Ĵ�������
			*/
			//@{
			template<typename _func>
			void
				RegisterStrictBinary(ContextNode& node, const string& name, _func f)
			{
				RegisterStrict(node, name, BinaryExpansion<_func>{f});
			}
			template<typename _type, typename _func>
			void
				RegisterStrictBinary(ContextNode& node, const string& name, _func f)
			{
				RegisterStrict(node, name, BinaryAsExpansion<_type, _func>{f});
			}
			//@}

			/*!
			\brief �Ƴ����ư󶨡�
			\exception BadIdentifier ��ǿ��ʱ�Ƴ������ڵ����ơ�
			\throw InvalidSyntax ��ʶ�����Ƿ��š�
			\sa IsNPLASymbol
			\sa RemoveIdentifier

			�Ƴ����ƺ͹�����ֵ�������Ƿ��Ƴ���
			������������ʾ�Ƿ�ǿ�ơ�����ǿ�ƣ��Ƴ������ڵ������׳��쳣��
			�ο������ķ���
			$undef! <symbol>
			$undef-checked! <symbol>
			*/
			LS_API void
				Undefine(TermNode&, ContextNode&, bool);


			/*
			\pre ��Ӷ��ԣ���һ����ָ��������֦�ڵ㡣
			\note ʵ��������ʽ��
			\throw InvalidSyntax �﷨����
			*/
			//@{
			/*!
			\brief �����жϣ�������ֵ������ȡ���ʽ��
			\sa ReduceChecked

			��ֵ��һ������Ϊ��������������ʱȡ�ڶ�������򵱵�������ʱȡ�������
			������ʽ�ο��ķ���
			$if <test> <consequent> <alternate>
			$if <test> <consequent>
			*/
			LS_API ReductionStatus
				If(TermNode&, ContextNode&);


			/*!
			\exception InvalidSyntax �쳣�������� ExtractParameters �׳���
			\sa EvaluateIdentifier
			\sa ExtractParameters
			\warning ���رհ��������ñ���������Ŀ�������������δ������Ϊ��
			\todo �Ż���������

			ʹ�� ExtractParameters �������б�����Ͱ󶨱�����
			Ȼ�����ýڵ��ֵΪ��ʾ �� ����������Ĵ�������
			��ʹ�� RegisterFormContextHandler ע�������Ĵ�������
			�� Scheme �Ȳ�ͬ�������������λ�õ���ʽ��ת�ƣ��ں���Ӧ��ʱ���ܽ�һ����ֵ��
			�����ò����������еİ󶨡���������������еİ������������Ե������ڹ���
			//@{
			\brief �� ���󣺲���һ������ǰ�����ĵĹ��̡�
			\note ʵ��������ʽ�������������ʽ��ת�ƣ��ں���Ӧ��ʱ���ܽ�һ����ֵ��

			������ʽ�ο��ķ���
			$lambda <formals> <body>
			*/
			LS_API void
				Lambda(TermNode&, ContextNode&);

			/*!
			\brief vau ������ֵΪһ������ǰ�����ĵķ��ϸ���ֵ�ĺ�����
			\note ��̬�����������Ĳ���������Ϊһ�� leo::ref<ContextNode> ����
			\throw InvalidSynta <eformal> ������Ҫ��

			��ʼ���� <eformal> ��ʾ��̬�����������Ĳ�����ӦΪһ�����Ż� #ignore ��
			������ʽ�ο��ķ���
			$vau <formals> <eformal> <body>
			*/
			LS_API void
				Vau(TermNode&, ContextNode&);
			//@}
			//@}


			/*!
			\sa ReduceChecked
			*/
			//@{
			/*!
			\brief �߼��롣

			���ϸ���ֵ���ɸ����������ֵ������߼��룺
			����һ�����û����������ʱ������ true ����������������ֵ���
			������ȫ��ֵΪ true ʱ�������һ�������ֵ�����򷵻� false ��
			������ʽ�ο��ķ���
			$and <test1>...
			*/
			LS_API ReductionStatus
				And(TermNode&, ContextNode&);

			/*!
			\brief �߼���

			���ϸ���ֵ���ɸ����������ֵ������߼���
			����һ�����û����������ʱ������ false ����������������ֵ���
			������ȫ��ֵΪ false ʱ���� false�����򷵻ص�һ������ false �������ֵ��
			������ʽ�ο��ķ���
			$or <test1>...
			*/
			LS_API ReductionStatus
				Or(TermNode&, ContextNode&);
			//@}

			/*!
			\brief ���� UTF-8 �ַ�����ϵͳ��������� int ���͵Ľ�������ֵ�С�
			\sa usystem
			*/
			LS_API void
				CallSystem(TermNode&);


			//@{
			/*!
			\brief �Ƚ����������ֵ��ȡ�
			\sa leo::HoldSame
			*/
			LS_API void
				EqualReference(TermNode&);

			/*!
			\brief �Ƚ����������ֵ��ȡ�
			\sa leo::ValueObject
			*/
			LS_API void
				EqualValue(TermNode&);
			//@}

			//@{
			/*!
			\brief ��ָ���ָ���Ļ�����ֵ��

			�Ա��ʽ <expression> �ͻ��� <environment> Ϊָ���Ĳ���������ֵ��
			������ ContextNode �����ñ�ʾ��
			�ο��ķ���
			eval <expression> <environment>
			*/
			LS_API ReductionStatus
				Eval(TermNode&);
			//@}

			/*!
			\brief ��ֵ��ʶ���õ�ָ�Ƶ�ʵ�塣
			\sa EvaluateIdentifier

			�ڶ���������ʵ�ֺ�������һ�� string ���͵Ĳ��������ֵΪָ����ʵ�塣
			�����Ʋ���ʧ��ʱ�����ص�ֵΪ ValueToken::Null ��
			*/
			LS_API ReductionStatus
				ValueOf(TermNode&, const ContextNode&);


			//@{
			/*!
			\brief ��װ�ϲ���ΪӦ���ӡ�

			�ο������ķ���
			wrap <combiner>
			*/
			LS_API ContextHandler
				Wrap(const ContextHandler&);

			//! \exception LSLException ���Ͳ�����Ҫ��
			//@{
			/*!
			\brief ��װ������ΪӦ���ӡ�

			�ο������ķ���
			wrap1 <operative>
			*/
			LS_API ContextHandler
				WrapOnce(const ContextHandler&);

			/*!
			\brief ���װӦ����Ϊ�ϲ��ӡ�

			�ο������ķ���
			unwrap <applicative>
			*/
			LS_API ContextHandler
				Unwrap(const ContextHandler&);
			//@}
			//@}

		} // namespace Forms;
	} // namspace v1;
} // namespace scheme;

#endif