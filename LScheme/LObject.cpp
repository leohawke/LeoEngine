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

	ValueObject
		ValueObject::MakeIndirect() const
	{
		return ValueObject(leo::polymorphic_downcast<const IValueHolder&>(
			Deref(content.get_holder())), holder_refer_tag());
	}
}
