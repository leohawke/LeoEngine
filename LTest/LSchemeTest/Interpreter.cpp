﻿#include "Interpreter.h"
#include <iostream>

#include <LFramework/LCLib/FCommon.h>

//#include YFM_Helper_Environment
//#include YFM_YSLib_Service_TextFile

using namespace leo;

#define LSL_TracePerform 1
#define LSL_TracePerformDetails 0

namespace scheme
{

#define LSL_NAME "LSL console"
#define LSL_VER "CPP17"
#define LSL_PLATFORM "[Win32]"
lconstexpr auto prompt("> ");
lconstexpr auto title(LSL_NAME" " LSL_VER" @ (" __DATE__", " __TIME__") "
	LSL_PLATFORM);

/// 519
namespace
{

/// 520
using namespace platform_ex::Windows;

/// 691
LB_NONNULL(3) void
PrintError(WConsole& wc, const LoggedEvent& e, const char* name = "Error")
{
	wc.UpdateForeColor(ErrorColor);
	LFL_TraceRaw(e.GetLevel(), "%s[%s]<%u>: %s", name, typeid(e).name(),
		unsigned(e.GetLevel()), e.what());
//	ExtractAndTrace(e, e.GetLevel());
}

void
PrintTermNode(std::ostream& os, const ValueNode& node, NodeToString node_to_str,
	IndentGenerator igen = DefaultGenerateIndent, size_t depth = 0)
{
	PrintIndent(os, igen, depth);
	os << EscapeLiteral(node.GetName()) << ' ';

	const auto print_node_str(
		[&](const ValueNode& nd) -> pair<lref<const ValueNode>, bool> {
		const TermNode& term(nd);
		const auto& tm(ReferenceTerm(term));
		const ValueNode& vnode(tm);

		if (&tm != &term)
			os << '*';
		return { vnode, PrintNodeString(os, vnode, node_to_str) };
	});
	const auto pr(print_node_str(node));

	if (!pr.second)
	{
		const auto& vnode(pr.first.get());

		os << '\n';
		if (vnode)
			TraverseNodeChildAndPrint(os, vnode, [&] {
			PrintIndent(os, igen, depth);
		}, print_node_str, [&](const ValueNode& nd) {
			return PrintTermNode(os, nd, node_to_str, igen, depth + 1);
		});
	}
}

} // unnamed namespace;


void
LogTree(const ValueNode& node, Logger::Level lv)
{
	std::ostringstream oss;

	PrintNode(oss, node, [](const ValueNode& node){
		return EscapeLiteral([&]() -> string{
			if (node.Value != v1::ValueToken::Null) {
				if (const auto p = AccessPtr<string>(node))
					return *p;
				if (const auto p = AccessPtr<TokenValue>(node))
					return sfmt("[TokenValue] %s", p->c_str());
				if (const auto p = AccessPtr<v1::ValueToken>(node))
					return sfmt("[ValueToken] %s", to_string(*p).c_str());
				if (const auto p = AccessPtr<bool>(node))
					return *p ? "[bool] #t" : "[bool] #f";
				if (const auto p = AccessPtr<int>(node))
					return sfmt("[int] %d", *p);
				if (const auto p = AccessPtr<unsigned>(node))
					return sfmt("[uint] %u", *p);
				if (const auto p = AccessPtr<double>(node))
					return sfmt("[double] %lf", *p);

				const auto& v(node.Value);
				const auto& t(v.GetType());

				if (t != leo::type_id<void>())
					return leo::quote(string(t.name()), '[', ']');
			}
			throw leo::bad_any_cast();
		}());
	});
	TraceDe(lv, "%s", oss.str().c_str());
}

void
LogTermValue(const TermNode& term, Logger::Level lv)
{
	LogTree(term, lv);
}


Interpreter::Interpreter(Application& app,
	std::function<void(REPLContext&)> loader)
	: wc(), err_threshold(RecordLevel(0x10)), line(),
#if LSL_TracePerformDetails
	context(true)
#else
	context()
#endif
{
	using namespace std;
	using namespace platform_ex;

	wc.UpdateForeColor(TitleColor);
	cout << title << endl << "Initializing...";
	//p_env.reset(new Environment(app));
	loader(context);
	cout << "LSL initialization OK!" << endl << endl;
	wc.UpdateForeColor(InfoColor);
	cout << "Type \"exit\" to exit,"
		" \"cls\" to clear screen, \"help\", \"about\", or \"license\""
		" for more information." << endl << endl;
}

Interpreter::~Interpreter()
{
}

void
Interpreter::HandleSignal(SSignal e)
{
	using namespace std;

	static lconstexpr auto not_impl("Sorry, not implemented: ");

	switch(e)
	{
	case SSignal::ClearScreen:
		wc.Clear();
		break;
	case SSignal::About:
		cout << not_impl << "About" << endl;
		break;
	case SSignal::Help:
		cout << not_impl << "Help" << endl;
		break;
	case SSignal::License:
		cout << not_impl << "License" << endl;
		break;
	default:
		LAssert(false, "Wrong command!");
	}
}

bool
Interpreter::Process()
{
	using namespace platform_ex;

	if(!line.empty())
	{
		wc.UpdateForeColor(SideEffectColor);
		try
		{
			line = DecodeArg(line);

			const auto res(context.Perform(line));

#if LSL_TracePerform
		//	wc.UpdateForeColor(InfoColor);
		//	cout << "Unrecognized reduced token list:" << endl;
			wc.UpdateForeColor(ReducedColor);
			LogTermValue(res);
#endif
		}
		catch(SSignal e)
		{
			if(e == SSignal::Exit)
				return {};
			wc.UpdateForeColor(SignalColor);
			HandleSignal(e);
		}
		CatchExpr(LSLException& e, PrintError(wc, e, "LSLException"))
		catch(LoggedEvent& e)
		{
			if(e.GetLevel() < err_threshold)
				throw;
			PrintError(wc, e);
		}
	}
	return true;
}

std::istream&
Interpreter::WaitForLine()
{
	return WaitForLine(std::cin, std::cout);
}
std::istream&
Interpreter::WaitForLine(std::istream& is, std::ostream& os)
{
	wc.UpdateForeColor(PromptColor);
	os << prompt;
	wc.UpdateForeColor(DefaultColor);
	return std::getline(is, line);
}

} // namespace scheme;

