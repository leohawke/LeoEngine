/*! \file Engine\Render\D3D12\RenderView.h
\ingroup Engine
\brief 视图逻辑相关封装。
*/

#ifndef LE_RENDER_D3D12_RenderView_h
#define LE_RENDER_D3D12_RenderView_h 1

#include "d3d12_dxgi.h"
#include "..\IRenderView.h"
#include "..\ITexture.hpp"
#include <LBase/linttype.hpp>

namespace platform_ex {
	namespace Windows {

		namespace D3D12 {
			class Device;
			class GraphicsBuffer;
			class Texture;

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

			class GPUDataStructView {
			public:
				GPUDataStructView(ID3D12Resource* res,ViewSimulation* view, uint16 first_subres, uint16 num_subres);

				ViewSimulation* View();


				uint16 FirstSubResIndex();
				uint16 SubResNum();

				ID3D12Resource* Resource();
			protected:
				ViewSimulation* view;

				uint16 first_subres;
				uint16 num_subres;

				observer_ptr<ID3D12Resource> res;
			};

			class RenderTargetView :public GPUDataStructView,public platform::Render::RenderTargetView {
			public:
				RenderTargetView(Texture2D& texture_2d, uint8 first_array_index, uint8 array_size, uint8 level);
				RenderTargetView(Texture3D& texture_3d, uint8 array_index, uint8 first_slice, uint8 num_slices, uint8 level);
				RenderTargetView(TextureCube& texture_cube, uint8 array_index, platform::Render::TextureCubeFaces face, uint8 level);
				RenderTargetView(GraphicsBuffer& gb, uint8 width, uint8 height, platform::Render::EFormat pf);
			};

			class DepthStencilView :public GPUDataStructView,public platform::Render::DepthStencilView {
			public:
				DepthStencilView(Texture2D& texture_1d_2d_cube, uint8 first_array_index, uint8 array_size, uint8 level);
				DepthStencilView(Texture3D& texture_3d, uint8 array_index, uint8 first_slice, uint8 num_slices, uint8 level);
				DepthStencilView(TextureCube& texture_cube, uint8 array_index, platform::Render::TextureCubeFaces face, uint8 level);
				DepthStencilView(uint16 width, uint16 height, platform::Render::EFormat pf, uint8 sample_count, uint8 sample_quality);
			};

			class UnorderedAccessView : public GPUDataStructView,public platform::Render::UnorderedAccessView {
			public:
				UnorderedAccessView(Texture2D& texture_1d_2d_cube, uint8 first_array_index, uint8 array_size, uint8 level);
				UnorderedAccessView(Texture3D& texture_3d, uint8 array_index, uint8 first_slice, uint8 num_slices, uint8 level);
				UnorderedAccessView(TextureCube& texture_cube, uint8 array_index, platform::Render::TextureCubeFaces face, uint8 level);
				UnorderedAccessView(GraphicsBuffer& gb, platform::Render::EFormat pf);


				void ResetInitCount();
			};
		}
	}
}

#endif