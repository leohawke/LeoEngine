#include "SContext.h"

using namespace leo;

namespace scheme
{

	void
		Session::DefaultParseByte(LexicalAnalyzer& lexer, char c)
	{
		lexer.ParseByte(c);
	}

	void
		Session::DefaultParseQuoted(LexicalAnalyzer& lexer, char c)
	{
		lexer.ParseQuoted(c);
	}


	namespace SContext
	{

		TLCIter
			Validate(TLCIter b, TLCIter e)
		{
			while (b != e && *b != ")")
				if (*b == "(")
				{
					// FIXME: Potential overflow.
					auto res(Validate(++b, e));

					if (res == e || *res != ")")
						throw LoggedEvent("Redundant '(' found.", Alert);
					b = ++res;
				}
				else
					++b;
			return b;
		}

		TLCIter
			Reduce(TermNode& term, TLCIter b, TLCIter e)
		{
			while (b != e && *b != ")")
				if (*b == "(")
				{
					// FIXME: Potential overflow.
					// NOTE: Explicit type 'TermNode' is intended.
					TermNode tm(AsIndexNode(term));
					auto res(Reduce(tm, ++b, e));

					if (res == e || *res != ")")
						throw LoggedEvent("Redundant '(' found.", Alert);
					term += std::move(tm);
					b = ++res;
				}
				else
					term += AsIndexNode(term, *b++);
			return b;
		}

		void
			Analyze(TermNode& root, const TokenList& token_list)
		{
			if (Validate(token_list.cbegin(), token_list.cend()) != token_list.cend())
				throw LoggedEvent("Redundant ')' found.", Alert);

			const auto res(Reduce(root, token_list.cbegin(), token_list.cend()));

			lassume(res == token_list.end());
		}
		void
			Analyze(TermNode& root, const Session& session)
		{
			Analyze(root, session.GetTokenList());
		}

	} // namespace SContext;

} // namespace scheme;
