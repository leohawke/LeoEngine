/*!	\file LScheme.cpp
\ingroup LSL
\par 修改时间:
2017-03-27 15:23 +0800
*/

#include "LScheme.h"
#include "LSchemREPL.h"
#include "SContext.h"
#include <LBase/scope_gurad.hpp>

using namespace leo;

namespace scheme
{
#define LS_Impl_LSLV1_Enable_Thunked true

	namespace v1
	{
		string
			to_string(ValueToken vt)
		{
			switch (vt)
			{
			case ValueToken::Null:
				return "null";
			case ValueToken::Undefined:
				return "undefined";
			case ValueToken::Unspecified:
				return "unspecified";
			case ValueToken::GroupingAnchor:
				return "grouping";
			case ValueToken::OrderedAnchor:
				return "ordered";
			}
			throw std::invalid_argument("Invalid value token found.");
		}


		void
			InsertChild(TermNode&& term, TermNode::Container& con)
		{
			con.insert(term.GetName().empty() ? AsNode('$' + MakeIndex(con),
				std::move(term.Value)) : std::move(MapToValueNode(term)));
		}

		void
			InsertSequenceChild(TermNode&& term, NodeSequence& con)
		{
			con.emplace_back(std::move(MapToValueNode(term)));
		}

		ValueNode
			TransformNode(const TermNode& term, NodeMapper mapper, NodeMapper map_leaf_node,
				NodeToString node_to_str, NodeInserter insert_child)
		{
			auto s(term.size());

			if (s == 0)
				return map_leaf_node(term);

			auto i(term.begin());
			const auto nested_call(leo::bind1(TransformNode, mapper, map_leaf_node,
				node_to_str, insert_child));

			if (s == 1)
				return nested_call(*i);

			const auto& name(node_to_str(*i));

			if (!name.empty())
				lunseq(++i, --s);
			if (s == 1)
			{
				auto&& nd(nested_call(*i));

				if (nd.GetName().empty())
					return AsNode(name, std::move(nd.Value));
				return{ ValueNode::Container{ std::move(nd) }, name };
			}

			ValueNode::Container node_con;

			std::for_each(i, term.end(), [&](const TermNode& tm) {
				insert_child(mapper ? mapper(tm) : nested_call(tm), node_con);
			});
			return{ std::move(node_con), name };
		}

		ValueNode
			TransformNodeSequence(const TermNode& term, NodeMapper mapper, NodeMapper
				map_leaf_node, NodeToString node_to_str, NodeSequenceInserter insert_child)
		{
			auto s(term.size());

			if (s == 0)
				return map_leaf_node(term);

			auto i(term.begin());
			auto nested_call(leo::bind1(TransformNodeSequence, mapper,
				map_leaf_node, node_to_str, insert_child));

			if (s == 1)
				return nested_call(*i);

			const auto& name(node_to_str(*i));

			if (!name.empty())
				lunseq(++i, --s);
			if (s == 1)
			{
				auto&& n(nested_call(*i));

				return AsNode(name, n.GetName().empty() ? std::move(n.Value)
					: ValueObject(NodeSequence{ std::move(n) }));
			}

			NodeSequence node_con;

			std::for_each(i, term.end(), [&](const TermNode& tm) {
				insert_child(mapper ? mapper(tm) : nested_call(tm), node_con);
			});
			return AsNode(name, std::move(node_con));
		}


		namespace
		{

			lconstexpr const auto GuardName("__$!");
			lconstexpr const auto LeafTermName("__$$@");
			lconstexpr const auto ListTermName("__$$");
			lconstexpr const auto LiteralTermName("__$$@_");
			lconstexpr const auto ParentContextName("__$@_parent");


				template<typename _func>
			TermNode
				TransformForSeparatorTmpl(_func f, const TermNode& term, const ValueObject& pfx,
					const ValueObject& delim, const string& name)
			{
				using namespace std::placeholders;
				// NOTE: Explicit type 'TermNode' is intended.
				TermNode res(AsNode(name, term.Value));

				if (IsBranch(term))
				{
					// NOTE: Explicit type 'TermNode' is intended.
					res += TermNode(AsIndexNode(res, pfx));
					leo::split(term.begin(), term.end(),
						leo::bind1(HasValue<ValueObject>, std::ref(delim)),
						std::bind(f, std::ref(res), _1, _2));
				}
				return res;
			}

