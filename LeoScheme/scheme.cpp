#include "scheme.h"
#include <exception>
#include <stdexcept>
namespace leo
{
	//与SEXP有关函数
	namespace scheme{
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
	}

	//全局变量
	namespace scheme{
		extern scheme_list scheme_nil = nullptr;
	}

	//运行时所需辅助函数
	namespace scheme{
		//error-impl
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
		void error(const scheme_string& msg, Lists&... lists)
		{
			throw std::runtime_error(msg + error_list(lists...));
		}
	}

	//求值函数声明
	namespace scheme{
		//error-impl
		scheme_value lookup_variable_value(const scheme_value& exp, scheme_list& env);
		//error-impl
		scheme_value eval_assignment(const scheme_value& exp, scheme_list& env);
		//error-impl
		scheme_value eval_definition(const scheme_value& exp, scheme_list& env);
		//impl
		scheme_value eval_if(const scheme_value& exp, scheme_list& env);
		//no-impl
		scheme_value eval_sequence(const scheme_value& exp, scheme_list& env);
	}

	//判断函数声明及定义
	namespace scheme{
		bool is_list(const scheme_value& value){
			if (value.can_cast<scheme_atom>())
				return false;
			auto list = value.cast_list();
#if 0
			
			if (list == scheme_nil)
				return false;
			else
				return is_list(list->mCdr);
#else
			std::uint32_t count = 0;
			for (;;){
				if (list == scheme_nil)
					return true;
				if (list->mCdr.can_cast<scheme_atom>())
					return false;
				list = list->mCdr.cast_list();
				++count;
				if (count == std::uint32_t(-1))
					error("发现循环引用,程序退出");
			}
#endif
		}

		bool pair(const scheme_value& value){
			return value.can_cast<scheme_list>();
		}

		bool tagged_list(const scheme_value& exp, const scheme_string& tag){
			if (pair(exp))
				return car(exp).cast_atom().to_word() == tag;
			else
				return false;
		}

		bool self_evaluating(const scheme_value& exp){
			return exp.can_cast<scheme_atom>() && exp.cast_atom().no_word();
		}

		bool symbol(const scheme_value& exp){
			return exp.can_cast<scheme_atom>() && exp.cast_atom().word();
		}

		bool variable(const scheme_value& exp){
			return symbol(exp);
		}

		//no-impl
		bool quoted(const scheme_list& exp);

		bool lambda(const scheme_list& exp){
			tagged_list(exp, "lambda");
		}
		//no-impl
		bool begin(const scheme_list& exp);
		//no-impl
		bool cond(const scheme_list& exp);

		bool assignment(const scheme_value& exp){
			tagged_list(exp, "set!");
		}

		bool definition(const scheme_value& exp){
			tagged_list(exp, "define");
		}

		bool ifexp(const scheme_value& exp){
			tagged_list(exp, "if");
		}

		bool application(const scheme_value& exp){
			return pair(exp);
		}

		bool no_operands(const scheme_value& exp){
			return null(exp);
		}
	}

	//选择和构造函数函数声明及定义
	namespace scheme{
		scheme_list list_of_values(const scheme_value& operands, scheme_list& env);

		scheme_value oper(const scheme_value& exp){
			return car(exp);
		}
		scheme_value operands(const scheme_value& exp){
			return cdr(exp);
		}

		scheme_value first_operand(const scheme_value& ops){
			return car(ops);
		}

		scheme_value rest_operands(const scheme_value& ops){
			return cdr(ops);
		}

		scheme_value assignment_variable(const scheme_value& exp){
			return cadr(exp);
		}

		scheme_value assignment_value(const scheme_value& exp){
			return caddr(exp);
		}

		scheme_value lambda_parameters(const scheme_value& exp){
			return cadr(exp);
		}

		scheme_value lambda_body(const scheme_value& exp){
			return cddr(exp);
		}

		scheme_value make_lambda(const scheme_value& parameters, const scheme_value& body){
			const static scheme_value lambda_word{ scheme_atom("lambda",scheme_atom::atom_t::atom_word) };
			return cons(lambda_word, cons(parameters, body));
		}

		scheme_value definition_variable(const scheme_value& exp){
			if (symbol(cadr(exp)))
				return cadr(exp);
			else
				return caadr(exp);
		}

		scheme_value definition_value(const scheme_value& exp){
			if (symbol(cadr(exp)))
				return cadddr(exp);
			else
				return make_lambda(cdadr(exp), cddr(exp));
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
				return scheme_value(scheme_atom(false));
		}

		scheme_value make_if(const scheme_value& predicate, const scheme_value& consequent, const scheme_value& alternative){
			const static scheme_value if_word{ scheme_atom("if", scheme_atom::atom_t::atom_word) };
			return list(if_word, predicate, consequent, alternative);
		}
	}

	

	//eval,apply定义
	namespace scheme
	{
		scheme_value apply(const scheme_value& procedure,const scheme_list& arguments){
			return apply(procedure.cast_list(), arguments);
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


		scheme_value lookup_variable_value(const scheme_value& exp, scheme_list& env)
		{
			return scheme_value(scheme_atom(static_cast<scheme_int>(0)));
		}

		scheme_value eval_assignment(const scheme_value& exp, scheme_list& env){
			return scheme_value(scheme_nil);
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


		scheme_value eval_if(const scheme_value& exp, scheme_list& env){
			if (true_exp(eval(if_predicate(exp), env)))
				return eval(if_consequent(exp), env);
			else
				return eval(if_alternative(exp), env);
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

	//求值函数定义
	namespace scheme{
		scheme_list list_of_values(const scheme_value& exps, scheme_list& env){
			if (no_operands(exps))
				return scheme_nil;
			else
				cons(eval(first_operand(exps), env), list_of_values(rest_operands(exps), env));
		}
	}
}