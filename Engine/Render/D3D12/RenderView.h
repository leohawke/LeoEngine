/*! \file Engine\Render\D3D12\RenderView.h
\ingroup Engine
\brief 视图逻辑相关封装。
*/

#ifndef LE_RENDER_D3D12_RenderView_h
#define LE_RENDER_D3D12_RenderView_h 1

#include "d3d12_dxgi.h"
#include "..\IRenderView.h"

namespace platform_ex {
	namespace Windows {

		namespace D3D12 {
			class Device;

			class ViewSimulation {
			public:
				ViewSimulation(COMPtr<ID3D12Resource> & Res, D3D12_DESCRIPTOR_HEAP_TYPE Type);
				ViewSimulation(COMPtr<ID3D12Resource> & resource, D3D12_SHADER_RESOURCE_VIEW_DESC const & desc);
				ViewSimulation(COMPtr<ID3D12Resource> & resource, D3D12_RENDER_TARGET_VIEW_DESC const & desc);
				ViewSimulation(COMPtr<ID3D12Resource> & resource, D3D12_DEPTH_STENCIL_VIEW_DESC const & desc);

				ViewSimulation(COMPtr<ID3D12Resource> & resource, D3D12_UNORDERED_ACCESS_VIEW_DESC const & desc);
				
				~ViewSimulation();

				DefGetter(const, D3D12_CPU_DESCRIPTOR_HANDLE const &, Handle, handle);

			private:
				COMPtr<ID3D12Resource> res;
				D3D12_DESCRIPTOR_HEAP_TYPE type;
				D3D12_CPU_DESCRIPTOR_HANDLE handle;

				Device& device;
			};

			class GPUDataStructView {};

			class RenderTargetView :public GPUDataStructView,public platform::Render::RenderTargetView {
			public:

			
			};

			class DepthStencilView :public GPUDataStructView,public platform::Render::DepthStencilView {};

			class UnorderedAccessView : public GPUDataStructView,public platform::Render::UnorderedAccessView {};
		}
	}
}

#endif