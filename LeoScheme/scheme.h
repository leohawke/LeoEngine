#pragma once
#include "Sexp\Sexp.hpp"

namespace leo
{
	namespace scheme
	{
		//because scheme_list != sexp_list
		using scheme_bool = sexp::sexp_bool;
		using scheme_char = sexp::sexp_char;
		using scheme_int = sexp::sexp_int;
		using scheme_real = sexp::sexp_real;
		using scheme_string = sexp::sexp_string;

		struct scheme_pair;

		using scheme_list = std::shared_ptr < scheme_pair > ;

		extern scheme_list schme_nil;

		scheme_list make_copy(sexp::sexp_list sexp_list);

		struct scheme_atom
		{
			using atom_t = sexp::sexp_value::atom_t;
			leo::any mValue;
			atom_t mType;

			scheme_atom(const scheme_bool& b)
				:mValue(b),mType(atom_t::atom_bool)
			{}

			scheme_atom(const scheme_char& c)
				:mValue(c), mType(atom_t::atom_char)
			{}

			scheme_atom(const scheme_int& i)
				:mValue(i), mType(atom_t::atom_int)
			{}

			scheme_atom(const scheme_real& r)
				:mValue(r), mType(atom_t::atom_real)
			{}

			scheme_atom(const scheme_string& s, atom_t t)
				:mValue(s), mType(t)
			{
				assert(t == atom_t::atom_word || t == atom_t:: atom_string);
			}

			scheme_atom(const sexp::sexp_value& sexp_value)
				:mValue(sexp_value.mAtomValue), mType(sexp_value.mAtomType)
			{
				assert(sexp_value.mValueType == sexp::sexp_value::any_atom);
			}

			template<typename T>
			bool can_cast()
			{
				if (typeid(T) == typeid(sexp_bool))
					return mAtomType == atom_t::atom_bool;
				if (typeid(T) == typeid(sexp_char))
					return mAtomType == atom_t::atom_char;
				if (typeid(T) == typeid(sexp_int))
					return mAtomType == atom_t::atom_int;
				if (typeid(T) == typeid(sexp_real))
					return mAtomType == atom_t::atom_real;
				if (typeid(T) == typeid(sexp_string))
					return( mAtomType == atom_t::atom_string ||  mAtomType == atom_t::atom_word);
				return false;
			}

			template<typename T>
			T& cast()
			{
				return *any_cast<T*>(&mValue);
			}

			bool no_word()
			{
				return mType != atom_t::atom_word;
			}
		};

		struct scheme_value
		{
			using any_t = sexp::sexp_value::any_t;
			leo::any mValue = scheme_list();
			any_t mType = any_t::any_list;

			scheme_value() = default;

			scheme_value(scheme_atom atom)
				:mValue(atom), mType(any_t::any_atom)
			{}

			scheme_value(scheme_list list)
				:mValue(list), mType(any_t::any_list)
			{}

			scheme_value& operator=(scheme_atom atom)
			{
				mValue = atom;
				mType = any_t::any_atom;
				return *this;
			}

			scheme_value& operator=(scheme_list list)
			{
				mValue = list;
				mType = any_t::any_list;
				return *this;
			}
		};

		struct scheme_pair
		{
			scheme_value mCar;
			scheme_value mCdr;
		};

		inline scheme_list make_scheme_list()
		{
			return leo::make_shared<scheme_pair>();
		}

		scheme_value eval(scheme_value& exp, scheme_list& env);
		void apply(scheme_list& procedure, scheme_list& arguments);
	}
}

