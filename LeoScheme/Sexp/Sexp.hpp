////////////////////////////////////////////////////////////////////////////
//
//  Sexp Head File<Leo Engine Script Impl>
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   LeoScheme/Sexp/Sexp.hpp
//  Version:     v1.00
//  Created:     9/1/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供引擎脚本的底层数据表示,基于S表达式
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef LeoScheme_Sexp_hpp
#define LeoScheme_Sexp_hpp


#include <IndePlatform\any.h>
#include <IndePlatform\memory.hpp>
#include <string>
#include <queue>

namespace leo
{
	namespace scheme
	{
		namespace sexp
		{
			using sexp_bool = bool;
			using sexp_char = char;
			using sexp_int = std::int64_t;
			using sexp_real = std::double_t;
			using sexp_string = std::string;
			using std::to_string;
			struct sexp;
			using sexp_list = std::shared_ptr < sexp > ;
			using const_sexp_list = std::shared_ptr < const sexp > ;

			struct sexp_value
			{
				enum any_t{
					any_atom,
					any_list
				};
				enum atom_t{
					//host impl : bool
					atom_bool,
					//host impl : std::int64_t
					atom_int,
					//host impl : double
					atom_real,
					//host impl : char
					atom_char,
					//host impl : std::string
					//encode : UTF-8
					atom_string,
					//host impl : std::string
					//encode : UTF-8
					atom_word
				};

				struct 
				{
					struct {
						leo::any mAtomValue;
						atom_t mAtomType;
					};

					sexp_list mListValue = nullptr;
				};

				any_t mValueType = any_list;

				sexp_value() = default;

				sexp_value(const sexp_bool& b)
					:mAtomValue(b), mValueType(any_atom), mAtomType(atom_bool)
				{}

				sexp_value(const sexp_char& c)
					:mAtomValue(c), mValueType(any_atom), mAtomType(atom_char)
				{}

				sexp_value(const sexp_int& i)
					:mAtomValue(i), mValueType(any_atom), mAtomType(atom_int)
				{}

				sexp_value(const sexp_real& r)
					:mAtomValue(r), mValueType(any_atom), mAtomType(atom_real)
				{}

				sexp_value(const sexp_string& s,atom_t t)
					:mAtomValue(s), mValueType(any_atom), mAtomType(t)
				{
					assert(t == atom_word || t == atom_string);
				}

				sexp_value(const sexp_value& rhs)
					:mValueType(rhs.mValueType)
				{
					if (mValueType == any_list)
						mListValue = rhs.mListValue;
					else
					{
						mAtomValue = rhs.mAtomValue;
						mAtomType = rhs.mAtomType;
					}
				}

				sexp_value& operator=(const sexp_value& rhs)
				{
					mValueType = rhs.mValueType;
					if (mValueType == any_list)
						mListValue = rhs.mListValue;
					else
					{
						mAtomValue = rhs.mAtomValue;
						mAtomType = rhs.mAtomType;
					}
					return *this;
				}

				sexp_value(const sexp_list& l)
					:mListValue(l), mValueType(any_list)
				{}

				template<typename T>
				bool can_cast() const
				{
					if (typeid(T) == typeid(sexp_bool))
						return mValueType == any_atom && mAtomType == atom_bool;
					if (typeid(T) == typeid(sexp_char))
						return mValueType == any_atom && mAtomType == atom_char;
					if (typeid(T) == typeid(sexp_int))
						return mValueType == any_atom && mAtomType == atom_int;
					if (typeid(T) == typeid(sexp_real))
						return mValueType == any_atom && mAtomType == atom_real;
					if (typeid(T) == typeid(sexp_string))
						return mValueType == any_atom && (mAtomType == atom_string || mAtomType == atom_word);
					if (typeid(T) == typeid(sexp_list))
						return mValueType == any_list;
					return false;
				}

				template<typename T>
				T cast_atom()
				{
					assert(mValueType == any_atom);
					return any_cast<T>(mAtomValue);
				}

				sexp_list cast_list()
				{
					assert(mValueType == any_list);
					return mListValue;
				}

			};

			struct sexp
			{
				sexp_value mValue;

				sexp_list mNext = nullptr;

				sexp()
				{}

				sexp(const sexp_bool& b)
					:mValue(b)
				{}

				sexp(const sexp_char& c)
					:mValue(c)
				{}

				sexp(const sexp_int& i)
					:mValue(i)
				{}

				sexp(const sexp_real& r)
					:mValue(r)
				{}

				sexp(const sexp_string& s,sexp_value::atom_t t)
					:mValue(s,t)
				{}

				sexp(const sexp& rhs)
					:mValue(rhs.mValue)
				{}

				sexp& operator=(const sexp& rhs)
				{
					mValue = rhs.mValue;
				}

				sexp(const sexp_list& l)
					:mValue(l)
				{}
			};

			struct token
			{
				using list = std::queue<token>;
				/*
				token类型:
				left_split: 左分隔符
				right_split : 右分隔符
				word : 变量名,函数名,关键字,宏
				*/
				enum ty_t { 
					//scheme impl : '('
					left_split,
					//scheme impl : ')'
					right_split, 
					word,
					string
				} ty = word;

				sexp_string mWordValue;

				token(ty_t t)
					:ty(t)
				{}
				token(sexp_string && val)
					:mWordValue(std::move(val))
				{}
			};

			sexp_string restore_escape_string(const sexp_string& str);
			sexp_string store_escape_string(const sexp_string& str);

			token::list lexicalanalysis(const sexp_char * str, std::size_t len);

			inline token::list lexicalanalysis(const sexp_string& str)
			{
				return lexicalanalysis(str.c_str(), str.length());
			}

			template<std::size_t SIZE>
			inline token::list lexicalanalysis(const sexp_char (&str)[SIZE])
			{
				return lexicalanalysis(str, SIZE);
			}

			//ops
			namespace ops
			{
				sexp_list make_copy(const const_sexp_list & s);

				sexp_list find_sexp(const sexp_char * word, const sexp_list & s);

				inline sexp_list find_sexp(const sexp_string& word, const sexp_list & s)
				{
					return find_sexp(word.c_str(), s);
				}

				sexp_list bfs_find_sexp(const sexp_char * word, const sexp_list & s);

				inline sexp_list bfs_find_sexp(const sexp_string& word, const sexp_list & s)
				{
					return bfs_find_sexp(word.c_str(), s);
				}

				std::size_t sexp_list_length(const const_sexp_list & s);

				sexp_string print_sexp(const const_sexp_list & s);
			}
			//endops

			template<typename T>
			inline sexp_list make_sexp(const T & value)
			{
				return leo::make_shared<sexp>(value);
			}

			inline sexp_list make_sexp(const sexp_string& value)
			{
				return leo::make_shared<sexp>(value, sexp_value::atom_string);
			}

			inline sexp_list make_sexp(const char *& value){
				return leo::make_shared<sexp>(sexp_string(value), sexp_value::atom_string);
			}

			template<std::size_t size>
			inline sexp_list make_sexp(const char (&value)[size]){
				return leo::make_shared<sexp>(sexp_string(value), sexp_value::atom_string);
			}

			template<std::size_t size>
			inline sexp_list make_sexp_word(const char(&value)[size]){
				return leo::make_shared<sexp>(sexp_string(value), sexp_value::atom_word);
			}


			inline sexp_list make_sexp(std::nullptr_t)
			{
				return leo::make_shared<sexp>(sexp_list());
			}

			sexp_list parse(token::list& tokens);
		}
	}
}
#endif