/*!	\file LSLContext.cpp
\ingroup Adaptor
\brief LSL 上下文。
\par 修改时间:
	2016-11-13 17:52 +0800
*/

#include "LSLContext.h"
#include <LBase/container.hpp>
#include <iostream>
#include <LScheme/LScheme.h>
/// 674
using namespace leo;
/// 676
using namespace std::placeholders;

namespace scheme
{

namespace v1
{

void
LoadSequenceSeparators(ContextNode& ctx, EvaluationPasses& passes)
{
	RegisterSequenceContextTransformer(passes, ctx, "$;", string(";")),
	RegisterSequenceContextTransformer(passes, ctx, "$,", string(","));
}

void
LoadDeafultLiteralPasses(ContextNode& ctx)
{
	AccessLiteralPassesRef(ctx)
		= [](TermNode& term, ContextNode&, string_view id) -> bool{
		LAssertNonnull(id.data());
		if(!id.empty())
		{
			const char f(id.front());

			// NOTE: Handling extended literals.
			if((f == '#'|| f == '+' || f == '-') && id.size() > 1)
				;
			else if(std::isdigit(f))
			{
				errno = 0;

				const auto ptr(id.data());
				char* eptr;
				const long ans(std::strtol(ptr, &eptr, 10));

				if(size_t(eptr - ptr) == id.size() && errno != ERANGE)
					term.Value = int(ans);
			}
			else
				return true;
		}
		return {};
	};
}

} // namespace v1;

} // namespace scheme;

