#include "Sexp.hpp"

#include <string>
#include <stack>
#include <strstream>
#include <sstream>


namespace leo
{
	namespace scheme
	{
		namespace sexp
		{
			sexp_string restore_escape_string(const sexp_string& str)
			{
				sexp_string result;
				for (std::size_t i = 0; i != str.size(); ++i)
				{
					if (str[i] == '\\')
					{
						switch (str[i+1])
						{
						case 't':
							result.push_back('\t');
							break;
						case 'v':
							result.push_back('\v');
							break;
						case 'r' :
							result.push_back('\r');
							break;
						case 'n':
							result.push_back('\n');
							break;
						case '\\':
							result.push_back('\\');
							break;
						case '"':
							result.push_back('"');
						}
						++i;
					}
					else
					{
						result.push_back(str[i]);
					}
				}
				return str;
			}
			sexp_string store_escape_string(const sexp_string& str)
			{
				sexp_string result;
				for (std::size_t i = 0; i != str.size(); ++i)
				{
					switch (str[i])
					{
					case '"':
						result.push_back('\\');
						break;
					case '\\':
						result.push_back('\\');
					case '\t':
						result.push_back('\\');
						break;
					case '\v':
						result.push_back('\\');
						break;
					case '\r':
						result.push_back('\\');
						break;
					case '\n':
						result.push_back('\\');
						break;
					default:
						break;
					}
					result.push_back(str[i]);
				}
				return str;
			}

			//需要增加字符串支持
			token::list lexicalanalysis(const sexp_char * str, std::size_t len)
			{
				enum { state_begin, state_word,state_string} state = state_begin;
				std::size_t pos = 0;
				sexp_string value(64, sexp_char());
				token::list tokenlist;
				auto s = str;

				auto pushword = [&]()
				{
					value.resize(pos);
					value.shrink_to_fit();
					tokenlist.emplace(store_escape_string(value));

					value = sexp_string(64, sexp_char());

					pos = 0;
					state = state_begin;
				};

				auto pushstring = [&]()
				{
					value.resize(pos);
					value.shrink_to_fit();
					tokenlist.emplace(std::move(value));
					tokenlist.back().ty = token::string;
					value = sexp_string(64, sexp_char());

					pos = 0;
					state = state_begin;
				};
				auto setchar = [&]()
				{
					if (value.length() == pos)
						value.resize(pos + 32);
					value[pos++] = *s;
				};

				while (*s)
				{
					switch (state)
					{
					case state_begin:
						switch (*s)
						{
						case '(':
							tokenlist.emplace(token::left_split);
							break;
						case ')':
							tokenlist.emplace(token::right_split);
							break;
						case'\t':
						case'\n':
						case'\r':
						case' ':
							break;
						case '"':
							state = state_string;
							break;
						default:
							--s;
							state = state_word;
						}
						break;
					case state_word:
						switch (*s)
						{
						case'\t':
						case'\n':
						case'\r':
						case' ':
							pushword();
							break;
						case '(':
						case ')':
							pushword();
							--s;
							break;
						default:
							setchar();
							break;
						}
						break;
					case state_string:
						if (*s == '"' && ( s[-1] != '\\' ||( s[-1] == '\\' && s[-2] == '\\') )){
							pushstring();
							break;
						}
						setchar();
						break;
					default:
						break;
					}
					++s;
				}

				return tokenlist;
			}
			namespace ops
			{
				sexp_list make_copy(const const_sexp_list & s)
				{
#if 0
					if (!s)
						return nullptr;
#endif
					std::shared_ptr<sexp> copysexp = leo::make_shared<sexp>(*s);
					if (s->mNext)
						copysexp->mNext = make_copy(s->mNext);
					return copysexp;
				}

				sexp_list find_sexp(const sexp_char * word, const sexp_list & s)
				{
					if (!s)
						return nullptr;
					if (s->mValue.can_cast<sexp_list>())
						if (auto sexp = find_sexp(word, s->mValue.cast_list()))
							return sexp;
						else
							return find_sexp(word, s->mNext);
					else
						if (s->mValue.can_cast<sexp_string>() && s->mValue.cast_atom<sexp_string>() == word)
							return s;
						else
							return find_sexp(word, s->mNext);
					return nullptr;
				}