			ReductionStatus
				AndOr(TermNode& term, ContextNode& ctx, bool is_and)
			{
				Forms::Retain(term);

				auto i(term.begin());

				if (++i != term.end())
				{
					if (std::next(i) == term.end())
						LiftTerm(term, *i);
					else
					{
						ReduceChecked(*i, ctx);
						if (leo::value_or(i->Value.AccessPtr<bool>(), is_and) == is_and)
							term.Remove(i);
						else
						{
							if (is_and)
								term.Value = false;
							else
								LiftTerm(term, i->Value);
							term.Remove(i);
							return ReductionStatus::Clean;
						}
					}
					return ReductionStatus::Retrying;
				}
				term.Value = is_and;
				return ReductionStatus::Clean;
			}

			template<typename _func>
			void
				EqualTerm(TermNode& term, _func f)
			{
				Forms::RetainN(term, 2);

				auto i(term.begin());
				const auto& x(Deref(++i));

				term.Value = f(x.Value, Deref(++i).Value);
			}

			class VauHandler final
			{
			private:
				/*!
				\brief 动态上下文名称。
				*/
				string eformal{};
				/*!
				\brief 形式参数对象。
				*/
				shared_ptr<TermNode> p_formals;
				//! \brief 捕获静态上下文，包含引入抽象时的静态环境。
				shared_ptr<ContextNode> p_context;
				//! \brief 闭包对象。
				shared_ptr<TermNode> p_closure;

			public:
				VauHandler(TermNode& term, ContextNode& ctx, bool ignore)
					: p_formals([&] {
					using namespace Forms;

					Retain(term);
					if (term.size() > 2)
					{
						auto& con(term.GetContainerRef());
						auto i(con.begin());
						const auto& formals(Deref(++i));

						if (!ignore)
						{
							const auto& eterm(Deref(++i));

							if (const auto p = TermToNamePtr(eterm))
							{
								eformal = *p;
								TraceDe(Debug, "Found context parameter name '%s'.",
									eformal.c_str());
								if (eformal == "#ignore")
									eformal.clear();
								else if (!IsLSLASymbol(eformal))
									throw InvalidSyntax("Symbol or '#ignore' expected"
										" for context parameter.");
							}
							else
								throw InvalidSyntax("Invalid context parameter found.");
						}
						TraceDe(Debug, "Found operator with %zu parameter(s) to be"
							" bound.", formals.size());
						auto res(make_shared<TermNode>(std::move(formals)));

						con.erase(con.cbegin(), ++i);
						return res;
					}
					else
						throw InvalidSyntax(
							"Insufficient terms in function abstraction.");
				}()),
					// NOTE: Capturing by reference.
					// TODO: Optimize. This does not need to be shared, since it would
					//	always be copied, if used.
					// TODO: Region inference?
					// FIXME: This may be unsafe if the external owner is destroyed.
					p_context(make_shared<ContextNode>(ctx)),
					p_closure(make_shared<TermNode>(std::move(term)))
				{}

