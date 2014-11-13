#include "scheme.h"
#include <stack>
#include <exception>
#include <stdexcept>
#include <functional>

#include <debug.hpp>

namespace leo
{
	namespace scheme
	{
		namespace ops{
			static void print_scheme_atom(const scheme_atom& atom, std::function<void(scheme_char)>& setchar){
				auto print_bool = [&](scheme_bool b)
				{
					setchar('#');
					if (b)
						setchar('T');
					else
						setchar('F');
				};
				auto print_char = [&](scheme_char c)
				{
					setchar('~');
					setchar(c);
				};
				auto print_string = [&](const scheme_string& str)
				{
					auto string = sexp::restore_escape_string(str);
					setchar('"');
					for (auto c : string)
						setchar(c);
					setchar('"');
				};
				auto print_word = [&](const scheme_string& string)
				{
					for (auto c : string)
						setchar(c);
				};
				auto print_int = [&](scheme_int i)
				{
					auto string = to_string(i);
					for (auto c : string)
						setchar(c);
				};
				auto print_real = [&](scheme_real r)
				{
					auto string = to_string(r);
					for (auto c : string)
						setchar(c);
				};

				switch (atom.mType)
				{
				case scheme_atom::atom_t::atom_bool:
					print_bool(atom.cast<scheme_bool>());
					break;
				case scheme_atom::atom_t::atom_char:
					print_char(atom.cast<scheme_char>());
					break;
				case scheme_atom::atom_t::atom_int:
					print_int(atom.cast<scheme_int>());
					break;
				case scheme_atom::atom_t::atom_real:
					print_real(atom.cast<scheme_real>());
					break;
				case scheme_atom::atom_t::atom_string:
					print_string(atom.cast<scheme_string>());
					break;
				case scheme_atom::atom_t::atom_word:
					print_word(atom.cast<scheme_string>());
					break;
				default:
					break;
				}
			}

			static void print_scheme_value(const scheme_value& value, std::function<void(scheme_char)>& setchar){
				if (value.can_cast<scheme_list>()){
					auto result = print(value.cast_list());
					for (auto c : result)
						setchar(c);
				}
				else{
					auto atom = value.cast_atom();
					print_scheme_atom(atom, setchar);
				}

			}

			static void print_scheme_list(const scheme_list & list, std::function<void(scheme_char)>& setchar){
				//注意,scheme_nil也是list
				if (list == scheme_nil)
					setchar('\''), setchar('('), setchar(')');
				else
				{
					auto head = car(list);
					auto tail = cdr(list).cast_list();
					setchar('(');
					for (;;){
						print_scheme_value(head, setchar);
						if (tail != scheme_nil)
							setchar(' ');
						else
							break;
						head = car(tail);
						tail = cdr(tail).cast_list();
					}
					setchar(')');
				}
			}

			scheme_string print(const scheme_list & list){
					{
						scheme_string str(64, scheme_char());
						std::size_t pos = 0;
						std::function<void(scheme_char)> setchar = [&](scheme_char c)
						{
							if (str.length() == pos)
								str.resize(pos + 32);
							str[pos++] = c;
						};

						if (list != scheme_nil && compound_procedure(list))
						{
							char pre_str[] = "(compouned-procedure (lambda ";
							for (auto c : pre_str)
								setchar(c);
							--pos;
							auto para_str = print(procedure_parameters(list));
							for (auto c : para_str)
								setchar(c);
							auto body_str = print(procedure_body(list));
							body_str[0] = ' ';
							for (auto c : body_str)
								setchar(c);
							setchar(')');
							str.resize(pos);
							str.shrink_to_fit();
							return str;
						}

						if (is_list(list)){
							print_scheme_list(list, setchar);
							str.resize(pos);
							str.shrink_to_fit();
							return str;
						}

						auto stack = std::stack<const scheme_value*>();

						const scheme_value  * space = reinterpret_cast<scheme_value  * >(0xffffffff);

						stack.push(&list->mCdr);
						stack.push(space);
						stack.push(&list->mCar);

						const scheme_value * data = nullptr;

						setchar('<');
						while (!stack.empty())
						{
							data = stack.top();
							stack.pop();
							if (data == nullptr)
								setchar('>');
							else if (data == space)
								setchar(' ');
							else if (data->can_cast<scheme_list>())
							{
								
								auto list = data->cast_list();
								if (is_list(list))
									print_scheme_list(list, setchar);
								else{

									setchar('<');
									stack.push(nullptr);
									stack.push(&list->mCdr);
									stack.push(space);
									stack.push(&list->mCar);
								}
							}
							else
							{
								auto atom = data->cast_atom();
								print_scheme_atom(atom, setchar);
							}
						}

						setchar('>');
						str.resize(pos);
						str.shrink_to_fit();
						return str;
					}
			}

