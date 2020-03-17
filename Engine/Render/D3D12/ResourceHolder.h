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

			void SetName(const char* name);
		protected:
			ResourceHolder();
			ResourceHolder(const COMPtr<ID3D12Resource>& pResource);
		protected:
			D3D12_RESOURCE_STATES curr_state;

			COMPtr<ID3D12Resource> resource;
		};
	}
}

#endif