				ReductionStatus
					operator()(TermNode& term, ContextNode& ctx) const
				{
					if (IsBranch(term))
					{
						// FIXME: Cyclic reference to context handler when the term value
						//	(i.e. the closure) is copied upward?
						using namespace Forms;
						const auto& formals(Deref(p_formals));
						// NOTE: Active record frame with outer scope bindings.
						// TODO: Optimize for performance.
						// NOTE: This is probably better to be copy-on-write. Since
						//	no immutable reference would be accessed before
						//	mutation, no care is needed for reference invalidation.
						// TODO: Optimize using initialization from iterator pair?
						// XXX: Referencing escaped variables (now only parameters need
						//	to be cared) form the context would cause undefined behavior
						//	(e.g. returning a reference to automatic object in the host
						//	language).
						// TODO: Reduce such undefined behavior resonably?
						ContextNode comp_ctx;

						// NOTE: Bind dynamic context.
						if (!eformal.empty())
							comp_ctx.GetBindingsRef().AddValue(eformal, ValueObject(ctx, OwnershipTag<>()));
						// NOTE: Since first term is expected to be saved (e.g. by
						//	%ReduceCombined), it is safe to reduce directly.
						RemoveHead(term);
						BindParameter(comp_ctx, formals, term);
						TraceDe(Debug, "Function called, with %ld shared term(s), %ld"
							" shared context(s), %zu parameter(s).", p_closure.use_count(),
							p_context.use_count(), formals.size());
						LAssert(&comp_ctx != &Deref(p_context),
							"Self reference of context found.");
						// NOTE: Static context is bound by setting parent context pointer.
						// NOTE: Shared ownership is necessary here to prevent the context
						//	disposed too early after the vau handler has been destroyed. The
						//	context has to live longer if there exists the child to capture
						//	the context and then return. And there cannot be cyclic
						//	reference.
						comp_ctx.GetBindingsRef().AddValue(ParentContextName, p_context);
						// NOTE: Beta reduction.
						// TODO: Implement accurate lifetime analysis rather than
						//	'p_closure.unique()'.
						ReduceCheckedClosure(term, comp_ctx, {}, *p_closure);
						return CheckNorm(term);
					}
					else
						throw LoggedEvent("Invalid composition found.", Alert);
				}
			};


		} // unnamed namespace;

		ReductionStatus
			Reduce(TermNode& term, ContextNode& ctx)
		{
#if LS_Impl_LSLV1_Enable_Thunked
			// TODO: Support other states?
			leo::swap_guard<Reducer> gd(true, ctx.Current);
			leo::swap_guard<bool> gd_skip(true, ctx.SkipToNextEvaluation);

#endif
			return ctx.RewriteGuarded(term,
				std::bind(ReduceOnce, std::ref(term), std::ref(ctx)));
		}

		void
			ReduceArguments(TNIter first, TNIter last, ContextNode& ctx)
		{
			if (first != last)
				// NOTE: The order of evaluation is unspecified by the language
				//	specification. It should not be depended on.
				ReduceChildren(++first, last, ctx);
			else
				throw InvalidSyntax("Argument not found.");
		}

		void
			ReduceChecked(TermNode& term, ContextNode& ctx)
		{
			CheckedReduceWith(Reduce, term, ctx);
		}

		void
			ReduceCheckedClosure(TermNode& term, ContextNode& ctx, bool move,
				TermNode& closure)
		{
			TermNode app_term(NoContainer, term.GetName());

			if (move)
				LiftTerm(app_term, closure);
			else
				app_term.SetContent(closure);
			// TODO: Test for normal form?
			// XXX: Term reused.
			ReduceChecked(app_term, ctx);
			term.SetContent(std::move(app_term));
		}

		void
			ReduceChildren(TNIter first, TNIter last, ContextNode& ctx)
		{
			// NOTE: Separators or other sequence constructs are not handled here. The
			//	evaluation can be potentionally parallel, though the simplest one is
			//	left-to-right.
			// TODO: Use %ExecutionPolicy?
			std::for_each(first, last, leo::bind1(Reduce, std::ref(ctx)));
		}

		ReductionStatus
			ReduceChildrenOrdered(TNIter first, TNIter last, ContextNode& ctx)
		{
			const auto tr([&](TNIter iter) {
				return leo::make_transform(iter, [&](TNIter i) {
					return Reduce(*i, ctx);
				});
			});

			return leo::default_last_value<ReductionStatus>()(tr(first), tr(last),
				ReductionStatus::Clean);
		}

		ReductionStatus
			ReduceFirst(TermNode& term, ContextNode& ctx)
		{
			return IsBranch(term) ? Reduce(Deref(term.begin()), ctx)
				: ReductionStatus::Clean;
		}

		ReductionStatus
			ReduceOrdered(TermNode& term, ContextNode& ctx)
		{
			const auto res(ReduceChildrenOrdered(term, ctx));

			if (IsBranch(term))
				LiftTerm(term, *term.rbegin());
			return res;
		}

		ReductionStatus
			ReduceTail(TermNode& term, ContextNode& ctx, TNIter i)
		{
			auto& con(term.GetContainerRef());

			con.erase(con.begin(), i);
			return Reduce(term, ctx);
		}


