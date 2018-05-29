#include "LObject.h"
#include <LBase/cast.hpp>

namespace leo {

	ImplDeDtor(IValueHolder)

	namespace
	{

		inline PDefH(IValueHolder&, HolderDownCast, leo::any_ops::holder& h)
			lnothrowv
			ImplRet(leo::polymorphic_downcast<IValueHolder&>(h))
			inline PDefH(const IValueHolder&, HolderDownCast,
				const leo::any_ops::holder& h) lnothrowv
			ImplRet(leo::polymorphic_downcast<const IValueHolder&>(h))

			inline PDefH(bool, HolderEquals, leo::any_ops::holder& h, const void* p)
			ImplRet(HolderDownCast(h).Equals(p))

	} // unnamed namespace;

	IValueHolder*
		ValueObject::GetHolderPtr() const
	{
		return leo::polymorphic_downcast<IValueHolder*>(content.get_holder());
	}
	IValueHolder&
		ValueObject::GetHolderRef() const
	{
		return HolderDownCast(Deref(content.get_holder()));
	}

	ValueObject
		ValueObject::Create(IValueHolder::Creation c) const
	{
		return leo::call_value_or([c](const IValueHolder& h) {
			return ValueObject(h, c);
		}, GetHolderPtr());
	}

	bool
		ValueObject::Equals(const ValueObject& x) const
	{
		const auto p_holder(content.get_holder());

		if (const auto q = x.content.get_holder())
			return p_holder == q || (p_holder && p_holder->type() == q->type()
				&& HolderEquals(*p_holder, q->get()));
		return !p_holder;
	}

	bool
		ValueObject::EqualsRaw(const void* p) const
	{
		if (const auto p_holder = content.get_holder())
			return HolderEquals(*p_holder, p);
		return !p;
	}

	bool
		ValueObject::EqualsUnchecked(const void* p) const
	{
		return HolderEquals(GetHolderRef(), p);
	}

	size_t
		ValueObject::OwnsCount() const lnothrow
	{
		return leo::invoke_value_or(&IValueHolder::OwnsCount, GetHolderPtr());
	}

}