			scheme_string print(const scheme_value & value){
				if (value.can_cast<scheme_list>())
					return print(value.cast_list());
				else{
					scheme_string str(64, scheme_char());
					std::size_t pos = 0;
					std::function<void(scheme_char)> setchar = [&](scheme_char c)
					{
						if (str.length() == pos)
							str.resize(pos + 32);
						str[pos++] = c;
					};
					print_scheme_atom(value.cast_atom(), setchar);

					str.resize(pos);
					str.shrink_to_fit();
					return str;
				}
			}
		}

		scheme_string scheme_atom::to_word() const{
			return ops::print(*this);
		}


		static scheme_value parse_word(scheme_string& word)
		{
			if ((word[0] >= '0' && word[0] <= '9') || ((word[0] == '+' || word[0] == '-') && word.size() > 1 && (word[1] >= '0' && word[1] <= '9')))
			{
				if (word.size() > 1 && (word[word.size() - 1] == 'x' || word[word.size() - 1] == 'X'))
				{
					scheme_int value;
					sscanf(word.c_str(), "%llx", &value);
					return scheme_atom(value);
				}
				else
				{
					if (word.find_first_of('.') != scheme_string::npos)
					{
						scheme_real value;
						sscanf(word.c_str(), "%lf", &value);
						return scheme_atom(value);
					}
					else
					{
						scheme_int value;
						sscanf(word.c_str(), "%lld", &value);
						return scheme_atom(value);
					}
				}
			}

			if (word[0] == '~')
			{
				return scheme_atom(word[1]);
			}

			if (word[0] == '#')
			{
				if (word[1] == 't' || word[1] == 'T')
					return scheme_atom(true);
				else
					return scheme_atom(false);
			}

			if (word[0] != '\"')
			{
				return scheme_atom(word, scheme_atom::atom_t::atom_word);
			}
			else
			{
				word.erase(word.cbegin());
				word.erase(--word.cend());
				return scheme_atom(word, scheme_atom::atom_t::atom_string);
			}
		}

		scheme_value read(){
			printf("\n;;;<=");
			char c = ' ';

			//跳过开始的所有空格
			while (c == ' ' || c == '\n' || c == '\r' || c == '\t')
				scanf("%c", &c);

			std::string input;
			input += c;
			//如果不是括号开始,说明是个原子,持续读入到空格或换行或制表,注意,不支持字符串!
			if (c != '('){
				while (c != ' ' && c != '\n' && c != '\r' && c != '\t'){
					scanf("%c", &c);
					input += c;
				}
				input.resize(input.size() - 1);
				DebugPrintf("读入字符串: %s\n", input.c_str());
				return parse_word(input);
			}
			else{
				auto depth = 1u;
				//直到读到depth 为0,读入一个( ,depth++,读入一个),depth--
				while (depth)
				{
					scanf("%c", &c);
					input += c;
					if (c == '(')
						++depth;
					else{
						if (c == ')')
							--depth;
					}
				}
				DebugPrintf("读入字符串: %s\n", input.c_str());
				auto tokens = sexp::lexicalanalysis(input);
				auto sexp = sexp::parse(tokens);
				auto scheme = make_copy(sexp);
				return scheme;
			}
		}
		void write(const scheme_value& exp){
			auto exp_string = ops::print(exp);
			printf("\n;;;=>%s\n", exp_string.c_str());
		}
	}
}