		void
			SetupTraceDepth(ContextNode& root, const string& name)
		{
			lunseq(
				root.GetBindingsRef().Place<size_t>(name),
				AccessGuardPassesRef(root) = [name](TermNode& term, ContextNode& ctx) {
				using leo::pvoid;
				auto& depth(AccessChild<size_t>(ctx.GetBindingsRef(), name));

				TraceDe(Informative, "Depth = %zu, context = %p, semantics = %p.",
					depth, pvoid(&ctx), pvoid(&term));
				++depth;
				return leo::unique_guard([&]() lnothrow{
					--depth;
				});
			}
			);
		}


		TermNode
			TransformForSeparator(const TermNode& term, const ValueObject& pfx,
				const ValueObject& delim, const string& name)
		{
			return TransformForSeparatorTmpl([&](TermNode& res, TNCIter b, TNCIter e) {
				auto child(AsIndexNode(res));

				while (b != e)
				{
					child += {b->GetContainer(), MakeIndex(child), b->Value};
					++b;
				}
				res += std::move(child);
			}, term, pfx, delim, name);
		}

		TermNode
			TransformForSeparatorRecursive(const TermNode& term, const ValueObject& pfx,
				const ValueObject& delim, const string& name)
		{
			return TransformForSeparatorTmpl([&](TermNode& res, TNCIter b, TNCIter e) {
				while (b != e)
					res += TransformForSeparatorRecursive(*b++, pfx, delim,
						MakeIndex(res));
			}, term, pfx, delim, name);
		}

		ReductionStatus
			ReplaceSeparatedChildren(TermNode& term, const ValueObject& name,
				const ValueObject& delim)
		{
			if (std::find_if(term.begin(), term.end(),
				leo::bind1(HasValue<ValueObject>, std::ref(delim))) != term.end())
				term = TransformForSeparator(term, name, delim, 
					TokenValue(term.GetName()));
			return ReductionStatus::Clean;
		}


		ReductionStatus
			FormContextHandler::operator()(TermNode& term, ContextNode& ctx) const
		{
			// TODO: Is it worth matching specific builtin special forms here?
			try
			{
				if (!Check || Check(term))
					return Handler(term, ctx);
				else
					// TODO: Use more specific exception type?
					throw std::invalid_argument("Term check failed.");
			}
			CatchExpr(LSLException&, throw)
				// TODO: Use semantic exceptions.
				CatchThrow(leo::bad_any_cast& e, LoggedEvent(
					leo::sfmt("Mismatched types ('%s', '%s') found.",
						e.from(), e.to()), Warning))
				// TODO: Use nested exceptions?
				CatchThrow(std::exception& e, LoggedEvent(e.what(), Err))
				// XXX: Use distinct status for failure?
				return ReductionStatus::Clean;
		}


		ReductionStatus
			StrictContextHandler::operator()(TermNode& term, ContextNode& ctx) const
		{
			// NOTE: This implementes arguments evaluation in applicative order.
			ReduceArguments(term, ctx);
			LAssert(IsBranch(term), "Invalid state found.");
			// NOTE: Matching function calls.
			return Handler(term, ctx);
#if false
			// TODO: Use other exception type with more precise information for this
			//	error? Also consider capture of contextual information in error.
			throw ListReductionFailure(leo::sfmt("Invalid list form with"
				" %zu term(s) not reduced found.", n), leo::Warning);
#endif
		}


		void
			RegisterSequenceContextTransformer(EvaluationPasses& passes, ContextNode& node,
				const string& name, const ValueObject& delim, bool ordered)
		{
			// TODO: Simplify by using %ReductionStatus as invocation result directly.
			//	passes += leo::bind1(ReplaceSeparatedChildren, name, delim);
			passes += [name, delim](TermNode& term) {
				return ReplaceSeparatedChildren(term, name, delim);
			};
			RegisterForm(node, name,
				ordered ? ReduceOrdered : [](TermNode& term, ContextNode& ctx) {
				ReduceChildren(term, ctx);
				return ReductionStatus::Retained;
			});
		}

