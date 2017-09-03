/*! \file typeindex.h
\ingroup LBase
\brief ISO C++ 类型信息扩展包装。
*/
#ifndef LBase_typeindex_h
#define LBase_typeindex_h 1

#include "typeinfo.h"
#include <typeindex>
namespace leo {
	class LB_API type_index {
	public:
		type_index(const type_info& info)
			:pinfo(&info) {
		}

		size_t hash_code() const lnothrow
		{	// return hash value
			return (pinfo->hash_code());
		}

		const char *name() const lnothrow
		{	// return name
			return (pinfo->name());
		}

		bool
			operator==(const type_index& rhs) const lnothrow
		{
			return *pinfo == *rhs.pinfo;
		}

		bool
			operator!=(const type_index& rhs) const lnothrow
		{
			return (!(*this == rhs));
		}

		bool operator<(const type_index& rhs) const lnothrow
		{	// test if *this < rhs
			return (pinfo->before(*rhs.pinfo));
		}

		bool operator>=(const type_index& rhs) const lnothrow
		{	// test if *this >= rhs
			return (!(*this < rhs));
		}

		bool operator>(const type_index& rhs) const lnothrow
		{	// test if *this > rhs
			return (rhs < *this);
		}

		bool operator<=(const type_index& rhs) const lnothrow
		{	// test if *this <= rhs
			return (!(rhs < *this));
		}
	private:
		const type_info* pinfo;
	};
}

namespace std {
	template<>
	struct hash<leo::type_index>
	{	// hash functor for type_index
		using argument_type = leo::type_index;
		using result_type = size_t;

		result_type operator()(const argument_type& keyval) const lnoexcept
		{
			return (keyval.hash_code());
		}
	};
}

#endif