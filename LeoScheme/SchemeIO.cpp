#include "scheme.h"
#include <stack>
#include <exception>
#include <stdexcept>
#include <functional>
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

			static void print_scheme_value(const scheme_value& value,std::function<void(scheme_char)>& setchar){
				if (value.can_cast<scheme_list>()){
					auto result = print(value.cast_list());
					for (auto c : result)
						setchar(c);
				}
				else{
					auto atom = value.cast_atom();
					print_scheme_atom(atom,setchar);
				}
					
			}

			static void print_scheme_list(const scheme_list & list, std::function<void(scheme_char)>& setchar){
				//×¢Òâ,scheme_nilÒ²ÊÇlist
				if (list == scheme_nil)
					setchar('\''), setchar('('), setchar(')');
				else
				{
					auto head = car(list);
					auto tail = cdr(list).cast_list();
					setchar('(');
					for (;;){
						print_scheme_value(head,setchar);
						head = car(tail);
						tail = cdr(tail).cast_list();
						if (tail != scheme_nil)
							setchar(' ');
						else
							break;
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

					if (is_list(list)){
						print_scheme_list(list,setchar);
						str.resize(pos);
						str.shrink_to_fit();
						return str;
					}

					auto stack = std::stack<const scheme_value*>();

					stack.push(&list->mCdr);
					stack.push(&list->mCar);

					const scheme_value * data = nullptr;
					const static scheme_value space(scheme_atom(' '));
					setchar('<');
					while (!stack.empty())
					{
						data = stack.top();
						if ( data == nullptr)
							setchar('>');
						else if (data->can_cast<scheme_list>())
						{
							stack.pop();
							auto list = data->cast_list();
							if (is_list(list))
								print_scheme_list(list, setchar);
							else{
								
								setchar('<');
								stack.push(nullptr);
								stack.push(&list->mCdr);
								stack.push(&space);
								stack.push(&list->mCar);
							}
						}
						else
						{
							stack.pop();
							auto atom = data->cast_atom();
							print_scheme_atom(atom,setchar);
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
	}
}