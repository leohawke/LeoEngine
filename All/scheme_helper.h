#include "sexp.hpp"
#include <fstream>
#include "leomath.hpp"
namespace leo
{

	template<typename string_type>
	float3 expack_float3(std::shared_ptr<scheme::sexp::sexp> sexp, const string_type& name)
	{
		static_assert(
			std::is_same<scheme::sexp::char_s, std::remove_cv<std::remove_all_extents<string_type>::type>::type>::value ||
			std::is_same<string_type, scheme::sexp::string_s>::value, "unsupport type");

		sexp = scheme::sexp::find_sexp(name, sexp);
		float3 result;
		auto f = sexp->next;
		result.x = Strtof(f->value.c_str(), 0);
		f = f->next;
		result.y = Strtof(f->value.c_str(), 0);
		f = f->next;
		result.z = Strtof(f->value.c_str(), 0);
		return result;
	}

	template<typename string_type>
	float3 expack_float2(std::shared_ptr<scheme::sexp::sexp> sexp, const string_type& name)
	{
		static_assert(
			std::is_same<scheme::sexp::char_s, std::remove_cv<std::remove_all_extents<string_type>::type>::type>::value ||
			std::is_same<string_type, scheme::sexp::string_s>::value, "unsupport type");
		sexp = scheme::sexp::find_sexp(name, sexp);
		float2 result;
		auto f = sexp->next;
		result.x = Strtof(f->value.c_str(), 0);
		f = f->next;
		result.y = Strtof(f->value.c_str(), 0);
		return result;
	}

	template<typename string_type>
	float expack_float(std::shared_ptr<scheme::sexp::sexp> sexp, const string_type& name)
	{
		static_assert(
		std::is_same<scheme::sexp::char_s, std::remove_cv<std::remove_all_extents<string_type>::type>::type>::value ||
			std::is_same<string_type,scheme::sexp::string_s>::value,"unsupport type");
		sexp = scheme::sexp::find_sexp(name, sexp);
		float result;
		auto f = sexp->next;
		result = Strtof(f->value.c_str(), 0);
		return result;
	}

	template<typename string_type>
	long expack_long(std::shared_ptr<scheme::sexp::sexp> sexp, const string_type& name)
	{
		static_assert(
			std::is_same<scheme::sexp::char_s, std::remove_cv<std::remove_all_extents<string_type>::type>::type>::value ||
			std::is_same<string_type, scheme::sexp::string_s>::value, "unsupport type");
		sexp = scheme::sexp::find_sexp(name, sexp);
		long result;
		auto f = sexp->next;
		result = Strtol(f->value.c_str(), 0,10);
		return result;
	}

	scheme::sexp::string_s expack_string(std::shared_ptr<scheme::sexp::sexp> sexp, const scheme::sexp::string_s& name)
	{
		sexp = scheme::sexp::find_sexp(name, sexp);
		return sexp->next->value;
	}

	const scheme::sexp::char_s * expack_string(std::shared_ptr<scheme::sexp::sexp> sexp, const scheme::sexp::char_s * name)
	{
		sexp = scheme::sexp::find_sexp(name, sexp);
		return sexp->next->value.c_str();
	}

	SQT expack_SQT(std::shared_ptr<scheme::sexp::sexp> sexp)
	{
		SeqSQT temp;
		temp.s = expack_float(sexp,S("s"));
		temp.t = expack_float3(sexp, S("t"));
		temp.q = expack_float3(sexp,S("q"));

		return SQT(temp);
	}

	template<typename string_type>
	std::shared_ptr<scheme::sexp::sexp> parse_file(const string_type& name)
	{
		static_assert(
			std::is_same<scheme::sexp::char_s, std::remove_cv<std::remove_all_extents<string_type>::type>::type>::value ||
			std::is_same<string_type, scheme::sexp::string_s>::value, "unsupport type");

		std::basic_ifstream<char> fin(name);

		if (!fin.good())
			throw std::runtime_error("打开文件失败");

		fin.seekg(0,std::ios_base::end);
		auto size = fin.tellg().seekpos()*sizeof(scheme::sexp::char_s) / sizeof(char);
		fin.seekg(2, std::ios_base::beg);
		std::vector<scheme::sexp::char_s> buffer(static_cast<std::size_t>(size / sizeof(scheme::sexp::char_s)*sizeof(char)) + 1, scheme::sexp::char_s());
		fin.read(reinterpret_cast<char*>(&buffer[0]), size);
		return scheme::sexp::parse(&buffer[0]);
	}
}