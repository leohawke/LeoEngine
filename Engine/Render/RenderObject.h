#pragma once

#include <atomic>
#include <LBase/lmemory.hpp>
#include <LBase/type_traits.hpp>

namespace platform::Render {
	class RObject
	{
	public:
		using value_type = std::atomic_uint32_t::value_type;


		virtual ~RObject();

		value_type AddRef()
		{
			return ++NumRefs;
		}

		uint32_t Release()
		{
			auto NewValue = --NumRefs;
			if (NewValue == 0)
			{
				delete this;
			}
			return NewValue;
		}

	private:
		std::atomic_uint32_t NumRefs = 1;
	};

	struct RObjectDeleter
	{
		void operator()(RObject* obj) const
		{
			obj->Release();
		}
	};

	template<typename T,limpl(typename = leo::enable_if_t<std::is_convertible_v<T*,RObject*>>)>
		std::shared_ptr<T> shared_raw_robject(T* p)
	{
			return leo::share_raw(p, RObjectDeleter());
	}

}