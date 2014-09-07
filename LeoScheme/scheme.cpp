#include "scheme.h"
#include <exception>
#include <stdexcept>
namespace leo
{
	namespace scheme
	{
		extern scheme_list nil = nullptr;

		scheme_list make_copy(sexp::sexp_list sexp_list)
		{
			if (!sexp_list)
				return nil;
			auto result = make_scheme_list();
			if (sexp_list->mValue.can_cast<sexp::sexp_list>())
			{
				result->mCar = make_copy(sexp_list->mValue.cast_list());
			}
			else
			{
				result->mCar = scheme_value(sexp_list->mValue);
			}
			result->mCdr = make_copy(sexp_list->mNext);
			return result;
		}

		bool self_evaluating(scheme_value& exp);

		bool variable(scheme_value& exp);
		scheme_value lookup_variable_value(scheme_value& exp, scheme_list& env);
		//bool quoted(scheme_list& exp);
		bool assignment(scheme_value& exp);
		bool definition(scheme_list& exp);
		bool ifexp(scheme_list& exp);
		//bool lambda(scheme_list& exp);
		//bool begin(scheme_list& exp);
		//bool cond(scheme_list& exp);
		bool application(scheme_list& exp);

		scheme_string error_list(scheme_list& exp)
		{
			return scheme_string();
		}

		scheme_string error_list()
		{
			return scheme_string();
		}

		template<typename... Lists>
		scheme_string error_list(scheme_list& exp, Lists&... lists)
		{
			return error_list(exp) + error_list(lists...);
		}

		template<typename... Lists>
		void error(const scheme_string& msg,List&... lists)
		{
			throw std::runtime_error(msg+error_list(lists...));
		}

		scheme_value eval(scheme_value& exp, scheme_list& env)
		{
			if (self_evaluating(exp))
				return exp;
			if (variable(exp))
				lookup_variable_value(exp, env);
			if (assignment(exp))
				;
		}

		void apply(scheme_list& procedure, scheme_list& arguments);

		bool self_evaluating(scheme_value& exp)
		{
			if (exp.mType == scheme_value::any_t::any_atom && any_cast<scheme_atom>(exp.mValue).no_word())
				return true;
			return false;
		}

		bool variable(scheme_value& exp)
		{
			if (exp.mType == scheme_value::any_t::any_atom && !any_cast<scheme_atom>(exp.mValue).no_word())
				return true;
			return false;
		}

		scheme_value lookup_variable_value(scheme_value& exp, scheme_list& env)
		{
			return scheme_value(scheme_atom(static_cast<scheme_int>(0)));
		}

		bool assignment(scheme_value& exp)
		{
			if (exp.mType == scheme_value::any_t::any_atom){
				auto atom = any_cast<scheme_atom*>(&exp.mValue);
				return atom->mType == scheme_atom::atom_t::atom_word && atom->cast<scheme_string>() == "set!";
			}
			return false;
		}
	}
}