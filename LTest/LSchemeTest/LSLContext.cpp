/*!	\file LSLContext.cpp
\ingroup Adaptor
\brief LSL 上下文。
\par 修改时间:
	2018-05-26 14:14 +0800
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

		LiteralPasses::HandlerType
			FetchExtendedLiteralPass()
		{
			return [](TermNode& term, ContextNode&, string_view id) -> ReductionStatus {
				LAssertNonnull(id.data());
				if (!id.empty())
				{
					const char f(id.front());

					// NOTE: Handling extended literals.
					if ((f == '#' || f == '+' || f == '-') && id.size() > 1)
					{
						// TODO: Support numeric literal evaluation passes.
						if (id == "#t" || id == "#true")
							term.Value = true;
						else if (id == "#f" || id == "#false")
							term.Value = false;
						else if (id == "#n" || id == "#null")
							term.Value = nullptr;
						else if (id == "+inf.0")
							term.Value = std::numeric_limits<double>::infinity();
						else if (id == "-inf.0")
							term.Value = -std::numeric_limits<double>::infinity();
						else if (id == "+inf.f")
							term.Value = std::numeric_limits<float>::infinity();
						else if (id == "-inf.f")
							term.Value = -std::numeric_limits<float>::infinity();
						else if (id == "+inf.t")
							term.Value = std::numeric_limits<long double>::infinity();
						else if (id == "-inf.t")
							term.Value = -std::numeric_limits<long double>::infinity();
						else if (id == "+nan.0")
							term.Value = std::numeric_limits<double>::quiet_NaN();
						else if (id == "-nan.0")
							term.Value = -std::numeric_limits<double>::quiet_NaN();
						else if (id == "+nan.f")
							term.Value = std::numeric_limits<float>::quiet_NaN();
						else if (id == "-nan.f")
							term.Value = -std::numeric_limits<float>::quiet_NaN();
						else if (id == "+nan.t")
							term.Value = std::numeric_limits<long double>::quiet_NaN();
						else if (id == "-nan.t")
							term.Value = -std::numeric_limits<long double>::quiet_NaN();
						else if (f != '#')
							return ReductionStatus::Retrying;
					}
					else if (std::isdigit(f))
					{
						errno = 0;

						const auto ptr(id.data());
						char* eptr;
#if 0
						const auto ans(std::strtod(ptr, &eptr));
						const auto idx{ size_t(eptr - ptr) };

						if (idx == id.size() && errno != ERANGE)
							term.Value = ans;
#else
						const long ans(std::strtol(ptr, &eptr, 10));

						if (size_t(eptr - ptr) == id.size() && errno != ERANGE)
							term.Value = int(ans);
#endif
					}
					else
						return ReductionStatus::Retrying;
				}
				return ReductionStatus::Clean;
			};
		}

	} // namespace v1;

} // namespace scheme;

