#include "LSLBuilder.h"
#include <LBase/lmathtype.hpp>
#include <charconv>
#include <cmath>

using namespace scheme;
using namespace v1;

namespace platform::lsl::context {
	ReductionStatus NormalNumberLiteral(TermNode& term, ContextNode&, string_view id) {
		auto none = std::errc{ 0 };
		auto e(id.back());
		auto f(id.front());
		if (e == 'u') {
			leo::uint32 uint32_ans{};
			auto ret = std::from_chars(id.data(), id.data() + id.size() - 1, uint32_ans);
			if (ret.ec == none) {
				term.Value = uint32_ans;
				return ReductionStatus::Clean;
			}
			else if (ret.ec == std::errc::result_out_of_range) {
				leo::uint64 uint64_ans{};
				auto ret = std::from_chars(id.data(), id.data() + id.size() - 1, uint64_ans);
				if (ret.ec == std::errc::result_out_of_range)
					throw InvalidSyntax("Literal number is too large.");
				else if (ret.ec == none) {
					term.Value = uint64_ans;
					return ReductionStatus::Clean;
				}
			}
		}
		else if (e == 'f') {
#if 1
			errno = 0;
			const auto ptr(id.data());
			char* eptr = const_cast<char*>(id.data()) + id.size() - 1;

			const float f_ans(std::strtof(ptr, &eptr));

			if (size_t(eptr - ptr) == id.size() && errno != ERANGE) {
				term.Value = f_ans;
				return ReductionStatus::Clean;
			}
			else if (errno == ERANGE) {
				errno = 0;
				throw InvalidSyntax("Literal number is too large.");
			}
#else
			float f_ans{};
			auto ret = std::from_chars(id.data(), id.data() + id.size() - 1, f_ans);
			if (ret.ec == none) {
				term.Value = f_ans;
				return ReductionStatus::Clean;
			}
#endif
		}
		else if (std::isdigit(f)) {
			leo::int32 int32_ans{};
			auto ret = std::from_chars(id.data(), id.data() + id.size(), int32_ans);
			if (ret.ec == none) {
				term.Value = int32_ans;
				return ReductionStatus::Clean;
			}
			else if (ret.ec == std::errc::result_out_of_range) {
				leo::int64 int64_ans{};
				auto ret = std::from_chars(id.data(), id.data() + id.size() - 1, int64_ans);
				if (ret.ec == std::errc::result_out_of_range)
					throw InvalidSyntax("Literal number is too large.");
				else if (ret.ec == none) {
					term.Value = int64_ans;
					return ReductionStatus::Clean;
				}
			}
			else {
#if 1
				errno = 0;
				const auto ptr(id.data());
				char* eptr;

				const double d_ans(std::strtof(ptr, &eptr));

				if (size_t(eptr - ptr) == id.size() && errno != ERANGE) {
					term.Value = d_ans;
					return ReductionStatus::Clean;
				}
				else if (errno == ERANGE) {
					errno = 0;
					throw InvalidSyntax("Literal number is too large.");
				}
#else
				double d_ans;
				auto ret = std::from_chars(id.data(), id.data() + id.size(), d_ans);
				if (ret.ec == none) {
					term.Value = d_ans;
					return ReductionStatus::Clean;
				}
				else if (ret.ec == std::errc::result_out_of_range)
					throw InvalidSyntax("Literal number is too large.");
#endif
			}
		}
		return ReductionStatus::Retrying;
	}


	LiteralPasses::HandlerType FetchNumberLiteral()
	{
		return [](TermNode& term, ContextNode& ctx, string_view id) -> ReductionStatus {
			LAssertNonnull(id.data());
			if (!id.empty())
			{
				auto res(NormalNumberLiteral(term, ctx, id));
				if (res == ReductionStatus::Clean)
					return res;

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
				else
					return ReductionStatus::Retrying;
			}
			return ReductionStatus::Clean;
		};
	}
}


namespace platform::lsl::math {

	using namespace Forms;

	template<typename _scalar>
	_scalar AccessToScalar(TermNode& term) {
		return platform::lsl::access::static_value_cast<_scalar, float, leo::int32, leo::uint32, leo::int64, leo::uint64>(term);
	}

