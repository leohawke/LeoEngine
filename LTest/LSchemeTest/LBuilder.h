/*!	\file LBuilder.h
\ingroup LTest
\brief LSL 解释实现。
\par 修改时间:
	2018-05-26 13:16 +0800
*/


#ifndef LTest_LScheme_LBuilder_h_
#define LTest_LScheme_LBuilder_h_ 1

#include "Interpreter.h"

namespace scheme
{

/// 674
using namespace leo;

namespace v1
{

/// 674
void
RegisterLiteralSignal(ContextNode&, const string&, SSignal);

} // namespace v1;

} // namespace scheme;

#endif