				sexp_list bfs_find_sexp(const sexp_char * word, const sexp_list & s)
				{
					if (!s)
						return nullptr;

					auto t = s;
					while (t)
					{
						if (s->mValue.can_cast<sexp_string>() && s->mValue.cast_atom<sexp_string>() == word){
								return t;
						}
						t = t->mNext;
					}

					t = s;
					std::shared_ptr<sexp> rt;
					while (t)
					{
						if (s->mValue.can_cast<sexp_list>()){
							if (rt = bfs_find_sexp(word,s->mValue.cast_list()))
								return rt;
						}
						t = t->mNext;
					}
					return nullptr;
				}

				std::size_t sexp_list_length(const const_sexp_list & s)
				{
					if (!s)
						return 0;
					auto next = s->mNext;
					auto result = 1;
					while (next)
					{
						++result;
						next = next->mNext;
					}
					return result;
				}

				sexp_string print_sexp(const const_sexp_list & s)
				{
					sexp_string str(64, sexp_char());
					std::size_t pos = 0;
					auto setchar = [&](sexp_char c)
					{
						if (str.length() == pos)
							str.resize(pos + 32);
						str[pos++] = c;
					};

					auto print_bool = [&](leo::any& any)
					{
						auto b = leo::any_cast<bool>(any);
						setchar('#');
						if (b)
							setchar('T');
						else
							setchar('F');
					};
					auto print_char = [&](leo::any& any)
					{
						auto c = leo::any_cast<sexp_char>(any);
						setchar('~');
						setchar(c);
					};
					auto print_string = [&](leo::any& any)
					{
						auto string =restore_escape_string(any_cast<sexp_string>(any));
						setchar('"');
						for (auto c : string)
							setchar(c);
						setchar('"');
					};
					auto print_word = [&](leo::any& any)
					{
						auto string = any_cast<sexp_string>(any);
						for (auto c : string)
							setchar(c);
					};
					auto print_int = [&](leo::any& any)
					{
						auto string = to_string(any_cast<std::int64_t>(any));
						for (auto c : string)
							setchar(c);
					};
					auto print_real = [&](leo::any& any)
					{
						auto string = to_string(any_cast<std::double_t>(any));
						for (auto c : string)
							setchar(c);
					};
					
#define RETURNSTRING {str.resize(pos);str.shrink_to_fit();return str;}

					if (!s)
						RETURNSTRING;

					auto fakehead = std::make_shared<sexp>(*s);

					fakehead->mValue = s->mValue;
					fakehead->mNext = s->mNext;
					auto stack = std::stack<sexp*>();

					stack.push(fakehead.get());

					int depth = 1;

					setchar('(');
					sexp * data = nullptr;
					while (!stack.empty())
					{
						data = stack.top();
						if (!data)
						{
							stack.pop();
							if (depth > 0)
							{
								setchar(')');
								--depth;
							}

							if (stack.empty())
								break;

							data = stack.top() = stack.top()->mNext.get();
							if (data)
								setchar('\n');
							if (data)
								for (auto i = depth; i != 0;--i)
									setchar('\t');
						}
						else if (data->mValue.can_cast<sexp_list>())
						{
							++depth;
							setchar('(');
							stack.push(data->mValue.cast_list().get());
						}
						else
						{
							switch (data->mValue.mAtomType)
							{
							case sexp_value::atom_bool:
								print_bool(data->mValue.mAtomValue);
								break;
							case sexp_value::atom_char:
								print_char(data->mValue.mAtomValue);
								break;
							case sexp_value::atom_int:
								print_int(data->mValue.mAtomValue);
								break;
							case sexp_value::atom_real:
								print_real(data->mValue.mAtomValue);
								break;
							case sexp_value::atom_string:
								print_string(data->mValue.mAtomValue);
								break;
							case sexp_value::atom_word:
								print_word(data->mValue.mAtomValue);
								break;
							default:
								break;
							}

							data = stack.top() = data->mNext.get();

							if (data)
								setchar(' ');
						}
					}

					while (depth > 0)
					{
						setchar(')');
					}

					RETURNSTRING;
				}
			}

