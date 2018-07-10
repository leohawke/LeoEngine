/*!	\file LBuilder.cpp
\ingroup LTest
\brief LSL 解释实现。
\par 修改时间:
	2016-11-18 15:44 +0800
*/


#include "LBuilder.h"
#include <streambuf>
#include <sstream>
#include <iostream>
#include <fstream>
#include <typeindex>
//#include YFM_LSL_Configuration
//#include YFM_Helper_Initialization
#include <LFramework/LCLib/Debug.h>
#include <LFramework/Win32/LCLib/Mingw32.h>
#include <LFramework/Helper/GUIApplication.h>
//#include YFM_YSLib_Service_TextFile
#include <LFramework/Win32/LCLib/NLS.h>
#include <LBase/scope_gurad.hpp>

namespace leo
{

}

namespace scheme
{

	namespace v1
	{

		void
			RegisterLiteralSignal(ContextNode& node, const string& name, SSignal sig)
		{
			RegisterLiteralHandler(node, name,
				[=](const ContextNode&) LB_ATTR(noreturn) -> ReductionStatus {
				throw sig;
			});
		}

	} // namespace A1;

} // namespace LSL;

using namespace scheme;
using namespace v1;
using namespace leo;
using namespace platform_ex;

namespace
{

	/// 327
	void
		ParseOutput(LexicalAnalyzer& lex)
	{
		const auto& cbuf(lex.GetBuffer());
		const auto xlst(lex.Literalize());
		using namespace std;
		const auto rlst(Tokenize(xlst));

		cout << "cbuf size:" << cbuf.size() << endl
			<< "xlst size:" << cbuf.size() << endl;
		for (const auto& str : rlst)
			cout << EncodeArg(str) << endl
			<< "* u8 length: " << str.size() << endl;
		cout << rlst.size() << " token(s) parsed." << endl;
	}

	/// 737
	void
		ParseStream(std::istream& is)
	{
		if (is)
		{
			Session sess;
			char c;

			while ((c = is.get()), is)
				Session::DefaultParseByte(sess.Lexer, c);
			ParseOutput(sess.Lexer);
			is.clear();
			is.seekg(0);
		}
	}

	observer_ptr<REPLContext> p_context;

	bool use_debug = {};

	ReductionStatus
		ProcessDebugCommand() {
		string cmd;
	begin:
		getline(std::cin, cmd);
		if (cmd == "r")
			return ReductionStatus::Retrying;
		else if (p_context && !cmd.empty()) {
			const bool u(use_debug);

			use_debug = {};
			FilterExceptions([&] {
				LogTermValue(p_context->Perform(cmd));
			}, lfsig);
			use_debug = u;
			goto begin;
		}
		return ReductionStatus::Clean;
	}

	ReductionStatus
		DefaultDebugAction(TermNode& term) {
		if (use_debug) {
			TraceDe(Debug, "List term: %p", leo::pvoid(&term));
			LogTermValue(term);
			return ProcessDebugCommand();
		}
		return ReductionStatus::Clean;
	}

	ReductionStatus
		DefaultLeafDebugAction(TermNode& term) {
		if (use_debug) {
			TraceDe(Debug, "Leaf term: %p", leo::pvoid(&term));
			LogTermValue(term);
			return ProcessDebugCommand();
		}
		return ReductionStatus::Clean;
	}

	template<typename _tNode, typename _fCallable>
	ReductionStatus
		ListCopyOrMove(TermNode& term, _fCallable f)
	{
		Forms::CallUnary([&](_tNode& node) {
			term.GetContainerRef() = node.CreateWith(f);
		}, term);
		return ReductionStatus::Clean;
	}

	template<typename _tNode, typename _fCallable>
	ReductionStatus
		TermCopyOrMove(TermNode& term, _fCallable f)
	{
		Forms::CallUnary([&](_tNode& node) {
			SetContentWith(term, node, f);
		}, term);
		return ReductionStatus::Clean;
	}

	int FetchListLength(TermNode& term) lnothrow
	{
		return int(term.size());
	}

	void
		LoadExternal(REPLContext& context, const string& name, ContextNode& ctx) {
		/*const auto p_sifs(Text::OpenSkippedBOMtream<
			IO::SharedInputMappedFileStream>(Text::BOM_UTF_8, name.c_str()));
		std::istream& is(*p_sifs);*/
		std::ifstream is(name);

		if (is) {
			TraceDe(Notice, "Test unit '%s' found.", name.c_str());
			FilterExceptions([&] {
				TryLoadSource(context, name.c_str(), is, ctx);
			});
		}
		else
			TraceDe(Notice, "Test unit '%s' not found.", name.c_str());

	}

	void
		LoadExternalRoot(REPLContext& context, const std::string&name) {
		LoadExternal(context, name, context.Root);
	}

	ArgumentsVector CommandArguments;

	void
		LoadSequenceSeparators(EvaluationPasses& passes)
	{
		RegisterSequenceContextTransformer(passes, TokenValue(";"), true),
		RegisterSequenceContextTransformer(passes, TokenValue(","));
	}

