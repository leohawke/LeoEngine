/*! \file Engine\Render\D3D12\Resource.h
\ingroup Engine
\brief ×ÊÔ´ÃèÊö¡£
*/

#ifndef LE_RENDER_D3D12_Resource_h
#define LE_RENDER_D3D12_Resource_h 1

#include "d3d12_dxgi.h"


namespace platform_ex::Windows {
	namespace D3D12 {
		class ResourceHolder {
		public:
			virtual ~ResourceHolder();

			bool UpdateResourceBarrier(D3D12_RESOURCE_BARRIER& barrier, D3D12_RESOURCE_STATES target_state);

			ID3D12Resource* Resource() const {
				return resource.Get();
			}

			D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
				return resource->GetGPUVirtualAddress();
			}

			void SetName(const char* name);

			D3D12_RESOURCE_DESC GetDesc() const {
				return desc;
			}

			bool IsDepthStencilResource() const { return bDepthStencil; }

			//TODO
			inline uint64 GetOffsetFromBaseOfResource() const { return 0; }

			bool IsTransitionNeeded(D3D12_RESOURCE_STATES target_state)
			{
				return target_state != curr_state;
			}

			D3D12_RESOURCE_STATES GetResourceState() const
			{
				return curr_state;
			}

			void SetResourceState(D3D12_RESOURCE_STATES state)
			{
				curr_state = state;
			}
		protected:
			ResourceHolder();

			ResourceHolder(const COMPtr<ID3D12Resource>& pResource, D3D12_RESOURCE_STATES in_state = D3D12_RESOURCE_STATE_COMMON);
		protected:
			D3D12_RESOURCE_STATES curr_state;

			COMPtr<ID3D12Resource> resource;

			D3D12_RESOURCE_DESC desc;

			bool bDepthStencil;
		};

		class FastClearResource
		{
		public:
			inline void GetWriteMaskProperties(void*& OutData, uint32& OutSize)
			{
				OutData = nullptr;
				OutSize = 0;
			}
		};

		class ResourceBarrierBatcher 
		{
		public:
			explicit ResourceBarrierBatcher()
			{};

			// Add a UAV barrier to the batch. Ignoring the actual resource for now.
			void AddUAV()
			{
				Barriers.resize(Barriers.size()+1);
				D3D12_RESOURCE_BARRIER& Barrier = Barriers.back();
				Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				Barrier.UAV.pResource = nullptr;	// Ignore the resource ptr for now. HW doesn't do anything with it.
			}

			// Add a transition resource barrier to the batch. Returns the number of barriers added, which may be negative if an existing barrier was cancelled.
			int32 AddTransition(ID3D12Resource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After, uint32 Subresource)
			{
				lconstraint(Before != After);

				if (!Barriers.empty())
				{
					// Check if we are simply reverting the last transition. In that case, we can just remove both transitions.
					// This happens fairly frequently due to resource pooling since different RHI buffers can point to the same underlying D3D buffer.
					// Instead of ping-ponging that underlying resource between COPY_DEST and GENERIC_READ, several copies can happen without a ResourceBarrier() in between.
					// Doing this check also eliminates a D3D debug layer warning about multiple transitions of the same subresource.
					const D3D12_RESOURCE_BARRIER& Last = Barriers.back();
					if (pResource == Last.Transition.pResource &&
						Subresource == Last.Transition.Subresource &&
						Before == Last.Transition.StateAfter &&
						After == Last.Transition.StateBefore &&
						Last.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
					{
						Barriers.pop_back();
						return -1;
					}
				}

				Barriers.resize(Barriers.size() + 1);
				D3D12_RESOURCE_BARRIER& Barrier = Barriers.back();
				Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				Barrier.Transition.StateBefore = Before;
				Barrier.Transition.StateAfter = After;
				Barrier.Transition.Subresource = Subresource;
				Barrier.Transition.pResource = pResource;
				return 1;
			}

			void AddAliasingBarrier(ID3D12Resource* pResource)
			{
				Barriers.resize(Barriers.size() + 1);
				D3D12_RESOURCE_BARRIER& Barrier = Barriers.back();
				Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
				Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				Barrier.Aliasing.pResourceBefore = NULL;
				Barrier.Aliasing.pResourceAfter = pResource;
			}

			// Flush the batch to the specified command list then reset.
			void Flush(ID3D12GraphicsCommandList* pCommandList)
			{
				if (!Barriers.empty())
				{
					lconstraint(pCommandList);
					{
						pCommandList->ResourceBarrier(static_cast<UINT>(Barriers.size()), Barriers.data());
					}
					Reset();
				}
			}

			// Clears the batch.
			void Reset()
			{
				Barriers.resize(0);	// Reset the array without shrinking (Does not destruct items, does not de-allocate memory).
			}

			const std::vector<D3D12_RESOURCE_BARRIER>& GetBarriers() const
			{
				return Barriers;
			}

		private:
			std::vector<D3D12_RESOURCE_BARRIER> Barriers;
		};

	}
}

#endif