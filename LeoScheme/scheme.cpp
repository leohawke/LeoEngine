#include "scheme.h"
#include "SchemeMath.h"
#include <exception>
#include <stdexcept>
#include <functional>
#include <map>


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

		

		using primitive_proc_type = std::function < scheme_value(const scheme_list& args) > ;

		static std::map<scheme_string, primitive_proc_type> primitive_procs_table;
	}

	//运行时所需辅助函数<环境操作,错误处理>
	namespace scheme{

		

		

		//修改变量var在环境env里的约束,使得该变量现在约束到值value,如果这一变量没有约束就发出一个错误信号
		void set_variable_value(const scheme_value& var, const scheme_value& value, scheme_list& env);

		//在环境env的第一个框架里加入一个新约束,它关联起变量var和值value
		void define_variable(const scheme_value& var, const scheme_value& value, scheme_list& env);

		//返回一个新环境,这个环境中包含了一个新的框架,其中所位于表variables的符号约束到表values里队友的元素,而其外围环境是环境base_env
		scheme_list extend_environment(const scheme_value& variables, const scheme_value& values, scheme_list& base_env);

		//返回符号var在环境env里的约束值,如果这一变量没有约束发出一个错误信号
		scheme_value* lookup_variable_value(const scheme_value& var, scheme_list& env);
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

		//impl
		scheme_value eval_cond(const scheme_value& exp, scheme_list& env);

		//impl
		scheme_value apply_change(const scheme_list& procedure, const scheme_list arguments, scheme_list& env);
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
				return car(exp).can_cast<scheme_atom>() && car(exp).cast_atom().to_word() == tag;
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
			return tagged_list(exp, "lambda");
		}

		bool begin(const scheme_value& exp){
			return tagged_list(exp, "begin");
		}
		//no-impl
		bool cond(const scheme_value& exp);

		bool assignment(const scheme_value& exp){
			return tagged_list(exp, "set!");
		}

		bool definition(const scheme_value& exp){
			return tagged_list(exp, "define");
		}

		bool ifexp(const scheme_value& exp){
			return tagged_list(exp, "if");
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

		

		bool primitive_procedure(const scheme_list& procedure){
			return tagged_list(procedure, "primitive");
		}

		bool cond(const scheme_value& exp){
			return tagged_list(exp, "cond");
		}

		bool cond_else_clause(const scheme_value& clause){
			return car(clause).can_cast<scheme_atom>() && car(clause).cast_atom().cast<scheme_string>() == "else";
		}
	}

	//选择和构造函数函数声明及定义
	namespace scheme{
		scheme_value list_of_values(const scheme_value& operands, scheme_list& env);

		inline scheme_value oper(const scheme_value& exp){
			return car(exp);
		}
		inline scheme_value operands(const scheme_value& exp){
			return cdr(exp);
		}

		inline scheme_value first_operand(const scheme_value& ops){
			return car(ops);
		}

		inline scheme_value rest_operands(const scheme_value& ops){
			return cdr(ops);
		}

		inline scheme_value assignment_variable(const scheme_value& exp){
			return cadr(exp);
		}

		inline scheme_value assignment_value(const scheme_value& exp){
			return caddr(exp);
		}

		inline scheme_value lambda_parameters(const scheme_value& exp){
			return cadr(exp);
		}

		inline scheme_value lambda_body(const scheme_value& exp){
			return cddr(exp);
		}

		inline scheme_value make_lambda(const scheme_value& parameters, const scheme_value& body){
			const static scheme_value lambda_word{ scheme_atom("lambda", scheme_atom::atom_t::atom_word) };
			return cons(lambda_word, cons(parameters, body));
		}

		inline scheme_value definition_variable(const scheme_value& exp){
			if (symbol(cadr(exp)))
				return cadr(exp);
			else
				return caadr(exp);
		}

		inline scheme_value definition_value(const scheme_value& exp){
			if (symbol(cadr(exp)))
				return caddr(exp);
			else
				return make_lambda(cdadr(exp), cddr(exp));
		}

		inline scheme_value if_predicate(const scheme_value& exp){
			return cadr(exp);
		}

		inline scheme_value if_consequent(const scheme_value& exp){
			return caddr(exp);
		}

		inline scheme_value if_alternative(const scheme_value& exp){
			if (!null(cdddr(exp)))
				return cadddr(exp);
			else
				return scheme_value(scheme_atom(false));
		}

		inline scheme_value make_if(const scheme_value& predicate, const scheme_value& consequent, const scheme_value& alternative){
			const static scheme_value if_word{ scheme_atom("if", scheme_atom::atom_t::atom_word) };
			return list(if_word, predicate, consequent, alternative);
		}

		inline scheme_value make_procedure(const scheme_value& parameters, const scheme_value& body, scheme_list& env){
			const static scheme_value procedure_word{ scheme_atom("procedure", scheme_atom::atom_t::atom_word) };
			return list(procedure_word, parameters, body,env);
		}

		inline scheme_value begin_actions(const scheme_value& exp){
			return cdr(exp);
		}

		inline scheme_value first_exp(const scheme_value& seq){
			return car(seq);
		}

		inline scheme_value rest_exps(const scheme_value& seq){
			return cdr(seq);
		}

		scheme_value list_of_values(const scheme_value& exps, scheme_list& env){
			if (no_operands(exps))
				return scheme_nil;
			else{
				scheme_list p = make_scheme_list();
				auto result = p;
				scheme_value rest = exps;
				for (;;){
					p->mCar = eval(first_operand(rest), env);
					rest = rest_operands(rest);
					if (no_operands(rest)){
						p->mCdr = scheme_nil;
						return result;
					}
					else{
						p->mCdr = make_scheme_list();
						p = p->mCdr.cast_list();
					}
				}
			}
		}


		inline scheme_list procedure_environment(const scheme_value& procedure){
			return cadddr(procedure).cast_list();
		}

		inline scheme_value make_begin(const scheme_value& seq){
			const static scheme_value begin_word{ scheme_atom("begin", scheme_atom::atom_t::atom_word) };
			return cons(begin_word, seq);
		}

		inline scheme_value sequence_exp(const scheme_value& seq){
			if (null(seq))
				return seq;
			else if (last_exp(seq))
				return first_exp(seq);
			else
				return make_begin(seq);
		}

		inline scheme_value cond_clauses(const scheme_value& exp){
			return cdr(exp);
		}

		inline scheme_value cond_predicate(const scheme_value& clause){
			return car(clause);
		}

		inline scheme_value cond_actions(const scheme_value& clause){
			return cdr(clause);
		}
	}



	//eval,apply定义
	namespace scheme
	{
		scheme_value apply(const scheme_value& procedure, const scheme_value& arguments){
			return apply(procedure.cast_list(), arguments.cast_list());
		}

		scheme_value eval(const scheme_value& exp, scheme_list& env)
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
				return make_procedure(lambda_parameters(exp), lambda_body(exp),env);
			if (begin(exp))
				return eval_sequence(begin_actions(exp), env);
			if (cond(exp))
				return eval_cond(exp, env);
			if (application(exp)){
				return apply(eval(oper(exp), env), list_of_values(operands(exp), env));
			}
			else
				error("Unknown expression type --EVAL", exp);
			return scheme_nil;
		}

		primitive_proc_type primitive_impl(const scheme_list& procedure){
			DebugPrintf("底层函数被调用\n");
			return primitive_procs_table[car(procedure->mCdr).cast_atom().cast<scheme_string>()];
		}

		scheme_value apply_primitive_procedure(const scheme_list& procedure, const scheme_list& arguments){
			return primitive_impl(procedure)(arguments);
		}


		scheme_value apply(const scheme_list& procedure, const scheme_list& arguments){

			if (primitive_procedure(procedure))
				return apply_primitive_procedure(procedure, arguments);
			else if (compound_procedure(procedure))
				return eval_sequence(procedure_body(procedure), extend_environment(procedure_parameters(procedure), arguments, procedure_environment(procedure)));
			error("Unknown procedure type -- APPLY ", procedure);
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



		scheme_value eval_assignment(const scheme_value& exp, scheme_list& env){
			set_variable_value(
				assignment_variable(exp),
				eval(assignment_value(exp), env),
				env);
			return scheme_nil;
		}

		scheme_value eval_definition(const scheme_value& exp, scheme_list& env){
			auto result = eval(definition_value(exp), env);
			define_variable(
				definition_variable(exp),
				result,
				env);
			return result;
		}

		scheme_value eval_if(const scheme_value& exp, scheme_list& env){
			if (true_exp(eval(if_predicate(exp), env)))
				return eval(if_consequent(exp), env);
			else
				return eval(if_alternative(exp), env);
		}

		scheme_value eval_cond(const scheme_value& exp, scheme_list& env){
			//首先取出clauses部分
			auto& clauses = cond_clauses(exp);
			if (null(clauses))
				error("cond子句为空");
			else{
				auto first = car(clauses);
				auto rest = cdr(clauses);
				for (;;){
					
					if (cond_else_clause(first)){
						if (null(rest))
							return eval(sequence_exp(cond_actions(first)), env);
						else
							error("else 不是最后一个cond子句", clauses);
					}
					else{
						auto pre = cond_predicate(first);
						auto actions = cond_actions(first);
						if (true_exp(eval(pre, env)))
							return eval(sequence_exp(actions), env);
						else if (!null(rest)){
							first = car(rest);
							rest = cdr(rest);
						}
						else
							return scheme_atom(false);
					}

				}
			}
			return scheme_atom(false);
		}
	}

	//运行时所需辅助函数<环境操作,错误处理>定义
	namespace scheme{
		scheme_list& enclosing_environment(scheme_list& env){
			return env->mCdr.cast_list();
		}

		scheme_value& first_frame(scheme_list& env){
			return env->mCar;
		}

		scheme_list the_empty_envirnoment(){
			return scheme_nil;
		}

		scheme_value make_frame(const scheme_value& variables, const scheme_value& values){
			return cons(variables, values);
		}

		const scheme_value& frame_variables(const scheme_value& frame){
			return car(frame);
		}

		scheme_value& frame_variables(scheme_value& frame){
			return car(frame);
		}

		const scheme_value& frame_values(const scheme_value& frame){
			return cdr(frame);
		}

		scheme_value& frame_values(scheme_value& frame){
			return cdr(frame);
		}

		void add_binding_to_frame(const scheme_value& var, const scheme_value& val, scheme_value& frame){
			//scheme_value(cons(val, cdr(frame)));
			set_car(frame, cons(var, car(frame)));
			set_cdr(frame, cons(val, cdr(frame)));
			var.can_cast<scheme_atom>();
		}

		scheme_list extend_environment(const scheme_value& vars, const scheme_value& vals, scheme_list& base_env){
			if (length(vars) == length(vals))
				return cons(make_frame(vars, vals), base_env);
			else
				error("错误: 参数数量不匹配", vars, vals);
			return scheme_nil;

		}

		scheme_value&  lookup_env_loop(const scheme_value& var, scheme_list& env, const scheme_value& vars, scheme_value& vals);
		scheme_value&  lookup_scan(const scheme_value& var, scheme_list& env, const scheme_value& vars, scheme_value& vals);

		scheme_value* lookup_variable_value(const scheme_value& var, scheme_list& env)
		{
#ifdef DEBUG
			auto var_name = ops::print(var);
			DebugPrintf("查找变量 %s 的值: \n",var_name.c_str());
#endif
			static scheme_value place_hold;
			return &lookup_env_loop(var, env,var,place_hold);
		}

		scheme_value& lookup_scan(const scheme_value& var, scheme_list& env, const scheme_value& vars,scheme_value& vals){
			if (null(vars))
				return lookup_env_loop(var, enclosing_environment(env), vars, vals);
			else if (var == car(vars))
				return car(vals);
			else
				return lookup_scan(var, env, cdr(vars), cdr(vals));
		}

		scheme_value&  lookup_env_loop(const scheme_value& var, scheme_list& env, const scheme_value& vars, scheme_value& vals){
			static scheme_value value_nil = scheme_nil;
			if (env == the_empty_envirnoment())
				error("Unbiund variable ", var);
			else{
				auto& frame = first_frame(env); 
				return lookup_scan(var, env, frame_variables(frame), frame_values(frame));
			}
			error("未知错误,函数不应该执行到这里");
			return value_nil;
		}

		void set_env_loop(const scheme_value& var, const scheme_value& val, scheme_list& env, const scheme_value& vars, scheme_value& vals);
		void set_scan(const scheme_value& var, const scheme_value& val, scheme_list& env, const scheme_value& vars, scheme_value& vals);

		void set_variable_value(const scheme_value& var, const scheme_value& val, scheme_list& env){
#ifdef DEBUG
			auto var_name = ops::print(var);
			DebugPrintf("设置变量 %s 的值\n", var_name.c_str());
#endif
			static scheme_value place_hold;
			return set_env_loop(var, val, env, var, place_hold);
		}

		void set_env_loop(const scheme_value& var, const scheme_value& val, scheme_list& env, const scheme_value& vars, scheme_value& vals){
			if (env == the_empty_envirnoment())
				error("Unbiund variable", var);
			else{
				auto& frame = first_frame(env);
				set_scan(var, val, env, frame_variables(frame), frame_values(frame));
			}
		}

		void set_scan(const scheme_value& var, const scheme_value& val, scheme_list& env, const scheme_value& vars, scheme_value& vals){
			if (null(vars))
				set_env_loop(var, val, enclosing_environment(env), vars, vals);
			else if (var == car(vars))
				set_car(vals, val);
			else
				set_scan(var, val, env, cdr(vars), cdr(vals));
		}


		void define_scan(const scheme_value& var, const scheme_value& val, scheme_list& env, scheme_value& frame, const scheme_value& vars, scheme_value& vals){
			if (null(vars))
				add_binding_to_frame(var, val, frame);
			else if (var == car(vars))
				set_car(vals, val);
			else
				define_scan(var, val, env, frame, cdr(vars), cdr(vals));
		}

		void define_variable(const scheme_value& var, const scheme_value& val, scheme_list& env){
#ifdef DEBUG
			auto var_name = ops::print(var);
			DebugPrintf("定义新变量 %s \n", var_name.c_str());
#endif
			auto & frame = first_frame(env);
			auto& vars = frame_variables(frame);
			auto& vals = frame_values(frame);
			define_scan(var, val, env, frame, frame_variables(frame), frame_values(frame));
		}

		std::pair<scheme_value, scheme_value> make_name_object(const scheme_string& funcname){
			std::pair<scheme_value, scheme_value> result;
			result.first = scheme_atom(funcname, scheme_atom::atom_t::atom_word);
			result.second = list(scheme_atom(scheme_string("primitive"), scheme_atom::atom_t::atom_word), scheme_atom(funcname, scheme_atom::atom_t::atom_string));
			return result;
		}

		bool change_procedure(const scheme_list& procedure){
			if (!primitive_procedure(procedure))
				return false;
			auto procedure_name = car(procedure->mCdr).cast_atom().cast<scheme_string>();
			if (procedure_name == "set-car!")
				return true;
			if (procedure_name == "set-cdr!")
				return true;
			return false;
		}

		scheme_value add(const scheme_list& args){
			auto l = length(args);
			if (l <= 1)
				error("+ 参数不够");

			scheme_real r_result;

			bool i = true;
			bool s = false;
			scheme_string s_result;

			auto first = args->mCar.cast_atom();
			auto next = args->mCdr;

			auto next_first = [&](){
				first = next.cast_list()->mCar.cast_atom();
				next = next.cast_list()->mCdr;
			};

			if (first.can_cast<scheme_string>()){
				s = true;
				s_result = first.cast<scheme_string>();
			}
			else{
				if (first.can_cast<scheme_real>()){
					r_result = first.cast<scheme_real>();
					i = false;
				}
				else
					r_result = static_cast<scheme_real>(first.cast<scheme_int>());
			}

			using atom_t = scheme_atom::atom_t;
			for (auto j = 1u; j != l; ++j){
				next_first();
				if (s)
					if (first.can_cast<scheme_string>())
						s_result += first.cast<scheme_string>();
					else
						error("+ string the next parameter must be string", first);
				else if (number(first)){
					if (first.can_cast<scheme_real>()){
						r_result += first.cast<scheme_real>();
						i = false;
					}
					else
						r_result += first.cast<scheme_int>();
				}
				else
					error("+ real/int the next parameter must be real/int", first);
			}

			if (s)
				return scheme_atom(s_result, atom_t::atom_string);
			if (i)
				return scheme_atom(static_cast<scheme_int>(r_result));
			else
				return scheme_atom(r_result);
		}

		scheme_value sub(const scheme_list& args){
			auto l = length(args);
			if (l <= 1)
				error("- 参数不够");

			scheme_real r_result;

			bool i = true;

			auto first = args->mCar.cast_atom();
			auto next = args->mCdr;

			auto next_first = [&](){
				first = next.cast_list()->mCar.cast_atom();
				next = next.cast_list()->mCdr;
			};

			if (first.can_cast<scheme_real>()){
					r_result = first.cast<scheme_real>();
					i = false;
			}
			else if (first.can_cast<scheme_int>())
				r_result = static_cast<scheme_real>(first.cast<scheme_int>());
			else
				error("- 参数类型错误", first);

			using atom_t = scheme_atom::atom_t;
			for (auto j = 1u; j != l; ++j){
				next_first();
				if (number(first)){
					if (first.can_cast<scheme_real>()){
						r_result -= first.cast<scheme_real>();
						i = false;
					}
					else
						r_result -= first.cast<scheme_int>();
				}
				else
					error("- real/int the next parameter must be real/int", first);
			}

			if (i)
				return scheme_atom(static_cast<scheme_int>(r_result));
			else
				return scheme_atom(r_result);
		}

		scheme_value mul(const scheme_list& args){
			auto l = length(args);
			if (l <= 1)
				error("* 参数不够");

			scheme_real r_result;

			bool i = true;

			auto first = args->mCar.cast_atom();
			auto next = args->mCdr;

			auto next_first = [&](){
				first = next.cast_list()->mCar.cast_atom();
				next = next.cast_list()->mCdr;
			};

			if (first.can_cast<scheme_real>()){
				r_result = first.cast<scheme_real>();
				i = false;
			}
			else if (first.can_cast<scheme_int>())
				r_result = static_cast<scheme_real>(first.cast<scheme_int>());
			else
				error("* 参数类型错误", first);

			using atom_t = scheme_atom::atom_t;
			for (auto j = 1u; j != l; ++j){
				next_first();
				if (number(first)){
					if (first.can_cast<scheme_real>()){
						r_result *= first.cast<scheme_real>();
						i = false;
					}
					else
						r_result *= first.cast<scheme_int>();
				}
				else
					error("* real/int the next parameter must be real/int", first);
			}

			if (i)
				return scheme_atom(static_cast<scheme_int>(r_result));
			else
				return scheme_atom(r_result);
		}

		scheme_value div(const scheme_list& args){
			auto l = length(args);
			if (l <= 1)
				error("/ 参数不够");

			scheme_real r_result;

			bool i = true;

			auto first = args->mCar.cast_atom();
			auto next = args->mCdr;

			auto next_first = [&](){
				first = next.cast_list()->mCar.cast_atom();
				next = next.cast_list()->mCdr;
			};

			if (first.can_cast<scheme_real>()){
				r_result = first.cast<scheme_real>();
				i = false;
			}
			else if (first.can_cast<scheme_int>())
				r_result = static_cast<scheme_real>(first.cast<scheme_int>());
			else
				error("/ 参数类型错误", first);

			using atom_t = scheme_atom::atom_t;
			for (auto j = 1u; j != l; ++j){
				next_first();
				if (number(first)){
					if (first.can_cast<scheme_real>()){
						r_result /= first.cast<scheme_real>();
						i = false;
					}
					else
						r_result /= first.cast<scheme_int>();
				}
				else
					error("* real/int the next parameter must be real/int", first);
			}

			if (i)
				return scheme_atom(static_cast<scheme_int>(r_result));
			else
				return scheme_atom(r_result);
		}

		
		inline bool operator<(const scheme_atom& lhs, const scheme_atom& rhs){
				return (lhs.can_cast<scheme_real>() ? lhs.cast<scheme_real>() : lhs.cast<scheme_int>()) <
					(rhs.can_cast<scheme_real>() ? rhs.cast<scheme_real>() : rhs.cast<scheme_int>());
		}

		inline bool operator>=(const scheme_atom& lhs, const scheme_atom& rhs){
			return !(lhs < rhs);
		}

		inline bool operator>(const scheme_atom& lhs, const scheme_atom& rhs){
				return (lhs.can_cast<scheme_real>() ? lhs.cast<scheme_real>() : lhs.cast<scheme_int>()) >
					(rhs.can_cast<scheme_real>() ? rhs.cast<scheme_real>() : rhs.cast<scheme_int>());
		}

		inline bool operator<=(const scheme_atom& lhs, const scheme_atom& rhs){
			return !(lhs > rhs);
		}
		
		scheme_value equal(const scheme_list& args){
			auto l = length(args);
			if (l != 2)
				error("= 参数数量不对");

			auto first = args->mCar.cast_atom();
			auto second = args->mCdr.cast_list()->mCar.cast_atom();

			return first == second;
		}
		scheme_value less(const scheme_list& args){
			auto l = length(args);
			if (l != 2)
				error("/ 参数数量不对");

			auto first = args->mCar.cast_atom();
			auto second = args->mCdr.cast_list()->mCar.cast_atom();

			return first < second;
		}
		scheme_value less_equal(const scheme_list& args){
			auto l = length(args);
			if (l != 2)
				error("/ 参数数量不对");

			auto first = args->mCar.cast_atom();
			auto second = args->mCdr.cast_list()->mCar.cast_atom();

			return first <= second;
		}
		scheme_value greater(const scheme_list& args){
			auto l = length(args);
			if (l != 2)
				error("/ 参数数量不对");

			auto first = args->mCar.cast_atom();
			auto second = args->mCdr.cast_list()->mCar.cast_atom();

			return first > second;
		}
		scheme_value greater_equal(const scheme_list& args){
			auto l = length(args);
			if (l != 2)
				error("/ 参数数量不对");

			auto first = args->mCar.cast_atom();
			auto second = args->mCdr.cast_list()->mCar.cast_atom();

			return first >= second;
		}



		scheme_value set_car_impl(const scheme_list& args){
			if (length(args) < 2)
				error("set-car! 没有提供值");

			scheme_value& first = args->mCar;
			if (!pair(first))
				error("set-car! 第一个参数应该是序对");

			
			if (length(args) > 2)
				error("set-car! 提供值过多");
			auto second = args->mCdr.cast_list()->mCar;
			auto first_string = ops::print(first);
			auto b = first.can_cast<scheme_list>();
			set_car(first, second);

			return first;
		}
		scheme_value set_cdr_impl(const scheme_list& args){
			if (length(args) < 2)
				error("set-cdr! 没有提供值");

			scheme_value& first = args->mCar;
			if (!pair(first))
				error("set-cdr! 第一个参数应该是序对");


			if (length(args) > 2)
				error("set-cdr! 提供值过多");
			auto second = args->mCdr.cast_list()->mCar;

			set_cdr(first, second);

			return first;
		}
		scheme_value cons_impl(const scheme_list& args){
			if (length(args) != 2)
				error("cons参数应为两个", args);
			auto & first = args->mCar.cast_atom();
			auto & second = args->mCdr.cast_list()->mCar.cast_atom();
			return cons(first, second);
		}

		scheme_value list_impl(const scheme_list& args){
			return args;
		}

		scheme_list setup_environment(){
			DebugPrintf("初始化环境...\n");
			auto env = scheme_nil;
			auto addpair = make_name_object("+");
			auto subpair = make_name_object("-");
			auto mulpair = make_name_object("*");
			auto divpair = make_name_object("/");
			auto equalpair = make_name_object("=");
			auto lesspair = make_name_object("<");
			auto greaterpair = make_name_object(">");
			auto less_equalpair = make_name_object("<=");
			auto greater_equalpair = make_name_object(">=");
			auto names = list(addpair.first,subpair.first,mulpair.first,divpair.first,
				equalpair.first,lesspair.first,greaterpair.first,less_equalpair.first,greater_equalpair.first);
			auto objects = list(addpair.second,subpair.second,mulpair.second,divpair.second,
				equalpair.second, lesspair.second, greaterpair.second, less_equalpair.second, greater_equalpair.second);
			primitive_procs_table["+"] = add;
			primitive_procs_table["-"] = sub;
			primitive_procs_table["*"] = mul;
			primitive_procs_table["/"] = div;
			primitive_procs_table["="] = equal;
			primitive_procs_table["<"] = less;
			primitive_procs_table[">"] = greater;
			primitive_procs_table["<="] = less_equal;
			primitive_procs_table[">="] = greater_equal;
			DebugPrintf("加入+,-,*,/的实现...\n");
			DebugPrintf("加入=,<,>,<=,>=的实现...\n");
			//注意,set-car!和set-cdr!是硬编码的
			auto set_car_pair = make_name_object("set-car!");
			auto set_cdr_pair = make_name_object("set-cdr!");
			names = cons(set_car_pair.first, names);
			objects = cons(set_car_pair.second, objects);
			names = cons(set_cdr_pair.first, names);
			objects = cons(set_cdr_pair.second, objects);

			auto cons_pair = make_name_object("cons");
			auto list_pair = make_name_object("list");
			names = cons(cons_pair.first, names);
			objects = cons(cons_pair.second, objects);
			names = cons(list_pair.first, names);
			objects = cons(list_pair.second, objects);
			primitive_procs_table["cons"] = cons_impl;
			primitive_procs_table["list"] = list_impl;
			primitive_procs_table["set-car!"] = set_car_impl;
			primitive_procs_table["set-cdr!"] = set_cdr_impl;
			DebugPrintf("加入 set-car!,set-cdr!,cons,list..\n");
			//数据操作函数定义

			auto and_pair = make_name_object("and");
			auto or_pair = make_name_object("or");
			auto not_pair = make_name_object("not");
			auto abs_pair = make_name_object("abs");
			names = cons(and_pair.first, names);
			objects = cons(and_pair.second, objects);
			names = cons(or_pair.first, names);
			objects = cons(or_pair.second, objects);
			names = cons(not_pair.first, names);
			objects = cons(not_pair.second, objects);
			names = cons(abs_pair.first, names);
			objects = cons(abs_pair.second, objects);
			primitive_procs_table["and"] = scheme_and;
			primitive_procs_table["or"] = scheme_or;
			primitive_procs_table["not"] = scheme_not;
			primitive_procs_table["abs"] = scheme_abs;
			return extend_environment(names, objects, env);
		}
	}
}

