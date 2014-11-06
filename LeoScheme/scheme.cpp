#include "scheme.h"
#include <exception>
#include <stdexcept>
namespace leo
{
	namespace scheme
	{
		extern scheme_list scheme_nil = nullptr;

		scheme_list make_copy(sexp::sexp_list sexp_list)
		{
			if (!sexp_list)
				return scheme_nil;
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

		bool self_evaluating(const scheme_value& exp);

		bool variable(const scheme_value& exp);
		scheme_value lookup_variable_value(const scheme_value& exp, scheme_list& env);

		//bool quoted(const scheme_list& exp);
		bool assignment(const scheme_value& exp);
		scheme_value eval_assignment(const scheme_value& exp, scheme_list& env);

		bool definition(const scheme_value& exp);
		scheme_value eval_definition(const scheme_value& exp, scheme_list& env);

		bool ifexp(const scheme_value& exp);
		scheme_value eval_if(const scheme_value& exp, scheme_list& env);

		scheme_value eval_sequence(const scheme_value& exp, scheme_list& env);
		//bool lambda(const scheme_list& exp);
		//bool begin(const scheme_list& exp);
		//bool cond(const scheme_list& exp);
		bool application(const scheme_value& exp);

		scheme_value apply(const scheme_value& procedure,const scheme_list& arguments){
			return apply(procedure.cast_list(), arguments);
		}

		scheme_value oper(const scheme_value& exp);
		scheme_value operands(const scheme_value& exp);

		scheme_list list_of_values(const scheme_value& operands, scheme_list& env);

		scheme_string error_list(const scheme_list& exp)
		{
			return scheme_string("()");
		}

		scheme_string error_list()
		{
			return scheme_string();
		}

		template<typename... Lists>
		scheme_string error_list(const scheme_list& exp, Lists&... lists)
		{
			return error_list(exp) + error_list(lists...);
		}

		template<typename... Lists>
		void error(const scheme_string& msg,Lists&... lists)
		{
			throw std::runtime_error(msg+error_list(lists...));
		}

		scheme_value eval(const scheme_value& exp,scheme_list& env)
		{
			if (self_evaluating(exp))
				return exp;
			if (variable(exp))
				return lookup_variable_value(exp, env);
			if (assignment(exp))
				return eval_assignment(exp, env);
			if (definition(exp))
				return eval_definition(exp, env);
			if (ifexp(exp))
				return eval_if(exp, env);
			if (application(exp))
				return apply(eval(oper(exp), env), list_of_values(operands(exp), env));
			return scheme_nil;
		}

		

		bool self_evaluating(const scheme_value& exp)
		{
			if (exp.mType == scheme_value::any_t::any_atom && exp.cast_atom().no_word())
				return true;
			return false;
		}

		bool variable(const scheme_value& exp)
		{
			if (exp.mType == scheme_value::any_t::any_atom && !exp.cast_atom().no_word())
				return true;
			return false;
		}

		scheme_value lookup_variable_value(const scheme_value& exp, scheme_list& env)
		{
			return scheme_value(scheme_atom(static_cast<scheme_int>(0)));
		}

		bool assignment(const scheme_value& exp)
		{
			if (exp.mType == scheme_value::any_t::any_list){
				auto list = exp.cast_list();
				if (list->mCar.mType != scheme_value::any_t::any_atom)
					return false;
				auto atom = list->mCar.cast_atom();
				return atom.mType == scheme_atom::atom_t::atom_word &&  atom.cast<scheme_string>() == "set!";
			}
			return false;
		}

		bool ifexp(const scheme_value& exp){
			if (exp.mType == scheme_value::any_t::any_list){
				auto list = exp.cast_list();
				if (list->mCar.mType != scheme_value::any_t::any_atom)
					return false;
				auto atom = list->mCar.cast_atom();
				return atom.mType == scheme_atom::atom_t::atom_word &&  atom.cast<scheme_string>() == "if";
			}
			return false;
		}

		scheme_value eval_assignment(const scheme_value& exp, scheme_list& env){
			return scheme_value(scheme_nil);
		}

		bool definition(const scheme_value& exp){
			if (exp.mType == scheme_value::any_t::any_list){
				auto list = exp.cast_list();
				if (list->mCar.mType != scheme_value::any_t::any_atom)
					return false;
				auto atom = list->mCar.cast_atom();
				return atom.mType == scheme_atom::atom_t::atom_word &&  atom.cast<scheme_string>() == "define";
			}
			return false;
		}

		scheme_value eval_definition(const scheme_value& exp, scheme_list& env){
			return scheme_value(scheme_nil);
		}


		bool true_exp(const scheme_value& exp){
			if (exp.mType == scheme_value::any_t::any_list)
				return exp.cast_list() != scheme_nil;
			else
				switch (exp.cast_atom().mType)
				{
				case scheme_atom::atom_t::atom_bool:
					return exp.cast_atom().cast<scheme_bool>();
				case scheme_atom::atom_t::atom_char:
					return exp.cast_atom().cast<scheme_char>();
				case scheme_atom::atom_t::atom_int:
					return exp.cast_atom().cast<scheme_int>();
				case scheme_atom::atom_t::atom_real:
					return exp.cast_atom().cast<scheme_real>();
				case scheme_atom::atom_t::atom_word:
				case scheme_atom::atom_t::atom_string:
					return exp.cast_atom().cast<scheme_string>() != "false";
				}
			return false;
		}

		scheme_value if_predicate(const scheme_value& exp){
			return cadr(exp);
		}

		scheme_value if_consequent(const scheme_value& exp){
			return caddr(exp);
		}

		scheme_value if_alternative(const scheme_value& exp){
			if (!null(cdddr(exp)))
				return cadddr(exp);
			else
				return scheme_value(scheme_atom("false", scheme_atom::atom_t::atom_word));
		}

		scheme_value eval_if(const scheme_value& exp, scheme_list& env){
			if (true_exp(eval(if_predicate(exp), env)))
				return eval(if_consequent(exp), env);
			else
				return eval(if_alternative(exp), env);
		}

		scheme_value oper(const scheme_value& exp){
			return car(exp);
		}
		scheme_value operands(const scheme_value& exp){
			return cdr(exp);
		}

		bool application(const scheme_value& exp){
			return exp.mType == scheme_value::any_t::any_list;
		}

		scheme_list list_of_values(const scheme_value& operands, scheme_list& env){
			return scheme_nil;
		}

		bool primitive_procedure(const scheme_list& procedure);

		scheme_value apply_primitive_procedure(const scheme_list& procedure, const scheme_list& arguments);

		bool compound_procedure(const scheme_list& procedure);

		scheme_value procedure_body(const scheme_list& procedure);
		
		scheme_list procedure_parameters(const scheme_list& procedure);

		scheme_list procedure_environment(const scheme_list& procedure);

		scheme_list extend_environment(const scheme_list& parameters, const scheme_list& arguments, scheme_list& env);

		scheme_value apply(const scheme_list& procedure, const scheme_list& arguments){
			if (primitive_procedure(procedure))
				return apply_primitive_procedure(procedure, arguments);
			else if (compound_procedure(procedure))
				return eval_sequence(procedure_body(procedure), extend_environment(procedure_parameters(procedure), arguments, procedure_environment(procedure)));
			error("Unknown procedure type -- APPLY", procedure);
			return scheme_value(scheme_nil);
		}

		
	}
}