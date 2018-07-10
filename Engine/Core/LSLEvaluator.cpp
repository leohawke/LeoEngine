#include "LSLEvaluator.h"
#include "LSLBuilder.h"

namespace platform {
	using namespace scheme;
	using namespace v1;


	void
		LoadSequenceSeparators(EvaluationPasses& passes)
	{
		RegisterSequenceContextTransformer(passes, TokenValue(";"), true),
			RegisterSequenceContextTransformer(passes, TokenValue(","));
	}

	LSLEvaluator::LSLEvaluator(std::function<void(REPLContext&)> loader)
	:
#ifdef NDEBUG
		context()
#else
		context(true)
#endif
	{
		auto& root(context.Root);

		//LoadSequenceSeparators(context.ListTermPreprocess),

		root.EvaluateLiteral
			= [](TermNode& term, ContextNode&, string_view id) -> ReductionStatus {
			LAssertNonnull(id.data());
			if (!id.empty())
			{
				const char f(id.front());

				// NOTE: Handling extended literals.
				if (IsLSLAExtendedLiteralNonDigitPrefix(f) && id.size() > 1)
				{
					// TODO: Support numeric literal evaluation passes.
					if (id == "#t" || id == "#true")
						term.Value = true;
					else if (id == "#f" || id == "#false")
						term.Value = false;
					else if (id == "#n" || id == "#null")
						term.Value = nullptr;
					// XXX: Redundant test?
					else if (IsLSLAExtendedLiteral(id))
						throw InvalidSyntax(f == '#' ? "Invalid literal found."
							: "Unsupported literal prefix found.");
					else
						return ReductionStatus::Retrying;
				}
				else if (std::isdigit(f))
				{
					errno = 0;

					const auto ptr(id.data());
					char* eptr;
					const long ans(std::strtol(ptr, &eptr, 10));

					if (size_t(eptr - ptr) == id.size() && errno != ERANGE)
						// XXX: Conversion to 'int' might be implementation-defined.
						term.Value = int(ans);
					// TODO: Supported literal postfix?
					else
						throw InvalidSyntax("Literal postfix is unsupported.");
				}
				else
					return ReductionStatus::Retrying;
			}
			return ReductionStatus::Clean;
		};

		root.EvaluateLiteral = lsl::context::FetchNumberLiteral();

		lsl::math::RegisterTypeLiteralAction(context);

		loader(context);
	}

	ImplDeDtor(LSLEvaluator)
}