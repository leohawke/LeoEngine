#include  "any.h"

namespace leo
{
	any::any(const any& a)
		: any()
	{
		if (a)
		{
			manager = a.manager,
				a.manager(storage, a.storage, any_ops::clone);
		}
	}
	any::~any()
	{
		if (manager)
			manager(storage, storage, any_ops::destroy);
	}

	void*
		any::get() const lnothrow
	{
		if (manager)
		{
			any_ops::any_storage t;

			manager(t, storage, any_ops::get_ptr);
			return t.access<void*>();
		}
		return{};
	}

		any_ops::holder*
		any::get_holder() const
	{
		if (manager)
		{
			any_ops::any_storage t;

			manager(t, storage, any_ops::get_holder_ptr);
			return t.access<any_ops::holder*>();
		}
		return{};
	}

	void
		any::clear() lnothrow
	{
		if (manager)
		{
			manager(storage, storage, any_ops::destroy);
			manager = {};
		}
	}

		void
		any::swap(any& a) lnothrow
	{
		leo::swap(storage, a.storage);
		std::swap(manager, a.manager);
	}

		const std::type_info&
		any::type() const lnothrow
	{
		if (manager)
		{
			any_ops::any_storage t;

			manager(t, storage, any_ops::get_type);
			return *t.access<const std::type_info*>();
		}
		return typeid(void);
	}
}