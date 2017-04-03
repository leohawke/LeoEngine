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
//#include YFM_NPL_Configuration
//#include YFM_Helper_Initialization
#include <LBase/Debug.h>
#include <LBase/Win32/Mingw32.h>
//#include YFM_YSLib_Service_TextFile
#include <LBase/Win32/NLS.h>

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
		[=](const ContextNode&) LB_ATTR(noreturn) -> ReductionStatus{
		throw sig;
	});
}

} // namespace A1;

} // namespace NPL;

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
	for(const auto& str : rlst)
		cout << EncodeArg(str) << endl
			<< "* u8 length: " << str.size() << endl;
	cout << rlst.size() << " token(s) parsed." <<endl;
}

/// 737
void
ParseStream(std::istream& is)
{
	if(is)
	{
		Session sess;
		char c;

		while((c = is.get()), is)
			Session::DefaultParseByte(sess.Lexer, c);
		ParseOutput(sess.Lexer);
	}
}


/// 740
void
LoadFunctions(REPLContext& context)
{
	using namespace std::placeholders;
	using namespace Forms;
	auto& root(context.Root);

	LoadSequenceSeparators(root, context.ListTermPreprocess);
	LoadDeafultLiteralPasses(root);
	RegisterLiteralSignal(root, "exit", SSignal::Exit);
	RegisterLiteralSignal(root, "cls", SSignal::ClearScreen);
	RegisterLiteralSignal(root, "about", SSignal::About);
	RegisterLiteralSignal(root, "help", SSignal::Help);
	RegisterLiteralSignal(root, "license", SSignal::License);
	// NOTE: Context builtins.

	DefineValue(root, "REPL-context", ValueObject(context, OwnershipTag<>()),
	{});

	DefineValue(root, "root-context", ValueObject(root, OwnershipTag<>()),
	{});

	// NOTE: Literal expression forms.

	RegisterForm(root, "$Retain", Retain);

	//RegisterForm(root, "$Retain1",
	//	leo::bind1(RetainN, 1));

	// NOTE: Binding and control forms.

	RegisterForm(root, "$lambda", Lambda);

	// NOTE: Privmitive procedures.

	RegisterForm(root, "$and", And);

	RegisterForm(root, "begin", ReduceOrdered),

		RegisterStrict(root, "eq?", EqualReference);

	RegisterForm(root, "list",
		static_cast<void(&)(TermNode&, ContextNode&)>(ReduceChildren));

	// NOTE: Arithmetic procedures.

	// FIXME: Overflow?

	RegisterStrict(root, "+", [](TermNode& term) {
		CallBinaryFold<int, leo::plus<>>(leo::plus<>(), 0, term); 
	});

	// FIXME: Overflow?

	RegisterStrictBinary<int>(root, "add2", leo::plus<>());

	// FIXME: Underflow?

	RegisterStrictBinary<int>(root, "-", leo::minus<>());

	// FIXME: Overflow?

	RegisterStrict(root, "*", [](TermNode& term) {
		CallBinaryFold<int, leo::multiplies<>>(leo::multiplies<>(), 1, term);
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
	RegisterStrict(root, "display", [](TermNode& term) {LogTree(term, Notice); });
	RegisterStrictUnary<const string>(root, "echo", Echo);
	RegisterStrict(root, "eval", [&](TermNode& term) 
		{
			EvaluateUnit(term, std::ref(context));
		}
	);

	RegisterStrict(root, "eval-in", [](TermNode& term) {
		const auto i(std::next(term.begin()));
		const auto& rctx(Access<REPLContext>(Deref(i)));
		term.Remove(i);
		EvaluateUnit(term, rctx);
	}, leo::bind1(RetainN, 2));

	RegisterStrictUnary<const std::string>(root, "lex", [&](const string& unit) {
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

	RegisterStrictUnary(root, "typeid", [](TermNode& term) {
		// FIXME: Get it work with %LB_Use_LightweightTypeID.
		return std::type_index(term.Value.GetType());
	});

	context.Perform("$define (ptype x) puts (nameof (typeid(x)))");

	RegisterStrictUnary<string>(root, "get-typeid",
		[&](const string& str) -> std::type_index {
		if (str == "bool")
			return typeid(bool);
		if (str == "int")
			return typeid(int);
		if (str == "string")
			return typeid(string);
		return typeid(void);
	});

	context.Perform("$define (bool? x) eqv? (get-typeid \"bool\")"
		" (typeid x)");

	context.Perform("$define (int? x) eqv? (get-typeid \"int\")"
		" (typeid x)");

	context.Perform("$define (string? x) eqv? (get-typeid \"string\")"
		" (typeid x)");

	// NOTE: String library.

	context.Perform(u8R"NPL($define (Retain-string str) ++ "\"" str "\"")NPL");

	RegisterStrictUnary<const int>(root, "itos", [](int x) {
		return to_string(x);
	});

	RegisterStrictUnary<const string>(root, "strlen", [&](const string& str) {
		return int(str.length());
	});

	// NOTE: SHBuild builitins.

	// XXX: Overriding.

	DefineValue(root, "SHBuild_BaseTerminalHook_",

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
}

} // unnamed namespace;


/// 304
int
main(int argc, char* argv[])
{
	using namespace std;

	lunused(argc), lunused(argv);
	return FilterExceptions([]{
		Application app;
		Interpreter intp(app, LoadFunctions);

		while(intp.WaitForLine() && intp.Process())
			;
	}, "::main") ? EXIT_FAILURE : EXIT_SUCCESS;
}