	template<typename _type>
	ReductionStatus TypeLiteralAction(TermNode & term)
	{

		const auto size(term.size());

		return ReductionStatus::Clean;
	}

	void RegisterTypeLiteralAction(REPLContext & context)
	{
		auto& root(context.Root);

		RegisterStrict(root, "float3", TypeLiteralAction<int>);
	}

	void LoadLSLContext(REPLContext & context) {
		using namespace std::placeholders;
		using namespace Forms;
		auto& root(context.Root);
		auto& root_env(root.GetRecordRef());

		LoadSequenceSeparators(context.ListTermPreprocess);
			root.EvaluateLiteral
			= [](TermNode& term, ContextNode&, string_view id) -> ReductionStatus {
			LAssertNonnull(id.data());
			if (!id.empty())
			{
				const char f(id.front());

				// NOTE: Handling extended literals.
				if (IsLSLAExtendedLiteralNonDigitPrefix(f) && id.size() > 1)
				{
					// TODO: Support numeric literal evaluation passes.
					if (id == "#t" || id == "#true")
						term.Value = true;
					else if (id == "#f" || id == "#false")
						term.Value = false;
					else if (id == "#n" || id == "#null")
						term.Value = nullptr;
					// XXX: Redundant test?
					else if (IsLSLAExtendedLiteral(id))
						throw InvalidSyntax(f == '#' ? "Invalid literal found."
							: "Unsupported literal prefix found.");
					else
						return ReductionStatus::Retrying;
				}
				else if (std::isdigit(f))
				{
					errno = 0;

					const auto ptr(id.data());
					char* eptr;
					const long ans(std::strtol(ptr, &eptr, 10));

					if (size_t(eptr - ptr) == id.size() && errno != ERANGE)
						// XXX: Conversion to 'int' might be implementation-defined.
						term.Value = int(ans);
					// TODO: Supported literal postfix?
					else
						throw InvalidSyntax("Literal postfix is unsupported.");
				}
				else
					return ReductionStatus::Retrying;
			}
			return ReductionStatus::Clean;
		};
		// NOTE: This is named after '#inert' in Kernel, but essentially
		//	unspecified in NPLA.
		root_env.Define("inert", ValueToken::Unspecified, {});
		// NOTE: This is like '#ignore' in Kernel, but with the symbol type. An
		//	alternative definition is by evaluating '$def! ignore $quote #ignore'
		//	(see below for '$def' and '$quote').
		root_env.Define("ignore", TokenValue("#ignore"), {});
		// NOTE: Primitive features, listed as RnRK, except mentioned above.

		RegisterTypeLiteralAction(context);
		context.Perform("float3 1 2 3");

		/*
		The primitives are provided here to maintain acyclic dependencies on derived
		forms, also for simplicity of semantics.
		The primitives are listed in order as Chapter 4 of Revised^-1 Report on the
		Kernel Programming Language. Derived forms are not ordered likewise.
		There are some difference of listed primitives.
		See $2017-02 @ %Documentation::Workflow::Annual2017.
		*/
		RegisterStrict(root, "eq?", Equal);
		RegisterStrict(root, "eql?", EqualLeaf);
		RegisterStrict(root, "eqr?", EqualReference);
		RegisterStrict(root, "eqv?", EqualValue);
		// NOTE: Like Scheme but not Kernel, '$if' treats non-boolean value as
		//	'#f', for zero overhead principle.
		RegisterForm(root, "$if", If);
		RegisterStrictUnary(root, "null?", ComposeReferencedTermOp(IsEmpty));
		RegisterStrictUnary(root, "nullpr?", IsEmpty);
		// NOTE: Though NPLA does not use cons pairs, corresponding primitives are
		//	still necessary.
		// NOTE: Since NPL has no con pairs, it only added head to existed list.
		RegisterStrict(root, "cons", Cons);
		RegisterStrict(root, "cons&", ConsRef);
		// NOTE: The applicative 'copy-es-immutable' is unsupported currently due to
		//	different implementation of control primitives.
		RegisterStrict(root, "eval", Eval);
		RegisterStrict(root, "eval&", EvalRef);

		// NOTE: Lazy form '$deflazy!' is the basic operation, which may bind
		//	parameter as unevaluated operands. For zero overhead principle, the form
		//	without recursion (named '$def!') is preferred. The recursion variant
		//	(named '$defrec!') is exact '$define!' in Kernel, and is used only when
		//	necessary.
		RegisterForm(root, "$deflazy!", DefineLazy);
		RegisterForm(root, "$def!", DefineWithNoRecursion);
		RegisterForm(root, "$defrec!", DefineWithRecursion);
		// NOTE: 'eqv? (() get-current-environment) (() ($vau () e e))' shall
		//	be evaluated to '#t'.
		RegisterForm(root, "$vau", Vau);
		RegisterForm(root, "$vau&", VauRef);
		RegisterForm(root, "$vaue", VauWithEnvironment);
		RegisterForm(root, "$vaue&", VauWithEnvironmentRef);
		RegisterStrictUnary<ContextHandler>(root, "wrap", Wrap);
		RegisterStrictUnary<ContextHandler>(root, "unwrap", Unwrap);

		RegisterStrict(root, "list", ReduceBranchToListValue);
		RegisterStrict(root, "list&", ReduceBranchToList);
		context.Perform(u8R"NPL(
		$def! $quote $vau (&x) #ignore x;
		$def! $set! $vau (&expr1 &formals .&expr2) env eval
			(list $def! formals (unwrap eval) expr2 env) (eval expr1 env);
		$def! $defv! $vau (&$f &formals &senv .&body) env eval
			(list $set! env $f $vau formals senv body) env;
		$def! $defv&! $vau (&$f &formals &senv .&body) env eval
			(list $set! env $f $vau& formals senv body) env;
	)NPL");

		RegisterForm(root, "$lambda", Lambda);
		RegisterForm(root, "$lambda&", LambdaRef);

		context.Perform(u8R"NPL(
		$defv! $setrec! (&expr1 &formals .&expr2) env eval
			(list $defrec! formals (unwrap eval) expr2 env) (eval expr1 env);
		$defv! $defl! (&f &formals .&body) env eval
			(list $set! env f $lambda formals body) env;
		$defv! $defl&! (&f &formals .&body) env eval
			(list $set! env f $lambda& formals body) env;
		$defl! first ((&x .)) x;
		$defl! rest ((#ignore .&x)) x;
		$defl! apply (&appv &arg .&opt) eval (cons () (cons (unwrap appv) arg))
			($if (null? opt) (() make-environment) (first opt));
		$defl! list* (&head .&tail)
			$if (null? tail) head (cons head (apply list* tail));
		$defv! $defw! (&f &formals &senv .&body) env eval
			(list $set! env f wrap (list* $vau formals senv body)) env;
		$defv! $defw&! (&f &formals &senv .&body) env eval
			(list $set! env f wrap (list* $vau& formals senv body)) env;
		$defv! $lambdae (&e &formals .&body) env
			wrap (eval (list* $vaue e formals ignore body) env);
		$defv! $lambdae& (&e &formals .&body) env
			wrap (eval (list* $vaue& e formals ignore body) env);
	)NPL");

		// NOTE: Some combiners are provided here as host primitives for
		//	more efficiency and less dependencies.
		// NOTE: The sequence operator is also available as infix ';' syntax sugar.
		RegisterForm(root, "$sequence", Sequence);

		context.Perform(u8R"NPL(
		$defv! $cond &clauses env
			$if (null? clauses) inert (apply ($lambda ((&test .&body) .clauses)
				$if (eval test env) (eval body env)
					(apply (wrap $cond) clauses env)) clauses);
	)NPL");

		// NOTE: Use of 'eql?' is more efficient than '$if'.
		context.Perform(u8R"NPL(
		$defl! not? (&x) eql? x #f;
		$defv! $when (&test .&vexpr) env $if (eval test env)
			(eval (list* $sequence vexpr) env);
		$defv! $unless (&test .&vexpr) env $if (not? (eval test env))
			(eval (list* $sequence vexpr) env);
	)NPL");
		RegisterForm(root, "$and?", And);
		RegisterForm(root, "$or?", Or);
		context.Perform(u8R"NPL(
		$defl! first-null? (&l) null? (first l);
		$defl! list-rest (&x) list (rest x);
		$defl! accl (&l &pred? &base &head &tail &sum) $sequence
			($defl! aux (&l &base)
				$if (pred? l) base (aux (tail l) (sum (head l) base)))
			(aux l base);
		$defl! accr (&l &pred? &base &head &tail &sum) $sequence
			($defl! aux (&l) $if (pred? l) base (sum (head l) (aux (tail l))))
			(aux l);
		$defl! foldr1 (&kons &knil &l) accr l null? knil first rest kons;
		$defw! map1 (&appv &l) env foldr1
			($lambda (&x &xs) cons (apply appv (list x) env) xs) () l;
		$defl! list-concat (&x &y) foldr1 cons y x;
		$defl! append (.&ls) foldr1 list-concat () ls;
		$defv! $let (&bindings .&body) env
			eval (list* () (list* $lambda (map1 first bindings) body)
				(map1 list-rest bindings)) env;
		$defv! $let* (&bindings .&body) env
			eval ($if (null? bindings) (list* $let bindings body)
				(list $let (list (first bindings))
				(list* $let* (rest bindings) body))) env;
	)NPL");
		context.Perform(u8R"NPL(
		$defl! unfoldable? (&l) accr l null? (first-null? l) first-null? rest
			$or?;
		$def! map-reverse $let ((&cenv () make-standard-environment)) wrap
			($sequence
				($set! cenv cxrs $lambdae (weaken-environment cenv) (&ls &cxr)
					accl ls null? () ($lambda (&l) cxr (first l)) rest cons)
				($vaue cenv (&appv .&ls) env accl ls unfoldable? ()
					($lambda (&ls) cxrs ls first) ($lambda (&ls) cxrs ls rest)
						($lambda (&x &xs) cons (apply appv x env) xs)));
		$defw! for-each-ltr &ls env $sequence (apply map-reverse ls env) inert;
	)NPL");
		// NOTE: Object interoperation.
		RegisterStrict(root, "ref", [](TermNode& term) {
			CallUnary([&](TermNode& tm) {
				LiftToReference(term, tm);
			}, term);
			return CheckNorm(term);
		});
		// NOTE: Environments.
		RegisterStrictUnary(root, "bound?",
			[](TermNode& term, const ContextNode& ctx) {
			return leo::call_value_or([&](string_view id) {
				return CheckSymbol(id, [&]() {
					return bool(ResolveName(ctx, id).first);
				});
			}, AccessTermPtr<string>(term));
		});
		context.Perform(u8R"NPL(
		$defv! $binds1? (&expr &s) env
			eval (list (unwrap bound?) (symbol->string s)) (eval expr env);
	)NPL");
		RegisterStrict(root, "value-of", ValueOf);
	}

	/// 740
	void
		LoadFunctions(REPLContext& context)
	{
		using namespace std::placeholders;
		using namespace Forms;
		auto& root(context.Root);
		auto& root_env(root.GetRecordRef());

		p_context = make_observer(&context);
		LoadLSLContext(context);
		// TODO: Extract literal configuration API.
		{
			// TODO: Blocked. Use C++14 lambda initializers to simplify
			//	implementation.

			root.EvaluateLiteral = [lit_base =std::move(root.EvaluateLiteral.begin()->second),lit_ext= FetchExtendedLiteralPass()](TermNode& term,
				ContextNode& ctx, string_view id)->ReductionStatus{
				const auto res(lit_ext(term, ctx, id));

				if (res == ReductionStatus::Clean
					&& term.Value.type() == leo::type_id<TokenValue>())
					return lit_base(term, ctx, id);
				return res;
			}; 
		}
		// NOTE: Literal builtins.
		RegisterLiteralSignal(root, "exit", SSignal::Exit);
		RegisterLiteralSignal(root, "cls", SSignal::ClearScreen);
		RegisterLiteralSignal(root, "about", SSignal::About);
		RegisterLiteralSignal(root, "help", SSignal::Help);
		RegisterLiteralSignal(root, "license", SSignal::License);
		// NOTE: Definition of %inert is in %LFramework.LSL.Dependency.
		// NOTE: Context builtins.
		root_env.Define("REPL-context", ValueObject(context, OwnershipTag<>()), {});
		root_env.Define("root-context", ValueObject(root, OwnershipTag<>()), {});
		// NOTE: Literal expression forms.
		RegisterForm(root, "$retain", Retain);
		//leo::bind1(RetainN, 1)
		RegisterForm(root, "$retain1", [](const TermNode& term) {return RetainN(term, 1); });
#if true
		// NOTE: Primitive features, listed as RnRK, except mentioned above. See
		//	%LFramework.LSL.Dependency.
		// NOTE: Definitions of eq?, eql?, eqr?, eqv? are in
		//	%LFramework.LSL.Dependency.
		RegisterStrictUnary<const string>(root, "symbol-string?", IsSymbol);
		// NOTE: Definitions of $if is in %LFramework.LSL.Dependency.
		//RegisterStrictUnary(root, "list?", ComposeReferencedTermOp(IsList));
		RegisterStrictUnary(root, "listpr?", IsList);
		// TODO: Add nonnull list predicate to improve performance?
		// NOTE: Definitions of null?, cons, cons&, eval, copy-environment,
		//	make-environment, get-current-environment, weaken-environment,
		//	lock-environment are in %LFramework.LSL.Dependency.
		// NOTE: Environment mutation is optional in Kernel and supported here.
		// NOTE: Definitions of $deflazy!, $def!, $defrec! are in
		//	%LFramework.LSL.Dependency.
		//leo::bind1(Undefine, _2, true)
		RegisterForm(root, "$undef!", [](TermNode& term, ContextNode& ctx) {return Undefine(term, ctx, true); });
		RegisterForm(root, "$undef-checked!", [](TermNode& term, ContextNode& ctx) {return Undefine(term, ctx, false); });
		// NOTE: Definitions of $vau, $vaue, wrap are in %LFramework.LSL.Dependency.
		// NOTE: The applicative 'wrap1' does check before wrapping.
		RegisterStrictUnary<ContextHandler>(root, "wrap1", WrapOnce);
		// NOTE: Definitions of unwrap is in %LFramework.LSL.Dependency.
#endif
	// NOTE: LSLA value transferring.
		RegisterStrictUnary(root, "vcopy", [](const TermNode& node) {
			return node.Value.MakeCopy();
		});
		RegisterStrictUnary(root, "vcopymove", [](TermNode& node) {
			// NOTE: Shallow copy or move.
			return node.Value.CopyMove();
		});
		RegisterStrictUnary(root, "vmove", [](const TermNode& node) {
			return node.Value.MakeMove();
		});
		RegisterStrictUnary(root, "vmovecopy", [](const TermNode& node) {
			return node.Value.MakeMoveCopy();
		});
		RegisterStrict(root, "lcopy", [](TermNode& term) {
			return ListCopyOrMove<const TermNode>(term, &ValueObject::MakeCopy);
		});
		RegisterStrict(root, "lcopymove", [](TermNode& term) {
			return ListCopyOrMove<TermNode>(term, &ValueObject::CopyMove);
		});
		RegisterStrict(root, "lmove", [](TermNode& term) {
			return ListCopyOrMove<const TermNode>(term, &ValueObject::MakeMove);
		});
		RegisterStrict(root, "lmovecopy", [](TermNode& term) {
			return ListCopyOrMove<const TermNode>(term, &ValueObject::MakeMoveCopy);
		});
		RegisterStrict(root, "tcopy", [](TermNode& term) {
			return TermCopyOrMove<const TermNode>(term, &ValueObject::MakeCopy);
		});
		RegisterStrict(root, "tcopymove", [](TermNode& term) {
			return TermCopyOrMove<TermNode>(term, &ValueObject::CopyMove);
		});
		RegisterStrict(root, "tmove", [](TermNode& term) {
			return TermCopyOrMove<const TermNode>(term, &ValueObject::MakeMove);
		});
		RegisterStrict(root, "tmovecopy", [](TermNode& term) {
			return TermCopyOrMove<const TermNode>(term, &ValueObject::MakeMoveCopy);
		});
		// XXX: For test or debug only.
		RegisterStrictUnary(root, "tt", DefaultDebugAction);
		RegisterStrictUnary<const string>(root, "dbg", [](const string& cmd) {
			if (cmd == "on")
				use_debug = true;
			else if (cmd == "off")
				use_debug = {};
			else if (cmd == "crash")
				leo::terminate();
		});
		RegisterForm(root, "$crash", [] {
			leo::terminate();
		});
		// NOTE: Derived functions with probable privmitive implementation.
		// NOTE: Definitions of list, list&, $quote are in
		//	%LFramework.LSL.Dependency.
#if true
		RegisterStrict(root, "id", [](TermNode& term) {
			RetainN(term);
			LiftTerm(term, Deref(std::next(term.begin())));
			LiftToSelfSafe(term);
			return CheckNorm(term);
		});
#else
		context.Perform(u8R"LSL($def! id $lambda (&x) x)LSL");
#endif
		RegisterStrict(root, "id&", [](TermNode& term) {
			RetainN(term);
			LiftTerm(term, Deref(std::next(term.begin())));
			return CheckNorm(term);
		});
		context.Perform(u8R"LSL($defl! xcons (&x &y) cons y x)LSL");
		// NOTE: Definitions of $set!, $defv!, $lambda, $setrec!, $defl!, first,
		//	rest, apply, list*, $defw!, $lambdae, $sequence, $cond,
		//	make-standard-environment, not?, $when, $unless are in
		//	%LFramework.LSL.Dependency.
		context.Perform(u8R"LSL(
		$defl! and? &x $sequence
			($defl! and2? (&x &y) $if (null? y) x
				($sequence ($def! h first y) (and2? ($if h x #f) (rest y))))
			(and2? #t x);
		$defl! or? &x $sequence
			($defl! or2? (&x &y) $if (null? y) x
				($sequence ($def! h first y) (or2? ($if h h x) (rest y))))
			(or2? #f x);
	)LSL");
		// NOTE: Definitions of $and?, $or?, first-null?,
		//	list-rest, accl, accr are in %LFramework.LSL.Dependency.
		context.Perform(u8R"LSL(
		$defl! foldl1 (&kons &knil &l) accl l null? knil first rest kons;
		$defw! map1-reverse (&appv &l) env foldl1
			($lambda (&x &xs) cons (apply appv (list x) env) xs) () l;
	)LSL");
		// NOTE: Definitions of foldr1, map1, list-concat are in
		//	%LFramework.LSL.Dependency.
		context.Perform(u8R"LSL(
		$defl! list-copy-strict (l) foldr1 cons () l;
		$defl! list-copy (obj) $if (list? obj) (list-copy-strict obj) obj;
	)LSL");
		// NOTE: Definitions of append is in %LFramework.LSL.Dependency.
		context.Perform(u8R"LSL(
		$defl! reverse (&l) foldl1 cons () l;
		$defl! snoc (&x &r) (list-concat r (list x));
	)LSL");
		// NOTE: Definitions of $let is in %LFramework.LSL.Dependency.
		context.Perform(u8R"LSL(
		$defl! filter (&accept? &ls) apply append
			(map1 ($lambda (&x) $if (apply accept? (list x)) (list x) ()) ls);
		$defl! reduce (&ls &bin &id) $cond
			((null? ls) id)
			((null? (rest ls)) first ls)
			(#t bin (first ls) (reduce (rest ls) bin id));
		$defl! assv (&object &alist) $let
			((alist
				filter ($lambda (&record) eqv? object (first record)) alist))
				$if (null? alist) () (first alist);
		$defl! memv? (&object &ls)
			apply or? (map1 ($lambda (&x) eqv? object x) ls);
		$defl! assq (&object &alist) $let
			((alist filter ($lambda (&record) eq? object (first record)) alist))
				($if (null? alist) () (first alist));
		$defl! memq? (&object &ls)
			apply or? (map1 ($lambda (&x) eq? object x) ls);
		$defl! equal? (&x &y) $if ($and? (branch? x) (branch? y))
			($and? (equal? (first x) (first y)) (equal? (rest x) (rest y)))
			(eqv? x y);
	)LSL");
		// NOTE: Definitions of $let* are in
		//	%LFramework.LSL.Dependency.
		context.Perform(u8R"LSL(
		$defv! $letrec (&bindings .&body) env
			eval (list $let () $sequence (list $def! (map1 first bindings)
				(list* () list (map1 rest bindings))) body) env;
		$defv! $letrec* (&bindings .&body) env
			eval ($if (null? bindings) (list* $letrec bindings body)
				(list $letrec (list (first bindings))
				(list* $letrec* (rest bindings) body))) env;
		$defv! $let-redirect (&expr &bindings .&body) env
			eval (list* () (eval (list* $lambda (map1 first bindings) body)
				(eval expr env)) (map1 list-rest bindings)) env;
		$defv! $let-safe (&bindings .&body) env
			eval (list* () $let-redirect
				(() make-standard-environment) bindings body) env;
		$defv! $remote-eval (&o &e) d eval o (eval e d);
		$defv! $bindings->environment &bindings denv
			eval (list $let-redirect (() make-environment) bindings
				(list lock-environment (list () get-current-environment))) denv;
		$defv! $provide! (&symbols .&body) env
			eval (list $def! symbols
				(list $let () $sequence body (list* list symbols))) env;
		$defv! $import! (&expr .&symbols) env
			eval (list $set! env symbols (cons list symbols)) (eval expr env);
		$def! foldr $let ((&cenv () make-standard-environment)) wrap
		(
			$set! cenv cxrs $lambdae (weaken-environment cenv) (ls cxr)
				accr ls null? () ($lambda (&l) cxr (first l)) rest cons;
			$vaue cenv (kons knil .ls) env
				(accr ls unfoldable? knil ($lambda (&ls) cxrs ls first)
				($lambda (&ls) cxrs ls rest) ($lambda (&x &st)
					apply kons (list-concat x (list st)) env))
		);
		$def! map $let ((&cenv () make-standard-environment)) wrap
		(
			$set! cenv cxrs $lambdae (weaken-environment cenv) (ls cxr)
				accr ls null? () ($lambda (&l) cxr (first l)) rest cons;
			$vaue cenv (appv .ls) env accr ls unfoldable? ()
				($lambda (&ls) cxrs ls first) ($lambda (&ls) cxrs ls rest)
					($lambda (&x &xs) cons (apply appv x env) xs)
		);
		$defw! for-each-rtl &ls env $sequence (apply map ls env) inert;
	)LSL");
		// NOTE: Definitions of unfoldable?, map-reverse for-each-ltr
		//	are in %LFramework.LSL.Dependency.
		RegisterForm(root, "$delay", [](TermNode& term, ContextNode&) {
			term.Remove(term.begin());

			ValueObject x(DelayedTerm(std::move(term)));

			term.Value = std::move(x);
			return ReductionStatus::Clean;
		});
		// TODO: Provide 'equal?'.
		RegisterForm(root, "evalv",
			static_cast<void(&)(TermNode&, ContextNode&)>(ReduceChildren));
		RegisterStrict(root, "force", [](TermNode& term) {
			RetainN(term);
			return EvaluateDelayed(term,
				Access<DelayedTerm>(Deref(std::next(term.begin()))));
		});
		// NOTE: Object interoperation.
		// NOTE: Definitions of ref is in %LFramework.LSL.Dependency.
		// NOTE: Environments.
		// NOTE: Definitions of bound?, value-of is in %LFramework.LSL.Dependency.
		// NOTE: Only '$binds?' is like in Kernel.
		context.Perform(u8R"LSL(
		$defw! environment-bound? (&expr &str) env
			eval (list bound? str) (eval expr env);
		$defv! $binds1? (&expr &s) env
			eval (list (unwrap bound?) (symbol->string s)) (eval expr env);
		$defv! $binds? (&expr .&ss) env $let ((&senv eval expr env))
			foldl1 $and? #t (map1 ($lambda (s) (wrap $binds1?) senv s) ss);
	)LSL");
		//leo::bind1(EvaluateUnit, std::ref(context))
		RegisterStrict(root, "eval-u", [&](TermNode& term) {
			return EvaluateUnit(term, std::ref(context));
		});
		//leo::bind1(RetainN, 2)
		RegisterStrict(root, "eval-u-in", [](TermNode& term) {
			const auto i(std::next(term.begin()));
			const auto& rctx(Access<REPLContext>(Deref(i)));

			term.Remove(i);
			EvaluateUnit(term, rctx);
		}, [](const TermNode& term) {
			return RetainN(term, 2);
		});
		RegisterStrictUnary<const string>(root, "lex", [&](const string& unit) {
			LexicalAnalyzer lex;

			for (const auto& c : unit)
				lex.ParseByte(c);
			return lex;
		});
		RegisterStrictUnary<const std::type_index>(root, "nameof",
			[](const std::type_index& ti) {
			return string(ti.name());
		});
		// NOTE: Type operation library.
		RegisterStrictUnary(root, "typeid", [](const TermNode& term) {
			// FIXME: Get it work with %YB_Use_LightweightTypeID.
			return std::type_index(ReferenceTerm(term).Value.type());
		});
		// TODO: Copy of operand cannot be used for move-only types.
		context.Perform("$defl! ptype (&x) puts (nameof (typeid x))");
		RegisterStrictUnary<string>(root, "get-typeid",
			[&](const string& str) -> std::type_index {
			if (str == "bool")
				return typeid(bool);
			if (str == "symbol")
				return typeid(TokenValue);
			// XXX: The environment type is not unique.
			if (str == "environment")
				return typeid(EnvironmentReference);
			if (str == "environment#owned")
				return typeid(shared_ptr<scheme::Environment>);
			if (str == "int")
				return typeid(int);
			if (str == "operative")
				return typeid(FormContextHandler);
			if (str == "applicative")
				return typeid(StrictContextHandler);
			if (str == "combiner")
				return typeid(ContextHandler);
			if (str == "string")
				return typeid(string);
			return typeid(void);
		});
		RegisterStrictUnary<const ContextHandler>(root, "get-combiner-type",
			[&](const ContextHandler& h) {
			return std::type_index(h.target_type());
		});
		context.Perform(u8R"LSL(
		$defl! typeid-match? (&x &id) eqv? (get-typeid id) (typeid x);
		$defl! bool? (&x) typeid-match? x "bool";
		$defl! symbol? (&x) $and? (typeid-match? x "symbol")
			(symbol-string? (symbol->string x));
		$defl! environment? (&x) $or? (typeid-match? x "environment")
			(typeid-match? x "environment#owned");
		$defl! combiner? (&x) typeid-match? x "combiner";
		$defl! operative? (&x) $and? (combiner? x)
			(eqv? (get-combiner-type x) (get-typeid "operative"));
		$defl! applicative? (&x) $and? (combiner? x)
			(eqv? (get-combiner-type x) (get-typeid "applicative"));
		$defl! int? (&x) typeid-match? x "int";
		$defl! string? (&x) typeid-match? x "string";
	)LSL");
		// NOTE: List library.
		// TODO: Check list type?
		RegisterStrictUnary(root, "list-length",
			ComposeReferencedTermOp(FetchListLength));
		RegisterStrictUnary(root, "listv-length", FetchListLength);
		RegisterStrictUnary(root, "branch?", ComposeReferencedTermOp(IsBranch));
		RegisterStrictUnary(root, "branchpr?", IsBranch);
		RegisterStrictUnary(root, "leaf?", ComposeReferencedTermOp(IsLeaf));
		RegisterStrictUnary(root, "leafpr?", IsLeaf);
		// NOTE: String library.
		// NOTE: Definitions of ++, string-empty?, string<- are in
		//	%LFramework.LSL.Dependency.
		RegisterStrictBinary<string>(root, "string=?", leo::equal_to<>());
		context.Perform(u8R"LSL($defl! retain-string (&str) ++ "\"" str "\"")LSL");
		RegisterStrictUnary<const int>(root, "itos", [](int x) {
			return to_string(x);
		});
		RegisterStrictUnary<const string>(root, "string-length",
			[&](const string& str) lnothrow{
				return int(str.length());
			});
		// NOTE: Definitions of string->symbol, symbol->string, string->regex,
		//	regex-match? are in %LFramework.LSL.Dependency.
		// NOTE: Comparison.
		RegisterStrictBinary<int>(root, "=?", leo::equal_to<>());
		RegisterStrictBinary<int>(root, "<?", leo::less<>());
		RegisterStrictBinary<int>(root, "<=?", leo::less_equal<>());
		RegisterStrictBinary<int>(root, ">=?", leo::greater_equal<>());
		RegisterStrictBinary<int>(root, ">?", leo::greater<>());
		// NOTE: Arithmetic procedures.
		// FIXME: Overflow?
		//std::bind(CallBinaryFold<int, leo::plus<>>,leo::plus<>(), 0, _1)
		RegisterStrict(root, "+", [](TermNode& term) {
			return CallBinaryFold<int, leo::plus<>>(leo::plus<>(),0, term);
		});
		// FIXME: Overflow?
		RegisterStrictBinary<int>(root, "add2", leo::plus<>());
		// FIXME: Underflow?
		RegisterStrictBinary<int>(root, "-", leo::minus<>());
		// FIXME: Overflow?
		//std::bind(CallBinaryFold<int,leo::multiplies< >> , leo::multiplies<>(), 1, _1)
		RegisterStrict(root, "*", [](TermNode& term) {
			return CallBinaryFold<int, leo::multiplies<>>(leo::multiplies<>(),1, term);
		});
		// FIXME: Overflow?
		RegisterStrictBinary<int>(root, "multiply2", leo::multiplies<>());
		RegisterStrictBinary<int>(root, "/", [](int e1, int e2) {
			if (e2 != 0)
				return e1 / e2;
			throw std::domain_error("Runtime error: divided by zero.");
		});
		RegisterStrictBinary<int>(root, "%", [](int e1, int e2) {
			if (e2 != 0)
				return e1 % e2;
			throw std::domain_error("Runtime error: divided by zero.");
		});
		// NOTE: I/O library.
		//leo::bind1(LogTermValue, Notice)
		RegisterStrict(root, "display", [](const TermNode& term) {
			return LogTermValue(term, Notice);
		});
		RegisterStrictUnary<const string>(root, "echo", Echo);
		//std::bind(LoadExternalRoot, std::ref(context), _1)
		RegisterStrictUnary<const string>(root, "load",
			[&](const std::string&name){
			return LoadExternalRoot(std::ref(context), name);
		});
		RegisterStrictUnary<const string>(root, "load-at-root",
			[&](const string& name) {
			const leo::guard<EnvironmentSwitcher>
				gd(root, root.SwitchEnvironment(root_env.shared_from_this()));

			LoadExternalRoot(context, name);
		});
		context.Perform(u8R"LSL(
		$defl! get-module (&filename .&opt)
			$let ((&env () make-standard-environment)) $sequence
				($unless (null? opt) ($set! env module-parameters (first opt)))
				(eval (list load filename) env) env;
	)LSL");
		RegisterStrictUnary<const string>(root, "ofs", [&](const string& path) {
			if (ifstream ifs{ path })
				return ifs;
			throw LoggedEvent(
				leo::sfmt("Failed opening file '%s'.", path.c_str()));
		});
		RegisterStrictUnary<const string>(root, "oss", [&](const string& str) {
			return std::istringstream(str);
		});
		RegisterStrictUnary<ifstream>(root, "parse-f", ParseStream);
		RegisterStrictUnary<LexicalAnalyzer>(root, "parse-lex", ParseOutput);
		RegisterStrictUnary<std::istringstream>(root, "parse-s", ParseStream);
		RegisterStrictUnary<const string>(root, "put", [&](const string& str) {
			std::cout << EncodeArg(str);
		});
		RegisterStrictUnary<const string>(root, "puts", [&](const string& str) {
			// XXX: Overridding.
			std::cout << EncodeArg(str) << std::endl;
		});
		// NOTE: Interoperation library.
		// NOTE: Definitions of env-get, system
		//	are in %LFramework.LSL.Dependency.
		// NOTE: Definition of env-set, cmd-get-args, system-get are also in
		//	%Tools.SHBuild.Main.
		RegisterStrictBinary<const string>(root, "env-set",
			[&](const string& var, const string& val) {
			SetEnvironmentVariable(var.c_str(), val.c_str());
		});
		RegisterStrict(root, "cmd-get-args", [](TermNode& term) {
			term.Clear();
			for (const auto& s : CommandArguments.Arguments)
				term.AddValue(MakeIndex(term), s);
			return ReductionStatus::Retained;
		});
		RegisterStrict(root, "system-get", [](TermNode& term) {
			CallUnaryAs<const string>([&](const string& cmd) {
				auto res(FetchCommandOutput(cmd.c_str()));

				term.Clear();
				term.AddValue(MakeIndex(0), leo::trim(std::move(res.first)));
				term.AddValue(MakeIndex(1), res.second);
			}, term);
			return ReductionStatus::Retained;
		});
		// NOTE: SHBuild builitins.
		// XXX: Overriding.
		root_env.Define("SHBuild_BaseTerminalHook_",
			ValueObject(std::function<void(const string&, const string&)>(
				[](const string& n, const string& val) {
			// XXX: Errors from stream operations are ignored.
			using namespace std;
			Terminal te;

			cout << te.LockForeColor(DarkCyan) << n;
			cout << " = \"";
			cout << te.LockForeColor(DarkRed) << val;
			cout << '"' << endl;
		})), true);
		context.Perform("$defl! iput (&x) puts (itos x)");
		LoadExternalRoot(context, "test.txt");
		root.EvaluateList.Add(DefaultDebugAction, 255);
		root.EvaluateLeaf.Add(DefaultLeafDebugAction, 255);
	}

} // unnamed namespace;


/// 304
int
main(int argc, char* argv[])
{
	using namespace std;
	leo::setnbuf(stdout);
	CommandArguments.Reset(argc, argv);
	lunused(argc), lunused(argv);
	return FilterExceptions([] {
		Application app;
		Interpreter intp(app, LoadFunctions);

		while (intp.WaitForLine() && intp.Process())
			;
	}, "::main") ? EXIT_FAILURE : EXIT_SUCCESS;
}

