/*!	\file LSLContext.h
\ingroup LTest
\brief LSL 上下文。
\par 修改时间:
	2018-05-26 14:14 +0800
*/


#ifndef LTest_LScheme_LSLContext_h_
#define LTest_LScheme_LSLContext_h_ 1

#include <LScheme/SContext.h>
#include <LScheme/LScheme.h>
#include <LScheme/LSchemREPL.h>

namespace scheme
{

namespace v1
{

LiteralPasses::HandlerType
	FetchExtendedLiteralPass();

} // namespace v1;

} // namespace scheme;

#endif