		ReductionStatus
			EvaluateDelayed(TermNode& term)
		{
			return leo::call_value_or([&](DelayedTerm& delayed) {
				return EvaluateDelayed(term, delayed);
			}, AccessPtr<DelayedTerm>(term), ReductionStatus::Clean);
		}
		ReductionStatus
			EvaluateDelayed(TermNode& term, DelayedTerm& delayed)
		{
			// NOTE: The referenced term is lived through the envaluation, which is
			//	guaranteed by the evaluated parent term.
			LiftDelayed(term, delayed);
			// NOTE: To make it work with %DetectReducible.
			return ReductionStatus::Retrying;
		}
	

		ReductionStatus
			EvaluateIdentifier(TermNode& term,const ContextNode& ctx, string_view id)
		{
			LAssertNonnull(id.data());
			if (const auto p = ResolveName(ctx, id))
			{
				// NOTE: The referenced term is lived through the envaluation, which is
				//	guaranteed by the context.
				if (p->empty())
					LiftTermRef(term, p->Value);
				else
					// XXX: Children are copied.
					term.SetContentIndirect(p->GetContainer(), p->Value);
				if (const auto p_handler = AccessPtr<LiteralHandler>(term))
					return (*p_handler)(ctx);
				// NOTE: Unevaluated term shall be detected and evaluated. See also
				//	$2017-02 @ %Documentation::Workflow::Annual2017.
				return IsLeaf(term) ? (term.Value.type()
					!= leo::type_id<TokenValue>() ? EvaluateDelayed(term)
					: ReductionStatus::Retrying) : ReductionStatus::Retained;
			}
			throw BadIdentifier(id);
		}

		ReductionStatus
			EvaluateLeafToken(TermNode& term, ContextNode& ctx, string_view id)
		{
			LAssertNonnull(id.data());
			// NOTE: Only string node of identifier is tested.
			if (!id.empty())
			{
				// NOTE: If necessary, there can be inserted some cleanup to remove
				//	empty tokens, returning %ReductionStatus::NeedRetr. Separators
				//	should have been handled in appropriate preprocess passes.
				const auto lcat(CategorizeBasicLexeme(id));

				switch (lcat)
				{
				case LexemeCategory::Code:
					// TODO: When do code literals need to be evaluated?
					id = DeliteralizeUnchecked(id);
					if (LB_UNLIKELY(id.empty()))
						break;
				case LexemeCategory::Symbol:
					return CheckReducible(InvokeLiteral(term, ctx, id))
						? EvaluateIdentifier(term, ctx, id) : ReductionStatus::Clean;
					// XXX: Empty token is ignored.
					// XXX: Remained reducible?
				case LexemeCategory::Data:
					// XXX: This should be prevented being passed to second pass in
					//	%TermToNamePtr normally. This is guarded by normal form handling
					//	in the loop in %Reduce.
					term.Value.emplace<string>(Deliteralize(id));
				default:
					break;
					// TODO: Handle other categories of literal.
				}
			}
			return ReductionStatus::Clean;
		}

		ReductionStatus
			ReduceCombined(TermNode& term, ContextNode& ctx)
		{
			if (IsBranch(term))
			{
				const auto& fm(Deref(leo::as_const(term).begin()));

				if (const auto p_handler = AccessPtr<ContextHandler>(fm))
				{
					const auto handler(std::move(*p_handler));
					const auto res(handler(term, ctx));

					// NOTE: Normalization: Cleanup if necessary.
					if (res == ReductionStatus::Clean)
						term.ClearContainer();
					return res;
				}
				// TODO: Capture contextual information in error.
				// TODO: Extract general form information extractor function.
				throw ListReductionFailure(
					sfmt("No matching combiner '%s' for operand with %zu argument(s)"
						" found.", [&](observer_ptr<const string> p) {
					return
						p ? *p : sfmt("#<unknown:%s>", fm.Value.type().name());
				}(TermToNamePtr(fm)).c_str(), FetchArgumentN(term)));
			}
			return ReductionStatus::Clean;
		}

		ReductionStatus
			ReduceLeafToken(TermNode& term, ContextNode& ctx)
		{
			return leo::call_value_or([&](string_view id) -> ReductionStatus {
				return EvaluateLeafToken(term, ctx, id);
				// FIXME: Success on node conversion failure?
			}, TermToNamePtr(term), ReductionStatus::Clean);
		}

