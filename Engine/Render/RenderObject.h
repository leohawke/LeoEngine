#pragma once

#include <LBase/type_traits.hpp>
#include <LBase/smart_ptr.hpp>
#include <LBase/linttype.hpp>
#include <atomic>
#include <stack>

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

		value_type Release()
		{
			auto NewValue = --NumRefs;
			if (NewValue == 0)
			{
				PendingDeletes.push(this);
			}
			return NewValue;
		}

		static void FlushPendingDeletes();

	private:
		std::atomic_uint32_t NumRefs = 1;

		static std::stack<RObject*> PendingDeletes;

		//D3D12 API don't do internal reference counting
		struct RObjectToDelete
		{
			RObjectToDelete(leo::uint32 InFrameDeleted = 0)
				: FrameDeleted(InFrameDeleted)
			{

			}

			std::vector<RObject*>	Objects;
			leo::uint32					FrameDeleted;
		};

		static std::vector<RObjectToDelete> DeferredDeletionQueue;
		static leo::uint32 CurrentFrame;
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