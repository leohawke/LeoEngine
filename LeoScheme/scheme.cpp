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

	//运行时所需辅助函数<环境操作,错误处理>
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

		//no-impl
		//修改变量var在环境env里的约束,使得该变量现在约束到值value,如果这一变量没有约束就发出一个错误信号
		void set_variable_value(const scheme_value& var, const scheme_value& value, scheme_list& env);

		//no-impl
		//在环境env的第一个框架里加入一个新约束,它关联起变量var和值value
		void define_variable(const scheme_value& var, const scheme_value& value, scheme_list& env);

		//返回一个新环境,这个环境中包含了一个新的框架,其中所位于表variables的符号约束到表values里队友的元素,而其外围环境是环境base_env
		scheme_list extend_environment(const scheme_value& variables, const scheme_value& values, scheme_list& base_env);

		//error-impl
		//返回符号var在环境env里的约束值,如果这一变量没有约束发出一个错误信号
		scheme_value lookup_variable_value(const scheme_value& var, scheme_list& env);
	}

	//求值函数声明
	namespace scheme{
		
		//impl
		scheme_value eval_assignment(const scheme_value& exp, scheme_list& env);
		//impl
		scheme_value eval_definition(const scheme_value& exp, scheme_list& env);
		//impl
		scheme_value eval_if(const scheme_value& exp, scheme_list& env);
		//impl
		scheme_value eval_sequence(const scheme_value& exps, scheme_list& env);
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
		bool quoted(const scheme_value& exp);

		bool lambda(const scheme_value& exp){
			tagged_list(exp, "lambda");
		}

		bool begin(const scheme_value& exp){
			tagged_list(exp, "begin");
		}
		//no-impl
		bool cond(const scheme_value& exp);

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

		bool last_exp(const scheme_value& exp){
			return null(cdr(exp));
		}

		bool compound_procedure(const scheme_list& procedure){
			return tagged_list(procedure, "procedure");
		}

		//no-impl
		bool primitive_procedure(const scheme_list& procedure);
	}

	//选择和构造函数函数声明及定义
	namespace scheme{
		scheme_value list_of_values(const scheme_value& operands, scheme_list& env);

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

		scheme_value make_procedure(const scheme_value& parameters, const scheme_value& body){
			const static scheme_value procedure_word{ scheme_atom("procedure", scheme_atom::atom_t::atom_word) };
			return list(procedure_word, parameters, body);
		}

		scheme_value begin_actions(const scheme_value& exp){
			return cdr(exp);
		}

		scheme_value first_exp(const scheme_value& seq){
			return car(seq);
		}

		scheme_value rest_exps(const scheme_value& seq){
			return cdr(seq);
		}

		scheme_value list_of_values(const scheme_value& exps, scheme_list& env){
			if (no_operands(exps))
				return scheme_nil;
			else
				cons(eval(first_operand(exps), env), list_of_values(rest_operands(exps), env));
		}

		scheme_value procedure_body(const scheme_value& procedure){
			return caddr(procedure);
		}

		scheme_value procedure_parameters(const scheme_value& procedure){
			return cadr(procedure);
		}

		scheme_list procedure_environment(const scheme_value& procedure){
			return cadddr(procedure).cast_list();
		}

		scheme_value make_begin(const scheme_value& seq){
			const static scheme_value begin_word{ scheme_atom("begin", scheme_atom::atom_t::atom_word) };
			return cons(begin_word,seq);
		}

		scheme_value sequence_exp(const scheme_value& seq){
			if (null(seq))
				return seq;
			else if (last_exp(seq))
				return first_exp(seq);
			else
				make_begin(seq);
		}
	}

	

	//eval,apply定义
	namespace scheme
	{
		scheme_value apply(const scheme_value& procedure, const scheme_value& arguments){
			return apply(procedure.cast_list(), arguments.cast_list());
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
			if (lambda(exp))
				return make_procedure(lambda_parameters(exp), lambda_body(exp));
			if (begin(exp))
				return eval_sequence(begin_actions(exp), env);
			if (application(exp))
				return apply(eval(oper(exp), env), list_of_values(operands(exp), env));
			else
				error("Unknown expression type --EVAL",exp);
			return scheme_nil;
		}

		scheme_value apply_primitive_procedure(const scheme_list& procedure, const scheme_list& arguments);
		

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
		scheme_value eval_sequence(const scheme_value& exps, scheme_list& env){
			if (last_exp(exps))
				return eval(first_exp(exps), env);
			else{
				eval(first_exp(exps), env);
				return eval_sequence(rest_exps(exps), env);
			}
		}

		scheme_value lookup_variable_value(const scheme_value& exp, scheme_list& env)
		{
			return scheme_value(scheme_atom(static_cast<scheme_int>(0)));
		}

		scheme_value eval_assignment(const scheme_value& exp, scheme_list& env){
			set_variable_value(
				assignment_variable(exp),
				eval(assignment_value(exp), env),
				env);
			return scheme_value(scheme_nil);
		}

		scheme_value eval_definition(const scheme_value& exp, scheme_list& env){
			define_variable(
				definition_variable(exp),
				eval(definition_value(exp), env),
				env);
			return scheme_value(scheme_nil);
		}

		scheme_value eval_if(const scheme_value& exp, scheme_list& env){
			if (true_exp(eval(if_predicate(exp), env)))
				return eval(if_consequent(exp), env);
			else
				return eval(if_alternative(exp), env);
		}
	}

	//运行时所需辅助函数<环境操作,错误处理>定义
	namespace scheme{
		scheme_list enclosing_environment(scheme_list& env){
			return cdr(env).cast_list();
		}

		scheme_value first_frame(scheme_list& env){
			return car(env);
		}

		scheme_list the_empty_envirnoment(){
			return make_scheme_list();
		}

		scheme_value make_frame(const scheme_value& variables, const scheme_value& values){
			return cons(variables, values);
		}

		scheme_value frame_variables(const scheme_value& frame){
			return car(frame);
		}

		scheme_value frame_values(const scheme_value& frame){
			return cdr(frame);
		}

		void add_binding_to_frame(const scheme_value& var, const scheme_value& val, scheme_value& frame){
			set_car(frame, cons(var, car(frame)));
			set_cdr(frame, cons(val, cdr(frame)));
		}

		scheme_list extend_environment(const scheme_value& vars, const scheme_value& vals, scheme_list& base_env){
			if (length(vars) == length(vals))
				return cons(make_frame(vars, vals), base_env);
			else
				error("length(vars) != length (vals)", vars, vals);
			return scheme_nil;
			
		}
	}
}