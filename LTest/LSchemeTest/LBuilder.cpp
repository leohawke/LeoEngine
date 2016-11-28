/*!	\file LBuilder.cpp
\ingroup LTest
\brief NPL 解释实现。
\par 修改时间:
	2016-11-18 15:44 +0800
*/


#include "LBuilder.h"
#include <streambuf>
#include <sstream>
#include <iostream>
#include <fstream>
//#include YFM_NPL_Configuration
//#include YFM_Helper_Initialization
#include <LBase/Debug.h>
#include <LBase/Win32/Mingw32.h>
//#include YFM_YSLib_Service_TextFile
#include <LBase/Win32/NLS.h>

namespace scheme
{

namespace v1
{

void
RegisterLiteralSignal(ContextNode& node, const string& name, SSignal sig)
{
	RegisterLiteralHandler(node, name,
		[=](const ContextNode&) LB_ATTR(noreturn) -> bool{
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
	RegisterFormContextHandler(root, "$quote", Quote, IsBranch);
	RegisterFormContextHandler(root, "$quote1",
		leo::bind1(QuoteN, 1), IsBranch);
	RegisterFormContextHandler(root, "$define",
		std::bind(DefineOrSet, _1, _2, true), IsBranch);
	RegisterFormContextHandler(root, "$set",
		std::bind(DefineOrSet, _1, _2, false), IsBranch);
	RegisterFormContextHandler(root, "$lambda", Lambda, IsBranch);
	RegisterFunction(root, "$display", leo::bind1(LogTree, Notice));
	RegisterUnaryFunction(root, "$ifdef",
		[](TermNode& term, const ContextNode& ctx){
		return leo::call_value_or<bool>([&](string_view id){
			return bool(LookupName(ctx, id));
		}, AccessPtr<string>(term));
	});
	// NOTE: Examples.
	// FIXME: Overflow?
	RegisterFunction(root, "+", std::bind(DoIntegerNAryArithmetics<
		leo::plus<>>, leo::plus<>(), 0, _1), IsBranch);
	// FIXME: Overflow?
	RegisterFunction(root, "add2", std::bind(DoIntegerBinaryArithmetics<
		leo::plus<>>, leo::plus<>(), _1), IsBranch);
	// FIXME: Underflow?
	RegisterFunction(root, "-", std::bind(DoIntegerBinaryArithmetics<
		leo::minus<>>, leo::minus<>(), _1), IsBranch);
	// FIXME: Overflow?
	RegisterFunction(root, "*", std::bind(DoIntegerNAryArithmetics<
		leo::multiplies<>>, leo::multiplies<>(), 1, _1), IsBranch);
	// FIXME: Overflow?
	RegisterFunction(root, "multiply2", std::bind(DoIntegerBinaryArithmetics<
		leo::multiplies<>>, leo::multiplies<>(), _1), IsBranch);
	RegisterFunction(root, "/", [](TermNode& term){
		DoIntegerBinaryArithmetics([](int e1, int e2){
			if(e2 != 0)
				return e1 / e2;
			throw std::domain_error("Runtime error: divided by zero.");
		}, term);
	}, IsBranch);
	RegisterFunction(root, "%", [](TermNode& term){
		DoIntegerBinaryArithmetics([](int e1, int e2){
			if(e2 != 0)
				return e1 % e2;
			throw std::domain_error("Runtime error: divided by zero.");
		}, term);
	}, IsBranch);
	RegisterUnaryFunction<const string>(root, "eval",
		[&](const string& t) {Eval(t, context); }
		);
	RegisterFunction(root, "system", CallSystem);
	RegisterUnaryFunction<const string>(root, "echo", Echo);
	RegisterUnaryFunction<const string>(root, "ofs", [&](const string& path){
		if(ifstream ifs{path})
			return ifs;
		throw LoggedEvent(
			leo::sfmt("Failed opening file '%s'.", path.c_str()));
	});
	RegisterUnaryFunction<const string>(root, "oss", [&](const string& str){
		return std::istringstream(str);
	});
	RegisterUnaryFunction<ifstream>(root, "parse-f", ParseStream);
	RegisterUnaryFunction<std::istringstream>(root, "parse-s", ParseStream);
	RegisterUnaryFunction<const string>(root, "lex", [&](const string& unit){
		LexicalAnalyzer lex;

		for(const auto& c : unit)
			lex.ParseByte(c);
		return lex;
	});
	RegisterUnaryFunction<LexicalAnalyzer>(root, "parse-lex", ParseOutput);
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

