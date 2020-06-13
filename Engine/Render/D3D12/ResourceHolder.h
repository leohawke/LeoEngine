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
		protected:
			ResourceHolder();

			ResourceHolder(const COMPtr<ID3D12Resource>& pResource, D3D12_RESOURCE_STATES in_state = D3D12_RESOURCE_STATE_COMMON);
		protected:
			D3D12_RESOURCE_STATES curr_state;

			COMPtr<ID3D12Resource> resource;

			D3D12_RESOURCE_DESC desc;

			bool bDepthStencil;
		};
	}
}

#endif