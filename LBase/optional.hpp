/*!	\file optional.hpp
\ingroup LBase
\brief 可选值包装类型。
\see http://www.boost.org/doc/libs/1_57_0/libs/optional/doc/html/optional/reference.html 。


*/

#ifndef LBase_Optional_hpp
#define LBase_Optional_hpp 1

#include "LBase/functional.hpp"
#include <optional>

namespace leo {
	using std::optional;
	using std::nullopt_t;
	using std::nullopt;
	using std::bad_optional_access;


	/*!
	\brief 合并可选值序列。
	\note 语义同 Boost.Signal2 的 \c boost::optional_last_value 。
	*/
	//@{
	template<typename _type>
	struct optional_last_value
	{
		template<typename _tIn>
		optional<_type>
			operator()(_tIn first, _tIn last) const
		{
			return std::accumulate(first, last, optional<_type>(),
				[](optional<_type>&, decltype(*first) val) {
				return lforward(val);
			});
		}
	};

	template<>
	struct optional_last_value<void> : default_last_value<void>
	{};
	//@}
}

#endif