	namespace details {
		template<typename _scalar, size_t multi>
		ReductionStatus TypeLiteralAction(TermNode& term, leo::math::data_storage<_scalar, multi>& ans) {
			const auto size(term.size());

			if (size > 1 && size < multi + 2) {
				auto i = std::next(term.begin());
				for (auto j = ans.begin(); i != term.end() && j != ans.end(); ++i, ++j)
					*j = platform::lsl::access::static_value_cast<_scalar, float, leo::int32, leo::uint32, leo::int64, leo::uint64>(*i);
			}
			else {
				throw  std::invalid_argument(leo::sfmt(
					"Invalid parameter count(>1 && < %u):%u.", multi + 2, size).c_str());
			}

			return ReductionStatus::Clean;
		}
	}

	template<typename _type>
	ReductionStatus TypeLiteralAction(TermNode & term)
	{
		_type ans = {};
		auto res = details::TypeLiteralAction(term, ans);
		term.Value = ans;
		return res;
	}

	void RegisterTypeLiteralAction(REPLContext & context)
	{
		auto& root(context.Root);

		RegisterStrict(root, "float3", TypeLiteralAction<leo::math::float3>);
	}

	namespace details {
		template<template<typename, typename> typename _functor>
		ReductionStatus Binary(TermNode& term, TermNode& argument)
		{
			if (
				_functor<float, leo::int32>()(term, argument) ||
				_functor<float, leo::uint32>()(term, argument) ||
				_functor<float, leo::int64>()(term, argument) ||
				_functor<float, leo::uint64>()(term, argument) ||
				_functor<float, float>()(term, argument) ||
				_functor<leo::int32, leo::int32>()(term, argument) ||
				_functor<leo::int32, leo::uint32>()(term, argument) ||
				_functor<leo::int32, leo::int64>()(term, argument) ||
				_functor<leo::int32, leo::uint64>()(term, argument) ||
				_functor<leo::int32, float>()(term, argument) ||
				_functor<leo::uint32, leo::int32>()(term, argument) ||
				_functor<leo::uint32, leo::uint32>()(term, argument) ||
				_functor<leo::uint32, leo::int64>()(term, argument) ||
				_functor<leo::uint32, leo::uint64>()(term, argument) ||
				_functor<leo::uint32, float>()(term, argument) ||
				_functor<leo::int64, leo::int32>()(term, argument) ||
				_functor<leo::int64, leo::uint32>()(term, argument) ||
				_functor<leo::int64, leo::int64>()(term, argument) ||
				_functor<leo::int64, leo::uint64>()(term, argument) ||
				_functor<leo::int64, float>()(term, argument) ||
				_functor<leo::uint64, leo::int32>()(term, argument) ||
				_functor<leo::uint64, leo::uint32>()(term, argument) ||
				_functor<leo::uint64, leo::int64>()(term, argument) ||
				_functor<leo::uint64, leo::uint64>()(term, argument) ||
				_functor<leo::uint64, float>()(term, argument))
				return ReductionStatus::Clean;
			else
				throw std::invalid_argument("type error");
		}

		template<typename _1, typename _2, typename _functor>
		struct BinaryFunctor {
			bool operator()(TermNode& term, TermNode& argument) {
				auto p_1 = leo::AccessPtr<_1>(term);
				auto p_2 = leo::AccessPtr<_2>(argument);
				if (p_1 && p_2) {
					term.Value =static_cast<leo::common_type_t<_1,_2>>(_functor()(*p_1, *p_2));
					return true;
				}
				return false;
			}
		};

		template<typename _1, typename _2>
		struct MulFunctor :BinaryFunctor<_1, _2, leo::multiplies<>> {
		};

		template<typename _1, typename _2>
		struct AddFunctor :BinaryFunctor<_1, _2, leo::plus<>> {
		};

		template<typename _1, typename _2>
		struct SubFunctor :BinaryFunctor<_1, _2, leo::minus<>> {
		};

		template<typename _1, typename _2>
		struct DivFunctor{
			bool operator()(TermNode& term, TermNode& argument) {
				term.Value = AccessToScalar<float>(term) / AccessToScalar<float>(argument);
				return true;
			}
		};

		template<typename _1, typename _2,typename = std::void_t<>>
		struct ModFunctor {
			bool operator()(TermNode& term, TermNode& argument) {
				auto p_1 = leo::AccessPtr<_1>(term);
				auto p_2 = leo::AccessPtr<_2>(argument);
				if (p_1 && p_2) {
					term.Value = fmodf(AccessToScalar<float>(term), AccessToScalar<float>(argument));
					return true;
				}
				return false;
			}
		};

