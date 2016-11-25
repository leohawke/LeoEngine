#include "LScheme.h"
#include "SContext.h"
#include <LBase/scope_gurad.hpp>

using namespace leo;

namespace scheme
{
	namespace v1
	{
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

			// TODO: Use %ReductionStatus as pass invocation result directly instead of this
			//	wrapper. This is not transformed yet to avoid G++ internal compiler error:
			//	"error reporting routines re-entered".
			lconstfn PDefH(ReductionStatus, ToStatus, bool res) lnothrow
				ImplRet(res ? ReductionStatus::NeedRetry : ReductionStatus::Success)

				//! \since build 736
				lconstfn PDefH(bool, NeedsRetry, ReductionStatus res) lnothrow
				ImplRet(res != ReductionStatus::Success)

				//! \since build 736
				template<typename _func>
			TermNode
				TransformForSeparatorTmpl(_func f, const TermNode& term, const ValueObject& pfx,
					const ValueObject& delim, const string& name)
			{
				using namespace std::placeholders;
				auto res(AsNode(name, term.Value));

				if (IsBranch(term))
				{
					res += AsIndexNode(res, pfx);
					leo::split(term.begin(), term.end(), leo::bind1(HasValue,
						std::ref(delim)), std::bind(f, std::ref(res), _1, _2));
				}
				return res;
			}

		} // unnamed namespace;

		GuardPasses&
			AccessGuardPassesRef(ContextNode& ctx)
		{
			return ctx.Place<GuardPasses>(GuardName);
		}

		EvaluationPasses&
			AccessLeafPassesRef(ContextNode& ctx)
		{
			return ctx.Place<EvaluationPasses>(LeafTermName);
		}

		EvaluationPasses&
			AccessListPassesRef(ContextNode& ctx)
		{
			return ctx.Place<EvaluationPasses>(ListTermName);
		}

		LiteralPasses&
			AccessLiteralPassesRef(ContextNode& ctx)
		{
			return ctx.Place<LiteralPasses>(LiteralTermName);
		}

		Guard
			InvokeGuard(TermNode& term, ContextNode& ctx)
		{
			return InvokePasses<GuardPasses>(GuardName, term, ctx);
		}

		ReductionStatus
			InvokeLeaf(TermNode& term, ContextNode& ctx)
		{
			return ToStatus(InvokePasses<EvaluationPasses>(LeafTermName, term, ctx));
		}

		ReductionStatus
			InvokeList(TermNode& term, ContextNode& ctx)
		{
			return ToStatus(InvokePasses<EvaluationPasses>(ListTermName, term, ctx));
		}

		ReductionStatus
			InvokeLiteral(TermNode& term, ContextNode& ctx, string_view id)
		{
			LAssertNonnull(id.data());
			return
				ToStatus(InvokePasses<LiteralPasses>(LiteralTermName, term, ctx, id));
		}


		ReductionStatus
			Reduce(TermNode& term, ContextNode& ctx)
		{
			const auto gd(InvokeGuard(term, ctx));

#ifndef LB_IMPL_MSCPP
			// NOTE: Rewriting loop until the normal form is got.
			return leo::retry_on_cond(leo::bind1(DetectReducible, std::ref(term)),
				[&]() -> ReductionStatus {
				if (IsBranch(term))
				{
					LAssert(term.size() != 0, "Invalid node found.");
					if (term.size() != 1)
						// NOTE: List evaluation.
						return InvokeList(term, ctx);
					else
					{
						// NOTE: List with single element shall be reduced as the
						//	element.
						LiftFirst(term);
						return Reduce(term, ctx);
					}
				}
				else if (!term.Value)
					// NOTE: Empty list.
					term.Value = ValueToken::Null;
				else if (AccessPtr<ValueToken>(term))
					// TODO: Handle special value token.
					;
				else
					return InvokeLeaf(term, ctx);
				// NOTE: Exited loop has produced normal form by default.
				return ReductionStatus::Success;
			}) == ReductionStatus::Success ? ReductionStatus::Success
				: ReductionStatus::NeedRetry;
#else
			auto cond = leo::bind1(DetectReducible, std::ref(term));
			auto f = [&]() -> ReductionStatus {
				if (IsBranch(term))
				{
					LAssert(term.size() != 0, "Invalid node found.");
					if (term.size() != 1)
						// NOTE: List evaluation.
						return InvokeList(term, ctx);
					else
					{
						// NOTE: List with single element shall be reduced as the
						//	element.
						LiftFirst(term);
						return Reduce(term, ctx);
					}
				}
				else if (!term.Value)
					// NOTE: Empty list.
					term.Value = ValueToken::Null;
				else if (AccessPtr<ValueToken>(term))
					// TODO: Handle special value token.
					;
				else
					return InvokeLeaf(term, ctx);
				// NOTE: Exited loop has produced normal form by default.
				return ReductionStatus::Success;
			};

			ReductionStatus res;

			do
				res = f();
			while (cond(res));
			return res == ReductionStatus::Success ? ReductionStatus::Success
				: ReductionStatus::NeedRetry;
#endif
		}

