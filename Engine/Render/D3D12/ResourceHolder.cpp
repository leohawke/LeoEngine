#include "ResourceHolder.h"

namespace platform_ex::Windows {
	namespace D3D12 {
		ImplDeDtor(ResourceHolder)

		bool ResourceHolder::UpdateResourceBarrier(D3D12_RESOURCE_BARRIER & barrier, D3D12_RESOURCE_STATES target_state)
		{
			if (curr_state == target_state)
				return false;
			else {
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

				barrier.Transition.pResource = resource.Get();

				barrier.Transition.StateBefore = curr_state;
				barrier.Transition.StateAfter = target_state;
				curr_state = target_state;
				return true;
			}
		}

		void Windows::D3D12::ResourceHolder::SetName(const char* name)
		{
			D3D::Debug(resource, name);
		}

		ResourceHolder::ResourceHolder()
			:curr_state(D3D12_RESOURCE_STATE_COMMON)
		{
		}

		ResourceHolder::ResourceHolder(const COMPtr<ID3D12Resource>& pResource, D3D12_RESOURCE_STATES in_state)
			: curr_state(in_state), resource(pResource){
			lconstraint(resource);
			desc = pResource->GetDesc();

			bDepthStencil = (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0;
		}
	}
}