		template<typename _1, typename _2>
		struct ModFunctor<_1,_2, std::void_t<std::enable_if_t<!leo::is_same<_1, float>::value && !leo::is_same<_2, float>::value>>> {
			bool operator()(TermNode& term, TermNode& argument) {
				auto p_1 = leo::AccessPtr<_1>(term);
				auto p_2 = leo::AccessPtr<_2>(argument);
				if (p_1 && p_2) {
					term.Value = AccessToScalar<leo::common_type_t<_1, _2>>(term) % AccessToScalar<leo::common_type_t<_1, _2>>(argument);
					return true;
				}
				return false;
			}
		};
	}

	ReductionStatus Mul(TermNode& term, TermNode& argument)
	{
		return details::Binary<details::MulFunctor>(term, argument);
	}

	ReductionStatus Add(TermNode& term, TermNode& argument)
	{
		return details::Binary<details::AddFunctor>(term, argument);
	}

	template<template<typename, typename> typename _functor>
	ReductionStatus Binary(TermNode& term) {
		const auto n(FetchArgumentN(term));
		if (n == 2) {
			auto i(std::next(term.begin()));
			term.Value = i->Value;
			return details::Binary<_functor>(term, *std::next(i));
		}
		else
			throw  std::invalid_argument(leo::sfmt(
				"Invalid parameter count  %u != 2.", n).c_str());
	}

	ReductionStatus Sub(TermNode& term)
	{
		return Binary<details::SubFunctor>(term);
	}

	ReductionStatus Div(TermNode& term)
	{
		return Binary<details::DivFunctor>(term);
	}

	ReductionStatus Mod(TermNode& term)
	{
		return Binary<details::ModFunctor>(term);
	}

	template<typename _func, typename _type>
	ReductionStatus BinaryFold(_func f, _type val, TermNode & term)
	{
		term.Value = val;
		const auto n(FetchArgumentN(term));
		auto i(std::next(term.begin()));
		auto j(std::next(i, typename std::iterator_traits<decltype(i)>::difference_type(n)));
		for (; i != j; ++j)
		{
			auto res(f(term, *i));
			if (res != ReductionStatus::Clean)
				return res;
		}
		return ReductionStatus::Clean;
	}

	template<typename _func>
	decltype(auto) FetchCMathFileUnaryFunction(_func f) {
		return [f = f](TermNode& term) {
			auto i(std::next(term.begin()));
			term.Value =f(AccessToScalar<float>(*i));
			return ReductionStatus::Clean;
		};
	}

	void RegisterCMathFile(REPLContext & context) {
		auto& root(context.Root);

		RegisterStrict(root, "sqrt", FetchCMathFileUnaryFunction(std::sqrtf));

#define AMBIGUOUS_SPECIAL(f) [](float v){return f(v);}
		//!\brief Exponential functions 
		//@{
		RegisterStrict(root, "exp", FetchCMathFileUnaryFunction(std::expf));
		RegisterStrict(root, "exp2", FetchCMathFileUnaryFunction(AMBIGUOUS_SPECIAL(std::exp2)));
		RegisterStrict(root, "expm1", FetchCMathFileUnaryFunction(AMBIGUOUS_SPECIAL(std::expm1)));
		RegisterStrict(root, "log", FetchCMathFileUnaryFunction(AMBIGUOUS_SPECIAL(std::log)));
		RegisterStrict(root, "log10", FetchCMathFileUnaryFunction(AMBIGUOUS_SPECIAL(std::log10)));
		RegisterStrict(root, "log2", FetchCMathFileUnaryFunction(AMBIGUOUS_SPECIAL(std::log2)));
		RegisterStrict(root, "loglp", FetchCMathFileUnaryFunction(AMBIGUOUS_SPECIAL(std::log1p)));
		//@}
#undef AMBIGUOUS_SPECIAL
	}

	void RegisterMathDotLssFile(REPLContext & context)
	{
		auto& root(context.Root);
		RegisterStrict(root, "*", [](TermNode& term) {
			return BinaryFold(Mul, 1, term);
		});
		RegisterStrict(root, "+", [](TermNode& term) {
			return BinaryFold(Add, 0, term);
		});
		RegisterStrict(root, "-", Sub);
		RegisterStrict(root, "/", Div);
		RegisterStrict(root, "%", Mod);
	}
}