		void
			ReduceArguments(TermNode::Container& con, ContextNode& ctx)
		{
			if (con.size() > 1)
				// NOTE: The order of evaluation is unspecified by the language
				//	specification. It should not be depended on.
				ReduceChildren(std::next(con.begin()), con.end(), ctx);
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
				app_term.SetContent(std::move(closure));
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
				root.Place<size_t>(name),
				AccessGuardPassesRef(root) = [name](TermNode& term, ContextNode& ctx) {
				using leo::pvoid;
				auto& depth(AccessChild<size_t>(ctx, name));

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
				leo::bind1(HasValue, std::ref(delim))) != term.end())
				term = TransformForSeparator(term, name, delim, term.GetName());
			return ReductionStatus::Success;
		}


		void
			FormContextHandler::operator()(TermNode& term, ContextNode& ctx) const
		{
			// TODO: Is it worth matching specific builtin special forms here?
			try
			{
				if (!Check || Check(term))
					Handler(term, ctx);
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
		}


		void
			FunctionContextHandler::operator()(TermNode& term, ContextNode& ctx) const
		{
			auto& con(term.GetContainerRef());

			// NOTE: This implementes arguments evaluation in applicative order.
			ReduceArguments(con, ctx);

			const auto n(con.size());

			if (n > 1)
			{
				// NOTE: Matching function calls.
				auto i(con.begin());

				// NOTE: Adjust null list argument application
				//	to function call without arguments.
				// TODO: Improve performance of comparison?
				if (n == 2 && Deref(++i).Value == ValueToken::Null)
					con.erase(i);
				Handler(term, ctx);
			}
			else
				// TODO: Use other exception type for this type of error?
				// TODO: Capture contextual information in error.
				throw ListReductionFailure(leo::sfmt("Invalid list form with"
					" %zu term(s) not reduced found.", n), leo::Warning);
			// TODO: Add unreduced form check? Is this better to be inserted in other
			//	passes?
#if false
			if (con.empty())
				TraceDe(Warning, "Empty reduced form found.");
			else
				TraceDe(Warning, "%zu term(s) not reduced found.", n);
#endif
		}


		void
			RegisterSequenceContextTransformer(EvaluationPasses& passes, ContextNode& node,
				const string& name, const ValueObject& delim)
		{
			// TODO: Simplify by using %ReductionStatus as invocation result directly.
			//	passes += leo::bind1(ReplaceSeparatedChildren, name, delim);
			passes += [name, delim](TermNode& term) {
				return NeedsRetry(ReplaceSeparatedChildren(term, name, delim));
			};
			RegisterFormContextHandler(node, name, [](TermNode& term, ContextNode& ctx) {
				RemoveHead(term);
				ReduceChildren(term, ctx);
			}, IsBranch);
		}


		ReductionStatus
			EvaluateContextFirst(TermNode& term, ContextNode& ctx)
		{
			if (IsBranch(term))
			{
				const auto& fm(Deref(leo::as_const(term).begin()));

				if (const auto p_handler = AccessPtr<ContextHandler>(fm))
					(*p_handler)(term, ctx);
				else
				{
					const auto p(AccessPtr<string>(fm));

					// TODO: Capture contextual information in error.
					throw ListReductionFailure(leo::sfmt("No matching form '%s'"
						" with %zu argument(s) found.", p ? p->c_str()
						: "#<unknown>", term.size()));
				}
			}
			return ReductionStatus::Success;
		}

		ReductionStatus
			EvaluateIdentifier(TermNode& term, ContextNode& ctx, string_view id)
		{
			LAssertNonnull(id.data());

			if (auto v = FetchValue(ctx, id))
			{
				term.Value = std::move(v);
				if (const auto p_handler = AccessPtr<LiteralHandler>(term))
					return ToStatus((*p_handler)(ctx));
			}
			else
				throw BadIdentifier(id);
			if (const auto p = AccessPtr<TermNode>(term))
			{
				term.SetContent(std::move(*p));
				// NOTE: To make it work with %DetectReducible.
				if (IsBranch(term))
					return ReductionStatus::NeedRetry;
			}
			return ReductionStatus::Success;
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
				const auto lcat(CategorizeLiteral(id));

				switch (lcat)
				{
				case LiteralCategory::Code:
					// TODO: When do code literals need to be evaluated?
					id = DeliteralizeUnchecked(id);
					if (LB_UNLIKELY(id.empty()))
						break;
				case LiteralCategory::None:
					return InvokeLiteral(term, ctx, id) == ReductionStatus::Success
						? ReductionStatus::Success : EvaluateIdentifier(term, ctx, id);
					// XXX: Empty token is ignored.
					// XXX: Remained reducible?
				case LiteralCategory::Data:
					term.Value = Deliteralize(id).to_string();
				default:
					break;
					// TODO: Handle other categories of literal.
				}
			}
			return ReductionStatus::Success;
		}

		ReductionStatus
			ReduceLeafToken(TermNode& term, ContextNode& ctx)
		{
			return leo::call_value_or([&](string_view id) -> ReductionStatus {
				return EvaluateLeafToken(term, ctx, id);
				// FIXME: Success on node conversion failure?
			}, TermToName(term), ReductionStatus::Success);
		}

		void
			SetupDefaultInterpretation(ContextNode& root, EvaluationPasses passes)
		{
			// TODO: Simplify by using %ReductionStatus as invocation result directly
			//	in leo. Otherwise current versions of G++ would crash here as internal
			//	compiler error: "error reporting routines re-entered".
			//	passes += ReduceFirst;
			//passes += leo::compose(NeedsRetry, ReduceFirst);
			passes += [](TermNode& term, ContextNode& ctx)->bool {return NeedsRetry(ReduceFirst(term, ctx));};
			// TODO: Insert more form evaluation passes: macro expansion, etc.
			//	passes += EvaluateContextFirst;
			//passes += leo::compose(NeedsRetry, EvaluateContextFirst);
			passes += [](TermNode& term, ContextNode& ctx)->bool {return NeedsRetry(EvaluateContextFirst(term, ctx)); };
			AccessListPassesRef(root) = std::move(passes);
			//	AccessLeafPassesRef(root) = ReduceLeafToken;
			//AccessLeafPassesRef(root) = leo::compose(NeedsRetry, ReduceLeafToken);
			AccessLeafPassesRef(root) = [](TermNode& term, ContextNode& ctx)->bool {return NeedsRetry(ReduceLeafToken(term, ctx)); };
		}

		namespace Forms
		{

			size_t
				QuoteN(const TermNode& term, size_t m)
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
					if (const auto p = TermToName(Deref(i)))
					{
						if (*p == mod)
						{
							con.erase(i);
							return true;
						}
					}
				}
				return{};
			}


			void
				DefineOrSet(TermNode& aterm, ContextNode& actx, bool define)
			{
				ReduceWithModifier(aterm, actx,
					[=](TermNode& term, ContextNode& ctx, bool mod) {
					auto& con(term.GetContainerRef());

					auto i(con.begin());

					++i;
					if (!i->empty())
					{
						const auto i_beg(i->begin());

						if (const auto p_id = TermToName(Deref(i_beg)))
						{
							const auto id(*p_id);

							i->GetContainerRef().erase(i_beg);
							Lambda(term, ctx);
							DefineOrSetFor(id, term, ctx, define, mod);
						}
						else
							throw LSLException("Invalid node category found.");
					}
					else if (const auto p_id = TermToName(Deref(i)))
					{
						const auto id(*p_id);

						TraceDe(Debug, "Found identifier '%s'.", id.c_str());
						if (++i != con.end())
						{
							CheckedReduceWith(ReduceTail, term, ctx, i);
							DefineOrSetFor(id, term, ctx, define, mod);
						}
						else if (define)
							RemoveIdentifier(ctx, id, mod);
						else
							throw InvalidSyntax("Source operand not found.");
						term.ClearTo(ValueToken::Unspecified);
					}
					else
						throw LSLException("Invalid node category found.");
				});
			}

			void
				DefineOrSetFor(const string& id, TermNode& term, ContextNode& ctx, bool define,
					bool mod)
			{
				if (CategorizeLiteral(id) == LiteralCategory::None)
					// XXX: Moved.
					// NOTE: Unevaluated term is directly saved.
					(define ? DefineValue : RedefineValue)
					(ctx, id, std::move(term), mod);
				else
					throw InvalidSyntax(define ? "Literal cannot be defined."
						: "Literal cannot be set.");
			}

			shared_ptr<vector<string>>
				ExtractLambdaParameters(const TermNode::Container& con)
			{
				TraceDe(Debug, "Found lambda abstraction form with %zu"
					" parameter(s) to be bound.", con.size());

				// TODO: Blocked. Use C++14 lambda initializers to reduce
				//	initialization cost by directly moving.
				auto p_params(make_shared<vector<string>>());
				set<string_view> svs;

				// TODO: Simplify?
				std::for_each(con.begin(), con.end(), [&](decltype(*con.begin()) pv) {
					const auto& name(Access<string>(pv));

					// FIXME: Missing identifier syntax check.
					// TODO: Throw %InvalidSyntax for invalid syntax.
					if (svs.insert(name).second)
						p_params->push_back(name);
					else
						throw InvalidSyntax(
							sfmt("Duplicate parameter name '%s' found.", name.c_str()));
				});
				return p_params;
			}

			void
				Lambda(TermNode& term, ContextNode& ctx)
			{
				auto& con(term.GetContainerRef());
				auto size(con.size());

				LAssert(size != 0, "Invalid term found.");
				if (size > 1)
				{
					auto i(con.begin());
					const auto p_params(ExtractLambdaParameters((++i)->GetContainer()));

					con.erase(con.cbegin(), ++i);

					// TODO: Optimize. This does not need to be shared, since it would
					//	always be copied, if used.
					const auto p_ctx(make_shared<ContextNode>(ctx));
					const auto p_closure(make_shared<TermNode>(std::move(con),
						term.GetName(), std::move(term.Value)));

					// FIXME: Cyclic reference to '$lambda' context handler when the
					//	term value (i.e. the closure) is copied upward?
					term.Value = ToContextHandler([=](TermNode& app_term) {
						auto& params(Deref(p_params));
						const auto n_params(params.size());
						const auto n_terms(app_term.size());

						TraceDe(Debug, "Lambda called, with %ld shared term(s), %ld shared"
							" context(s), %zu parameter(s).", p_closure.use_count(),
							p_ctx.use_count(), n_params);
						if (n_terms == 0)
							throw LoggedEvent("Invalid application found.", Alert);

						const auto n_args(n_terms - 1);

						if (n_args == n_params)
						{
							auto j(app_term.begin());
							// TODO: Optimize for performance.
							// NOTE: This is probably better to be copy-on-write. Since
							//	no immutable reference would be accessed before
							//	mutation, no care is needed for reference invalidation.
							auto app_ctx(Deref(p_ctx));

							++j;
							// NOTE: Introduce parameters as per lexical scoping rules.
							for (const auto& param : params)
							{
								// XXX: Moved.
								// NOTE: Unevaluated operands are directly saved.
								app_ctx[param].Value = std::move(*j);
								++j;
							}
							LAssert(j == app_term.end(),
								"Invalid state found on passing arguments.");
							// NOTE: Beta reduction.
							ReduceCheckedClosure(app_term, app_ctx, p_closure.unique(),
								*p_closure);
						}
						else
							throw ArityMismatch(n_params, n_args);
					});
					con.clear();
				}
				else
					throw InvalidSyntax("Syntax error in lambda abstraction.");
			}

		} // namespace Forms;

	} // namesapce v1;

} // namespace scheme;