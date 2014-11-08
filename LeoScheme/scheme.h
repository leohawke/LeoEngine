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
		using sexp::to_string;
		struct scheme_pair;

		using scheme_list = std::shared_ptr < scheme_pair > ;

		extern scheme_list scheme_nil;

		scheme_list make_copy(sexp::sexp_list sexp_list);

		struct scheme_atom
		{
			using atom_t = sexp::sexp_value::atom_t;
			leo::any mValue;
			atom_t mType;

			scheme_atom(const scheme_bool& b)
				:mValue(b), mType(atom_t::atom_bool)
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
				assert(t == atom_t::atom_word || t == atom_t::atom_string);
			}

			scheme_atom(const sexp::sexp_value& sexp_value)
				:mValue(sexp_value.mAtomValue), mType(sexp_value.mAtomType)
			{
				assert(sexp_value.mValueType == sexp::sexp_value::any_atom);
			}

			template<typename T>
			bool can_cast() const
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
					return(mAtomType == atom_t::atom_string || mAtomType == atom_t::atom_word);
				return false;
			}

			template<typename T>
			const T& cast() const
			{
				return *any_cast<const T*>(&mValue);
			}

			template<typename T>
			T& cast()
			{
				return *any_cast<T*>(&mValue);
			}

			bool no_word() const
			{
				return mType != atom_t::atom_word;
			}

			bool word() const
			{
				return mType == atom_t::atom_word;
			}

			scheme_string to_word() const;
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

			scheme_atom cast_atom() const
			{
				assert(mType == any_t::any_atom);
				return any_cast<const scheme_atom>(mValue);
			}

			scheme_list cast_list() const
			{
				assert(mType == any_t::any_list);
				return any_cast<const scheme_list>(mValue);
			}

			template<typename T>
			bool can_cast() const
			{
				if (typeid(T) == typeid(scheme_atom))
					return mType == any_t::any_atom;
				if (typeid(T) == typeid(scheme_list))
					return mType == any_t::any_list;
				return false;
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

		scheme_value eval(const scheme_value& exp, scheme_list& env);
		scheme_value apply(const scheme_list& procedure, const scheme_list& arguments);

		scheme_value read();
		void write(const scheme_value& exp);


		inline scheme_value car(const scheme_list& list){
			return list->mCar;
		}

		inline scheme_value car(const scheme_value& list){
			assert(list.mType == scheme_value::any_t::any_list);
			return list.cast_list()->mCar;
		}

		inline scheme_value cdr(const scheme_list& list){
			return list->mCdr;
		}

		inline scheme_value cdr(const scheme_value& list){
			assert(list.mType == scheme_value::any_t::any_list);
			return list.cast_list()->mCdr;
		}

		inline scheme_list cons(const scheme_value& car, const scheme_value& cdr){
			return make_shared<scheme_pair>(car, cdr);
		}

		inline scheme_list list(const scheme_value& car)
		{
			return cons(car,scheme_nil);
		}

		inline scheme_list list()
		{
			return scheme_nil;
		}

		template<typename... Value>
		inline scheme_list list(const scheme_value& head, Value&... tail)
		{
			return cons(head,list(tail...));
		}

		template<typename T>
		inline scheme_value cadr(const T& list){
			static_assert(std::is_same<T, scheme_value>::value || std::is_same<T, scheme_list>::value, "T is scheme_value or scheme_list");
			auto tail = cdr(list);
			return car(tail);
		}

		template<typename T>
		inline scheme_value caddr(const T& list){
			static_assert(std::is_same<T, scheme_value>::value || std::is_same<T, scheme_list>::value, "T is scheme_value or scheme_list");
			auto tail = cdr(list);
			tail = cdr(tail);
			return car(tail);
		}

		template<typename T>
		inline scheme_value cddr(const T& list){
			static_assert(std::is_same<T, scheme_value>::value || std::is_same<T, scheme_list>::value, "T is scheme_value or scheme_list");
			return cdr(cdr(list));
		}

		template<typename T>
		inline scheme_value cdadr(const T& list){
			static_assert(std::is_same<T, scheme_value>::value || std::is_same<T, scheme_list>::value, "T is scheme_value or scheme_list");
			return cdr(car(cdr(list)));
		}

		template<typename T>
		inline scheme_value cdddr(const T& list){
			static_assert(std::is_same<T, scheme_value>::value || std::is_same<T, scheme_list>::value, "T is scheme_value or scheme_list");
			return cdr(cdr(cdr(list)));
		}

		template<typename T>
		inline scheme_value cadddr(const T& list){
			static_assert(std::is_same<T, scheme_value>::value || std::is_same<T, scheme_list>::value, "T is scheme_value or scheme_list");
			return car(cdddr(list));
		}

		template<typename T>
		inline scheme_value caadr(const T& list){
			static_assert(std::is_same<T, scheme_value>::value || std::is_same<T, scheme_list>::value, "T is scheme_value or scheme_list");
			return car(car(cdr(list)));
		}

		inline bool null(const scheme_value& exp){
			return exp.cast_list() == scheme_nil;
		}

		inline bool null(const scheme_list& list){
			return list == scheme_nil;
		}

		bool is_list(const scheme_value& value);
		bool pair(const scheme_value& value);
		bool symbol(const scheme_value& exp);

		namespace ops{
			scheme_string print(const scheme_list & list);
			scheme_string print(const scheme_value & value);
		}
	}
}

