#include "Configuration.h"
#include "SContext.h"
#include <LBase/ios.hpp>

namespace scheme {
	std::ostream&
		operator<<(std::ostream& os, const Configuration& conf)
	{
		PrintNode(os, conf.GetRoot(), LiteralizeEscapeNodeLiteral);
		return os;
	}

	std::istream&
		operator >> (std::istream& is, Configuration& conf)
	{
		using sb_it_t = std::istreambuf_iterator<char>;
		// TODO: Validate for S-expression?
		Session sess(sb_it_t(is), sb_it_t{});

		TryExpr(conf.root = v1::LoadNode(SContext::Analyze(std::move(sess))))
			CatchExpr(..., leo::rethrow_badstate(is, std::ios_base::failbit))
			return is;
	}
}