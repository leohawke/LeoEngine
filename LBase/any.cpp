/*! \file any.cpp
\ingroup LBase
\brief 动态泛型类型。
*/
#include  "LBase/any.h"
#include "LBase/cassert.h"

namespace leo
{
	namespace any_ops
	{

		holder::~holder() = default;

	} // namespace any_ops;


	bad_any_cast::~bad_any_cast() = default;

	const char*
		bad_any_cast::from() const lnothrow
	{
		return
			from_type() == type_id<void>() ? "unknown" : from_type().name();
	}

	const char*
		bad_any_cast::to() const lnothrow
	{
		return to_type() == type_id<void>() ? "unknown" : to_type().name();
	}

	const char*
		bad_any_cast::what() const lnothrow
	{
		return "Failed conversion: any_cast.";
	}


	namespace details
	{

		any_ops::any_storage&
			any_base::call(any_ops::any_storage& t, any_ops::op_code op) const
		{
			lconstraint(manager);

			manager(t, storage, op);
			return t;
		}

		void
			any_base::clear() lnothrowv
		{
			lconstraint(manager);

			manager(storage, storage, any_ops::destroy);
			manager = {};
		}

		void
			any_base::copy(const any_base& a)
		{
			lconstraint(manager);

			manager(storage, a.storage, any_ops::clone);
		}

		void
			any_base::destroy() lnothrowv
		{
			lconstraint(manager);

			manager(storage, storage, any_ops::destroy);
		}

		void*
			any_base::get() const lnothrowv
		{
			return unchecked_access<void*>(any_ops::get_ptr);
		}

		any_ops::holder*
			any_base::get_holder() const
		{
			return unchecked_access<any_ops::holder*>(any_ops::get_holder_ptr);
		}

		void
			any_base::swap(any_base& a) lnothrow
		{
			std::swap(storage, a.storage),
				std::swap(manager, a.manager);
		}

	} // namespace details;


	any::any(const any& a)
		: any_base(a)
	{
		if (manager)
			copy(a);
	}
	any::~any()
	{
		if (manager)
			destroy();
	}

	void
		any::reset() lnothrow
	{
		if (manager)
			clear();
	}
}