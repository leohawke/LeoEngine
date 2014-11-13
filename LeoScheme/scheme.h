#ifndef LEO_SCHEME_H
#define LEO_SCHEME_H
#include "Sexp\Sexp.hpp"
#ifdef LB_IMPL_MSCPP
#pragma warning(push)
#pragma warning(disable:4800)
#endif
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
				if (typeid(T) == typeid(scheme_bool))
					return mType == atom_t::atom_bool;
				if (typeid(T) == typeid(scheme_char))
					return mType == atom_t::atom_char;
				if (typeid(T) == typeid(scheme_int))
					return mType == atom_t::atom_int;
				if (typeid(T) == typeid(scheme_real))
					return mType == atom_t::atom_real;
				if (typeid(T) == typeid(scheme_string))
					return(mType == atom_t::atom_string || mType == atom_t::atom_word);
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

		inline bool operator==(const scheme_atom& lhs, const scheme_atom& rhs){
			if (lhs.mType == rhs.mType){
				switch (lhs.mType)
				{
				case scheme_atom::atom_t::atom_bool:
					return lhs.cast<scheme_bool>() == rhs.cast<scheme_bool>();
				case scheme_atom::atom_t::atom_char:
					return lhs.cast<scheme_char>() == rhs.cast<scheme_char>();
				case scheme_atom::atom_t::atom_int:
					return lhs.cast<scheme_int>() == rhs.cast<scheme_int>();
				case scheme_atom::atom_t::atom_real:
					return lhs.cast<scheme_real>() == rhs.cast<scheme_real>();
				case scheme_atom::atom_t::atom_string:
				case scheme_atom::atom_t::atom_word:
					return lhs.cast<scheme_string>() == rhs.cast<scheme_string>();
				}
			}
			return false;
		}

		

		struct scheme_value
		{
			enum class any_t {
				any_list,
				any_atom,
				any_ptr,
				any_cpp_temp
			};
			leo::any mValue = scheme_list();
			any_t mType = any_t::any_list;

			scheme_value() = default;

			scheme_value(scheme_atom atom)
				:mValue(atom), mType(any_t::any_atom)
			{}

			scheme_value(scheme_list list)
				:mValue(list), mType(any_t::any_list)
			{}

			scheme_value(const scheme_value* ptr)
				:mValue(ptr),mType(any_t::any_ptr){
			}

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

			scheme_value& operator=(const scheme_value* ptr){
				mValue = ptr;
				mType = any_t::any_ptr;
				return *this;
			}

			const scheme_atom& cast_atom() const
			{
				assert(mType == any_t::any_atom || mType == any_t::any_ptr);
				return mType == any_t::any_ptr ? (*any_cast<const scheme_value*const*>(&mValue))->cast_atom() : *any_cast<const scheme_atom*>(&mValue);
			}

			const scheme_list& cast_list() const
			{
				assert(mType == any_t::any_list || mType == any_t::any_ptr);
				return mType == any_t::any_ptr ? (*any_cast<const scheme_value*const*>(&mValue))->cast_list() : *any_cast<const scheme_list*>(&mValue);
			}

			scheme_atom& cast_atom()
			{
				assert(mType == any_t::any_atom || mType == any_t::any_ptr);
				return mType == any_t::any_ptr ? (*any_cast<scheme_value**>(&mValue))->cast_atom() : *any_cast<scheme_atom*>(&mValue);
			}

			scheme_list& cast_list()
			{
				assert(mType == any_t::any_list || mType == any_t::any_ptr);
				return mType == any_t::any_ptr ? (*any_cast<scheme_value**>(&mValue))->cast_list() : *any_cast<scheme_list*>(&mValue);
			}

			template<typename T>
			bool can_cast() const{
				return true;
			}

			template<>
			bool can_cast<scheme_atom>() const{
				return mType == any_t::any_atom || (mType == any_t::any_ptr &&  any_cast<const scheme_value* const>(mValue)->can_cast<scheme_atom>());
			}

			template<>
			bool can_cast<scheme_list>() const{
				return mType == any_t::any_list || (mType == any_t::any_ptr &&  any_cast<const scheme_value* const>(mValue)->can_cast<scheme_list>());
			}
		};

		inline bool operator==(const scheme_value& lhs, const scheme_value& rhs){
			if (lhs.can_cast<scheme_list>() && rhs.can_cast<scheme_list>())
				return lhs.cast_list() == rhs.cast_list();
			else if ((lhs.can_cast<scheme_atom>() && rhs.can_cast<scheme_atom>()))
				return lhs.cast_atom() == rhs.cast_atom();
			else
				return false;
		}

		struct scheme_pair
		{
			scheme_value mCar;
			scheme_value mCdr;

			scheme_pair(const scheme_value& car, const scheme_value& cdr)
				:mCar(car), mCdr(cdr){
			}

			scheme_pair() = default;
		};

		inline scheme_list make_scheme_list()
		{
			return leo::make_shared<scheme_pair>();
		}

		scheme_value eval(const scheme_value& exp, scheme_list& env);
		scheme_value apply(const scheme_list& procedure, const scheme_list& arguments);

		scheme_value read();
		void write(const scheme_value& exp);

		inline const scheme_value& car(const scheme_value& list){
			assert(list.mType == scheme_value::any_t::any_list);
			return list.cast_list()->mCar;
		}

		inline scheme_value& car(scheme_value& list){
			assert(list.mType == scheme_value::any_t::any_list);
			return list.cast_list()->mCar;
		}

		inline const scheme_value& cdr(const scheme_value& list){
			assert(list.mType == scheme_value::any_t::any_list);
			return list.cast_list()->mCdr;
		}

		inline scheme_value& cdr(scheme_value& list){
			assert(list.mType == scheme_value::any_t::any_list);
			return list.cast_list()->mCdr;
		}

		inline scheme_list cons(const scheme_value& car, const scheme_value& cdr){
			return make_shared<scheme_pair>( car, cdr );
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

		inline const scheme_value& cadr(const scheme_value& list){
			return car(cdr(list));
		}

		inline scheme_value& cadr(scheme_value& list){
			return car(cdr(list));
		}

		inline const scheme_value& caddr(const scheme_value& list){
			return car(cdr(cdr(list)));
		}

		inline scheme_value& caddr(scheme_value& list){
			return car(cdr(cdr(list)));
		}

		inline const scheme_value& cddr(const scheme_value& list){
			return cdr(cdr(list));
		}

		inline scheme_value& cddr(scheme_value& list){
			return cdr(cdr(list));
		}

		inline const scheme_value& cdadr(const scheme_value& list){
			return cdr(car(cdr(list)));
		}

		inline scheme_value& cdadr(scheme_value& list){
			return cdr(car(cdr(list)));
		}

		inline const scheme_value&  cdddr(const scheme_value& list){
			return cdr(cdr(cdr(list)));
		}

		inline scheme_value&  cdddr(scheme_value& list){
			return cdr(cdr(cdr(list)));
		}

		inline const scheme_value& cadddr(const scheme_value& list){
			return car(cdddr(list));
		}

		inline scheme_value& cadddr(scheme_value& list){
			return car(cdddr(list));
		}

		inline  const scheme_value& caadr(const scheme_value& list){
			return car(car(cdr(list)));
		}

		inline  scheme_value& caadr(scheme_value& list){
			return car(car(cdr(list)));
		}

		inline void set_car(scheme_value& pair, const scheme_value& car){
			pair.cast_list()->mCar = car;
		}

		inline void set_cdr(scheme_value& pair, const scheme_value& cdr){
			pair.cast_list()->mCdr = cdr;
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
		bool tagged_list(const scheme_value& exp, const scheme_string& tag);
		inline bool compound_procedure(const scheme_list& procedure){
			return tagged_list(procedure, "procedure");
		}
		inline scheme_value procedure_body(const scheme_value& procedure){
			return caddr(procedure);
		}

		inline scheme_value procedure_parameters(const scheme_value& procedure){
			return cadr(procedure);
		}
		inline scheme_int length(const scheme_value& exp){
			assert(exp.can_cast<scheme_list>());

			auto l = 0u;
			auto rest = exp;
			for (; rest.cast_list() != scheme_nil;){
				++l;
				rest = cdr(rest);
			}

			return l;
		}

		scheme_list setup_environment();


		

		namespace ops{
			scheme_string print(const scheme_list & list);
			scheme_string print(const scheme_value & value);
		}


		inline scheme_string error_list(const scheme_value& exp)
		{
			return ops::print(exp);
		}

		inline scheme_string error_list()
		{
			return scheme_string();
		}

		template<typename... Lists>
		inline scheme_string error_list(const scheme_value& exp, Lists&... lists)
		{
			return error_list(exp) + error_list(lists...);
		}

		template<typename... Lists>
		inline void error(const scheme_string& msg, Lists&... lists)
		{
			throw std::runtime_error(msg + error_list(lists...));
		}

		inline bool true_exp(const scheme_value& exp){
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

		inline bool number(const scheme_value& exp){
			return exp.can_cast<scheme_atom>() && (exp.cast_atom().can_cast<scheme_int>() || exp.cast_atom().can_cast<scheme_real>());
		}

		inline bool number(const scheme_atom& atom){
			return atom.can_cast<scheme_int>() || atom.can_cast<scheme_real>();
		}
	}
}

#ifdef LB_IMPL_MSCPP
#pragma warning(pop)
#endif

#endif