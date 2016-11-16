#include "LObject.h"
#include <LBase/cast.hpp>

namespace leo {

	ImplDeDtor(IValueHolder)
		bool
		operator==(const ValueObject& x, const ValueObject& y)
	{
		const auto p(x.content.get_holder());

		if (const auto q = y.content.get_holder())
			return p == q
			|| (p && polymorphic_downcast<const IValueHolder&>(*p)
				== polymorphic_downcast<const IValueHolder&>(*q));
		else
			return !p;
		return{};
	}
}
