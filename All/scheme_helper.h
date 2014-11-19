#include "sexp.hpp"

#define		Sprintf std::sprintf
#define     Sscanf  std::sscanf
#define     S(x) x
#define		Strtof std::strtof
#define		Strtol std::strtol

#include <fstream>
#include "leomath.hpp"
namespace leo
{

	template<typename string_type>
	float3 expack_float3(std::shared_ptr<scheme::sexp::sexp> sexp, const string_type& name)
	{

		sexp = scheme::sexp::ops::find_sexp(name, sexp);
		float3 result;
		auto f = sexp->mNext;
		result.x =static_cast<float>(f->mValue.cast_atom<scheme::sexp::sexp_real>());
		f = f->mNext;
		result.y = static_cast<float>(f->mValue.cast_atom<scheme::sexp::sexp_real>());
		f = f->mNext;
		result.z = static_cast<float>(f->mValue.cast_atom<scheme::sexp::sexp_real>());
		return result;
	}

	template<typename string_type>
	float3 expack_float2(std::shared_ptr<scheme::sexp::sexp> sexp, const string_type& name)
	{
		sexp = scheme::sexp::ops::find_sexp(name, sexp);
		float2 result;
		auto f = sexp->mNext;
		result.x = static_cast<float>(f->mValue.cast_atom<scheme::sexp::sexp_real>());
		f = f->mNext;
		result.y = static_cast<float>(f->mValue.cast_atom<scheme::sexp::sexp_real>());
		return result;
	}

	template<typename string_type>
	float expack_float(std::shared_ptr<scheme::sexp::sexp> sexp, const string_type& name)
	{
		sexp = scheme::sexp::ops::find_sexp(name, sexp);
		float result;
		auto f = sexp->mNext;
		result = static_cast<float>(f->mValue.cast_atom<scheme::sexp::sexp_real>());
		return result;
	}

	template<typename string_type>
	long expack_long(std::shared_ptr<scheme::sexp::sexp> sexp, const string_type& name)
	{
		sexp = scheme::sexp::ops::find_sexp(name, sexp);
		long result;
		auto f = sexp->mNext;
		result = static_cast<long>(f->mValue.cast_atom<scheme::sexp::sexp_int>());
		return result;
	}

	scheme::sexp::sexp_string expack_string(std::shared_ptr<scheme::sexp::sexp> sexp, const scheme::sexp::sexp_string& name)
	{
		sexp = scheme::sexp::ops::find_sexp(name, sexp);
		return sexp->mNext->mValue.cast_atom<scheme::sexp::sexp_string>();
	}

	SQT expack_SQT(std::shared_ptr<scheme::sexp::sexp> sexp)
	{
		SQT temp;
		temp.s = expack_float(sexp,S("s"));
		temp.t = expack_float3(sexp, S("t"));
		temp.q = expack_float3(sexp,S("q"));

		return SQT(temp);
	}

	template<typename string_type>
	std::shared_ptr<scheme::sexp::sexp> parse_file(const string_type& name)
	{

		std::basic_ifstream<char> fin(name);

		if (!fin.good())
			throw std::runtime_error("打开文件失败");

		fin.seekg(0,std::ios_base::end);
		auto size = static_cast<std::size_t>(fin.tellg().seekpos());
		fin.seekg(3, std::ios_base::beg);
		std::vector<char> buffer((size + 1), char());
		fin.read(reinterpret_cast<char*>(&buffer[0]), size);
		auto tokens =scheme::sexp::lexicalanalysis(&buffer[0], size);
		return scheme::sexp::parse(tokens);
	}

	inline scheme::sexp::sexp_list make_sexp(const uint16 & value)
	{
		return leo::make_shared<scheme::sexp::sexp>(static_cast<scheme::sexp::sexp_int>(value));
	}

	template<typename string_type,typename value_type>
	std::shared_ptr<scheme::sexp::sexp> pack_key_value(const string_type& key, const value_type& value){
		auto result = scheme::sexp::make_sexp_word(key);
		
		using scheme::sexp::make_sexp;
		using leo::make_sexp;

		result->mNext = make_sexp(value);
		return result;
	}
}