		void
			SetupDefaultInterpretation(ContextNode& root, EvaluationPasses passes)
		{
			passes += ReduceHeadEmptyList;
			passes += [](TermNode& term, ContextNode& ctx)->ReductionStatus {return ReduceFirst(term, ctx);};
			passes += [](TermNode& term, ContextNode& ctx)->ReductionStatus {return ReduceCombined(term, ctx); };
			AccessListPassesRef(root) = std::move(passes);
			AccessLeafPassesRef(root) = [](TermNode& term, ContextNode& ctx)->ReductionStatus {return ReduceLeafToken(term, ctx); };
		}


		REPLContext::REPLContext(bool trace)
		{
			using namespace std::placeholders;

			SetupDefaultInterpretation(Root,
				std::bind(std::ref(ListTermPreprocess), _1, _2));
			if (trace)
				SetupTraceDepth(Root);
		}


		namespace Forms
		{
			size_t
				RetainN(const TermNode& term, size_t m)
			{
				const auto n(FetchArgumentN(term));

				if (n != m)
					throw ArityMismatch(m, n);
				return n;
			}

			bool
				ExtractModifier(TermNode::Container& con, const ValueObject& mod)
			{
				LAssert(!con.empty(), "Empty node container found.");
				if (con.size() > 1)
				{
					const auto i(std::next(con.cbegin()));

					// XXX: Modifier is treated as special name.
					if (const auto p = TermToNamePtr(Deref(i)))
						if (*p == mod)
						{
							con.erase(i);
							return true;
						}
				}
				return{};
			}

			ReductionStatus
				If(TermNode& term, ContextNode& ctx)
			{
				const auto size(term.size());

				if (size == 3 || size == 4)
				{
					auto i(term.begin());

					ReduceChecked(Deref(++i), ctx);
					if (!leo::value_or(i->Value.AccessPtr<bool>()))
						++i;
					if (++i != term.end())
					{
						LiftTerm(term, *i);
						return ReductionStatus::Retrying;
					}
				}
				else
					throw InvalidSyntax("Syntax error in conditional form.");
				return ReductionStatus::Clean;
			}

			
			void
				Lambda(TermNode& term, ContextNode& ctx)
			{
				// NOTE: %ToContextHandler implies strict evaluation of arguments in
				//	%StrictContextHandler::operator().
				term.Value = ToContextHandler(VauHandler(term, ctx, true));
			}

			void
				Vau(TermNode& term, ContextNode& ctx)
			{
				term.Value = ContextHandler(FormContextHandler(VauHandler(term, ctx, {})));
			}


			ReductionStatus
				And(TermNode& term, ContextNode& ctx)
			{
				return AndOr(term, ctx, true);
			}

			ReductionStatus
				Or(TermNode& term, ContextNode& ctx)
			{
				return AndOr(term, ctx, {});
			}

			void
				CallSystem(TermNode& term)
			{
				CallUnaryAs<const string>(leo::compose(usystem, std::mem_fn(&string::c_str)), term);
			}

			void
				EqualReference(TermNode& term)
			{
				EqualTerm(term, leo::HoldSame);
			}

			void
				EqualValue(TermNode& term)
			{
				EqualTerm(term, leo::equal_to<>());
			}

			ReductionStatus
				Eval(TermNode& term)
			{
				RetainN(term, 2);

				const auto i(std::next(term.begin()));
				auto& ctx(Access<ContextNode>(Deref(std::next(i))));

				LiftTerm(term, Deref(i));
				return Reduce(term, ctx);
			}

			void
				EvaluateUnit(TermNode& term, const REPLContext& ctx)
			{
				CallUnaryAs<const string>([ctx](const string& unit) {
					REPLContext(ctx).Perform(unit);
				}, term);
			}

			ReductionStatus
				ValueOf(TermNode& term, const ContextNode& ctx)
			{
				RetainN(term);
				LiftTerm(term, Deref(std::next(term.begin())));
				if (const auto p_id = AccessPtr<string>(term))
					TryRet(EvaluateIdentifier(term, ctx, *p_id))
					CatchIgnore(BadIdentifier&)
					term.Value = ValueToken::Null;
				return ReductionStatus::Clean;
			}

		} // namespace Forms;

	} // namesapce v1;

} // namespace scheme;