			static void parse_word(const sexp_string& word, const sexp_list& dst)
			{
				if ((word[0] >= '0' && word[0] <= '9') || ((word[0] == '+' || word[0] == '-') && word.size() > 1 && (word[1] >= '0' && word[1] <= '9')))
				{
					if (word.size() > 1 && (word[word.size() - 1] == 'x' || word[word.size() - 1] == 'X'))
					{
						sexp_int value;
						sscanf(word.c_str(), "%llx", &value);
						dst->mValue = sexp_value(value);
					}
					else
					{
						if (word.find_first_of('.') != sexp_string::npos)
						{
							sexp_real value;
							sscanf(word.c_str(), "%lf", &value);
							dst->mValue =sexp_value(value);
						}
						else
						{
							sexp_int value;
							sscanf(word.c_str(), "%lld", &value);
							dst->mValue = value;
							dst->mValue = sexp_value(value);
						}
					}
				}
				else if (word[0] == '~')
				{
					dst->mValue =sexp_value(word[1]);
				}
				else if (word[0] == '#')
				{
					if (word[1] == 't' || word[1] == 'T')
						dst->mValue =sexp_value(true);
					else
						dst->mValue =sexp_value(false);
				}
				else
				{
					dst->mValue =sexp_value(word,sexp_value::atom_word);
				}
			}

			sexp_list parse(token::list& tokens)
			{
				enum { state_begin, state_atom, state_blist, state_elist } state = state_begin;
				unsigned int depth = 0;
				std::shared_ptr<sexp> sx = nullptr;

				struct parse_data_t
				{
					std::shared_ptr<sexp> fst;
					std::shared_ptr<sexp> lst;
					parse_data_t(std::shared_ptr<sexp> fst, std::shared_ptr<sexp> lst)
						:fst(fst), lst(lst)
					{}
				};

				std::stack<parse_data_t> stack;
				//sexp_string value;
				parse_data_t lvl(nullptr, nullptr);
				while (!tokens.empty())
				{
					switch (state)
					{
					case state_begin:
						switch (tokens.front().ty)
						{
						case token::left_split:
							state = state_blist;
							break;
						case token::right_split:
							state = state_elist;
							break;
						default:
							state = state_atom;
							break;
						}
						break;
					case state_atom:
						sx = make_sexp(nullptr);
						parse_word(tokens.front().mWordValue,sx);
						if (tokens.front().ty == token::string)
							sx->mValue.mAtomType = sexp_value::atom_string;
						if (!stack.empty())
						{
							lvl = stack.top();
							if (!lvl.fst)
								stack.top().fst = stack.top().lst = sx;
							else
							{
								stack.top().lst->mNext = sx;
								stack.top().lst = sx;
							}
						}
						else
							throw;
						tokens.pop();
						state = state_begin;
						break;
					case state_blist:
						depth++;
						sx = make_sexp(nullptr);
						if (stack.empty())
						{
							stack.emplace(sx, sx);
						}
						else
						{
							parse_data_t data = stack.top();
							if (data.lst)
								stack.top().lst->mNext = sx;
							else
								stack.top().fst = sx;
							stack.top().lst = sx;
						}
						stack.emplace(nullptr, nullptr);
						tokens.pop();
						state = state_begin;
						break;
					case state_elist:
						if (depth == 0)
							throw;
						--depth;
						lvl = stack.top();
						stack.pop();
						sx = lvl.fst;
						lvl.fst = lvl.lst = nullptr;
						if (!stack.empty())
							stack.top().lst->mValue = sx;
						else
							throw;

						state = state_begin;
						if (depth == 0)
						{
							while (!stack.empty())
							{
								lvl = stack.top();
								stack.pop();
								sx = lvl.fst;
								lvl.fst = lvl.lst = nullptr;
							}
							return sx->mValue.cast_list();
						}
						tokens.pop();
						break;
					default:
						break;
					}
				}

				return  sx->mValue.cast_list();
			}
		}
	}
}