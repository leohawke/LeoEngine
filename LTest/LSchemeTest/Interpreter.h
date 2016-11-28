/*!	\file Interpreter.h
\ingroup LTest
\brief LScheme 解释器。
\par 修改时间:
	2016-11-13 17:49 +0800
*/


#ifndef LTEST_LScheme_Interpreter_h_
#define LTEST_LScheme_Interpreter_h_ 1

#include "LSLContext.h"
#include <LBase/Win32/Consoles.h>
#include <LBase/LConsole.h>
#include "LApplication.h"
#include <iosfwd>
#include <functional>

namespace scheme
{

/// 592
using namespace leo::Consoles;
/// 674
using leo::Logger;
/// 740
using v1::REPLContext;

/// 304
enum class SSignal
{
	Exit,
	ClearScreen,
	About,
	Help,
	License
};


/// 673
void
LogTree(const ValueNode&, Logger::Level = leo::Debug);


/*!
\build 控制台默认颜色。
*/
lconstexpr Color DefaultColor(Gray), TitleColor(Cyan),
	InfoColor(White), ErrorColor(Red), PromptColor(DarkGreen),
	SignalColor(DarkRed), SideEffectColor(Yellow), ReducedColor(Magenta);


/// 304
class Interpreter
{
private:
	/// 520
	platform_ex::Windows::WConsole wc;
	/// 674
	leo::RecordLevel err_threshold;
	/// 689
	//leo::unique_ptr<leo::Environment> p_env;
	/// 674
	string line;
	/// 740
	REPLContext context;

public:
	/// 740
	Interpreter(leo::Application&, std::function<void(REPLContext&)>);

	void
	HandleSignal(SSignal);

	bool
	Process();

	std::istream&
	WaitForLine();
	/// 696
	std::istream&
	WaitForLine(std::istream&, std::ostream&);
};

} // namespace NPL;

#endif

