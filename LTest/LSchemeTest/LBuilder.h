/*!	\file LBuilder.h
\ingroup LTest
\brief LSL 解释实现。
\par 修改时间:
	2016-11-13 04:05 +0800
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


/// 696
//@{
template<typename _func>
void
DoIntegerBinaryArithmetics(_func f, TermNode& term)
{
	Forms::QuoteN(term, 2);

	auto i(term.begin());
	const int e1(Access<int>(Deref(++i)));

	// TODO: Remove 'to_string'?
	term.Value = to_string(f(e1, Access<int>(Deref(++i))));
}

template<typename _func>
void
DoIntegerNAryArithmetics(_func f, int val, TermNode& term)
{
	const auto n(Forms::FetchArgumentN(term));
	auto i(term.begin());
	const auto j(leo::make_transform(++i, [](TNIter i){
		return Access<int>(Deref(i));
	}));

	// FIXME: Overflow?
	term.Value = to_string(std::accumulate(j, std::next(j, n), val, f));
}
//@}

} // namespace v1;

} // namespace scheme;

#endif

