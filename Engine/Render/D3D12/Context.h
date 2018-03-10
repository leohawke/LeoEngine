/*! \file Engine\Render\D3D12\Context.h
\ingroup Engine
\brief 绘制创建封装。
*/
#ifndef LE_RENDER_D3D12_Context_h
#define LE_RENDER_D3D12_Context_h 1

#include "../Effect/CopyEffect.h"
#include "../IContext.h"
#include "InputLayout.hpp"
#include "Adapter.h"
#include "Display.h"
#include "Texture.h"
#include "ShaderCompose.h"
#include "PipleState.h"
#include "GraphicsBuffer.hpp"
#include "Fence.h"

#include <UniversalDXSDK/d3d12.h>

#include <LBase/concurrency.h>

#include <unordered_map>

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {
			namespace  Effect = platform::Render::Effect;
			using namespace platform::Render::IFormat;

			class Device final : platform::Render::Device  {
			public:
				Device(DXGI::Adapter& adapter);

				D3D12_CPU_DESCRIPTOR_HANDLE AllocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type);
				void DeallocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
				
				ID3D12Device* operator->() lnoexcept;

				Texture1D* CreateTexture(uint16 width, uint8 num_mipmaps, uint8 array_size,
					EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data = nullptr) override;

				Texture2D* CreateTexture(uint16 width,uint16 height,uint8 num_mipmaps, uint8 array_size,
					EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data = nullptr) override;
				
				Texture3D* CreateTexture(uint16 width, uint16 height,uint16 depth, uint8 num_mipmaps, uint8 array_size,
					EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data = nullptr) override;
				
				TextureCube* CreateTextureCube(uint16 size, uint8 num_mipmaps, uint8 array_size,
					EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data = nullptr) override;

				platform::Render::Caps& GetCaps() override;

				ShaderCompose* CreateShaderCompose(std::unordered_map<ShaderCompose::Type, leo::observer_ptr<const asset::ShaderBlobAsset>> pShaderBlob, leo::observer_ptr<platform::Render::Effect::Effect> pEffect) override;

				//\brief D3D12 Buffer 创建时没有BIND_FLAG
				GraphicsBuffer* CreateBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const*>  init_data = nullptr);

				GraphicsBuffer* CreateConstanBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const *>  init_data = nullptr) override;
				GraphicsBuffer* CreateVertexBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const *>  init_data = nullptr) override;
				GraphicsBuffer* CreateIndexBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const *>  init_data = nullptr) override;

				leo::observer_ptr<ID3D12RootSignature> CreateRootSignature(std::array<size_t, ShaderCompose::NumTypes * 4> num, bool vertex_shader, bool stream_output);

				PipleState* CreatePipleState(const platform::Render::PipleState& state) override;

				InputLayout* CreateInputLayout() override;

				platform::Render::Effect::CopyEffect& BiltEffect();

				platform::Render::InputLayout& PostProcessLayout();

				leo::observer_ptr<ID3D12DescriptorHeap> CreateDynamicCBVSRVUAVDescriptorHeap(uint32_t num);

				leo::observer_ptr<ID3D12PipelineState> CreateRenderPSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC&);
			public:
				friend class Context;
			
				enum CommandType {
					Command_Render = 0,
					//Command_Compute,
					//Command_Copy,
					Command_Resource,
					CommandTypeCount
				};

				//@{
				//\brief 使用者可以修改这些值满足特定需求
				lconstexpr static UINT const NUM_MAX_RENDER_TARGET_VIEWS = 1024+Display::NUM_BACK_BUFFERS;
				lconstexpr static UINT const NUM_MAX_DEPTH_STENCIL_VIEWS = 128;
				lconstexpr static UINT const NUM_MAX_CBV_SRV_UAVS = 4 * 1024;

				//@}
				D3D12_CPU_DESCRIPTOR_HANDLE null_srv_handle,null_uav_handle;

			private:
				void FillCaps();

				void DeviceEx(ID3D12Device* device, ID3D12CommandQueue* cmd_queue, D3D_FEATURE_LEVEL feature_level);
			private:
				//@{
				//\brief base object for swapchain
				COMPtr<ID3D12Device> d3d_device;
				COMPtr<ID3D12CommandQueue> d3d_cmd_queue;
				//@}

				D3D_FEATURE_LEVEL d3d_feature_level;

				//@{
				//\brief object for create object
				

				array<COMPtr<ID3D12CommandAllocator>, CommandTypeCount> d3d_cmd_allocators;
				//COMPtr<ID3D12CommandQueue> d3d_cmd_compute_quque;
				//COMPtr<ID3D12CommandQueue> d3d_cmd_copy_quque;

				array<COMPtr<ID3D12DescriptorHeap>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> d3d_desc_heaps;
				array<UINT, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> d3d_desc_incres_sizes;
				//@{
				//\brief host'memory so large can enough to store many bool
				array<bool, NUM_MAX_RENDER_TARGET_VIEWS> rtv_heap_flag;
				array<bool, NUM_MAX_DEPTH_STENCIL_VIEWS> dsv_heap_flag;
				array<bool, NUM_MAX_CBV_SRV_UAVS> cbv_srv_uav_heap_flag;
				//@}

				//@}

				platform::Render::Caps d3d_caps;

				std::unique_ptr<platform::Render::Effect::CopyEffect> bilt_effect;
				std::unique_ptr<InputLayout> postprocess_layout;

				std::unordered_map<size_t, COMPtr<ID3D12RootSignature>> root_signatures;

				std::vector<COMPtr<ID3D12DescriptorHeap>> cbv_srv_uav_heap_cache;

				std::unordered_map<size_t,COMPtr<ID3D12PipelineState>> graphics_psos;
			};

			class Context : public platform::Render::Context {
			public:
				DefGetter(override, Device&, Device, *device);

				void Push(const platform::Render::PipleState&) override;

				void Render(const Effect::Effect& effect, const Effect::Technique& tech, const platform::Render::InputLayout& layout) override;

				void BeginFrame() override;
				void EndFrame() override;
			public:
				void CreateDeviceAndDisplay() override;
			private:
				Context();

				void DoBindFrameBuffer(const std::shared_ptr<platform::Render::FrameBuffer>&) override;
			public:

				DXGI::Adapter& DefaultAdapter();

				DefGetter(const lnothrow, IDXGIFactory4*, DXGIFactory4, adapter_list.GetDXGIFactory4());

				void SyncCPUGPU(bool force = true);

				const COMPtr<ID3D12GraphicsCommandList>& GetCommandList(Device::CommandType) const;
				std::mutex& GetCommandListMutex(Device::CommandType);

				void SyncCommand(Device::CommandType);
				void ResetCommand(Device::CommandType);

				const COMPtr<ID3D12CommandQueue>& GetCommandQueue(Device::CommandType) const;

				void CommitCommandList(Device::CommandType);
				friend class Device;

				void ClearPSOCache();

				void UpdateRenderPSO(const Effect::Effect& effect, const Effect::Technique& tech,const Effect::Pass& pass, const platform::Render::InputLayout& layout);

			private:
				void ContextEx(ID3D12Device* device, ID3D12CommandQueue* cmd_queue);
			private:
				array<std::unique_ptr<Fence>, Device::CommandTypeCount> fences;
				DXGI::AdapterList adapter_list;

				shared_ptr<Device> device;
				shared_ptr<Display> display;

				array<COMPtr<ID3D12GraphicsCommandList>,Device::CommandTypeCount> d3d_cmd_lists;
				array<std::mutex, Device::CommandTypeCount> cmd_list_mutexs;

				array<COMPtr<ID3D12CommandQueue>, Device::CommandTypeCount-1> d3d_cmd_queues;

			public:
				static Context& Instance();
			};
		}
	}
}

#endif