/*! \file Engine\Render\D3D12\Context.h
\ingroup Engine
\brief 绘制创建封装。
*/
#ifndef LE_RENDER_D3D12_Context_h
#define LE_RENDER_D3D12_Context_h 1

#include "../IContext.h"
#include "InputLayout.hpp"
#include "Adapter.h"
#include "Display.h"
#include "Texture.h"
#include "ShaderCompose.h"
#include "PipleState.h"
#include "GraphicsBuffer.hpp"
#include "Fence.h"
#include "RayContext.h"
#include "GraphicsPipelineState.h"

#include <LBase/concurrency.h>

#include <unordered_map>

namespace platform::Render::Effect {
	class Effect;
	class CopyEffect;
}

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {
			namespace  Effect = platform::Render::Effect;
			using namespace platform::Render::IFormat;

			class RootSignature;
			class RootSignatureMap;


			class Device final : platform::Render::Device  {
			public:
				Device(DXGI::Adapter& InAdapter);

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

				ShaderCompose* CreateShaderCompose(std::unordered_map<platform::Render::ShaderType, const asset::ShaderBlobAsset*> pShaderBlob, platform::Render::Effect::Effect* pEffect) override;

				//\brief D3D12 Buffer 创建时没有BIND_FLAG
				GraphicsBuffer* CreateBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const*>  init_data = nullptr);

				GraphicsBuffer* CreateConstanBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const *>  init_data = nullptr) override;
				GraphicsBuffer* CreateVertexBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const *>  init_data = nullptr) override;
				GraphicsBuffer* CreateIndexBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const *>  init_data = nullptr) override;

				platform::Render::HardwareShader* CreateShader(const platform::Render::ShaderInitializer& initializer) override;

				leo::observer_ptr<RootSignature> CreateRootSignature(const QuantizedBoundShaderState& QBSS);

				PipleState* CreatePipleState(const platform::Render::PipleState& state) override;

				InputLayout* CreateInputLayout() override;

				GraphicsPipelineState* CreateGraphicsPipelineState(const platform::Render::GraphicsPipelineStateInitializer& initializer) override;

				UnorderedAccessView* CreateUnorderedAccessView(platform::Render::Texture2D* InTexture) override;

				platform::Render::Effect::CopyEffect& BiltEffect();

				platform::Render::InputLayout& PostProcessLayout();

				leo::observer_ptr<ID3D12DescriptorHeap> CreateDynamicCBVSRVUAVDescriptorHeap(uint32_t num);

				leo::observer_ptr<ID3D12PipelineState> CreateRenderPSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC&);

				const Fence& GetRenderFence() const;

				ID3D12Device* GetDevice() const
				{
					return d3d_device.Get();
				}
			public:
				friend class Context;
				friend class RayContext;
				friend class RayDevice;
			
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

				//@{
				//\brief null bind object
				D3D12_CPU_DESCRIPTOR_HANDLE null_srv_handle, null_uav_handle;
				//@}
			private:
				void FillCaps();

				void DeviceEx(ID3D12Device* device, ID3D12CommandQueue* cmd_queue, D3D_FEATURE_LEVEL feature_level);

				void CheckFeatureSupport(ID3D12Device* device);

				struct CmdAllocatorDependencies
				{
					COMPtr<ID3D12CommandAllocator> cmd_allocator;
					std::vector<COMPtr<ID3D12DescriptorHeap>> cbv_srv_uav_heap_cache;
					std::vector<std::pair<COMPtr<ID3D12Resource>, uint32_t>> recycle_after_sync_upload_buffs;
					std::vector<std::pair<COMPtr<ID3D12Resource>, uint32_t>> recycle_after_sync_readback_buffs;
					std::vector<COMPtr<ID3D12Resource>> recycle_after_sync_residency_buffs;
				};
				std::shared_ptr<CmdAllocatorDependencies> CmdAllocatorAlloc();
				void CmdAllocatorRecycle(std::shared_ptr<CmdAllocatorDependencies> const & cmd_allocator, uint64_t fence_val);
			private:
				DXGI::Adapter& adapter;

				//@{
				//\brief base object for swapchain
				COMPtr<ID3D12Device> d3d_device;
				COMPtr<ID3D12CommandQueue> d3d_cmd_queue;
				//@}

				D3D_FEATURE_LEVEL d3d_feature_level;

				//@{
				//\brief object for create object
				
				std::list<std::pair<std::shared_ptr<CmdAllocatorDependencies>, uint64_t>> d3d_render_cmd_allocators;
				std::shared_ptr<CmdAllocatorDependencies> curr_render_cmd_allocator;
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

				//@{
				//\brief cache Device Object for performance
				std::unordered_map<size_t, COMPtr<ID3D12DescriptorHeap>> cbv_srv_uav_heaps;
				//@}

				//@}

				platform::Render::Caps d3d_caps;

				std::unique_ptr<platform::Render::Effect::CopyEffect> bilt_effect;
				std::unique_ptr<InputLayout> postprocess_layout;

				std::unique_ptr<RootSignatureMap> root_signatures;

				std::unordered_map<size_t,COMPtr<ID3D12PipelineState>> graphics_psos;

				std::multimap<uint32_t, COMPtr<ID3D12Resource>> upload_resources;
				std::multimap<uint32_t, COMPtr<ID3D12Resource>> readback_resources;

				array<std::unique_ptr<Fence>, Device::CommandTypeCount> fences;

				//feature support
				D3D12_RESOURCE_BINDING_TIER ResourceBindingTier;
				D3D12_RESOURCE_HEAP_TIER ResourceHeapTier;

				D3D_ROOT_SIGNATURE_VERSION RootSignatureVersion;
			public:
				D3D12_RESOURCE_BINDING_TIER GetResourceBindingTier() const
				{
					return ResourceBindingTier;
				}

				D3D_ROOT_SIGNATURE_VERSION GetRootSignatureVersion() const
				{
					return RootSignatureVersion;
				}
			};

			class Context : public platform::Render::Context {
			public:
				DefGetter(override, Device&, Device, *device);

				void Push(const platform::Render::PipleState&) override;

				void Render(const Effect::Effect& effect, const Effect::Technique& tech, const platform::Render::InputLayout& layout) override;

				void BeginFrame() override;
				void EndFrame() override;

				Display& GetDisplay() override;

				RayContext& GetRayContext() override;
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

				void ExecuteUAVBarrier();

				const COMPtr<ID3D12CommandQueue>& GetCommandQueue(Device::CommandType) const;

				void CommitCommandList(Device::CommandType);
				friend class Device;

				void ClearPSOCache();

				void UpdateRenderPSO(const Effect::Effect& effect, const Effect::Technique& tech,const Effect::Pass& pass, const platform::Render::InputLayout& layout);

				void UpdateCbvSrvUavSamplerHeaps(const ShaderCompose&);

				void RSSetViewports(UINT NumViewports, D3D12_VIEWPORT const *pViewports);

				enum  InnerReourceType{
					Upload,
					Readback
				};

				template<InnerReourceType type>
				COMPtr<ID3D12Resource> InnerResourceAlloc(leo::uint32 size) {
					return InnerResourceAlloc(type, size);
				}

				template<InnerReourceType type>
				void InnerResourceRecycle(COMPtr<ID3D12Resource> resource, leo::uint32 size) {
					return InnerResourceRecycle(type,resource, size);
				}

				void ResidencyResource(COMPtr<ID3D12Resource> resource);
			private:
				void ContextEx(ID3D12Device* device, ID3D12CommandQueue* cmd_queue);

				COMPtr<ID3D12Resource> InnerResourceAlloc(InnerReourceType type, leo::uint32 size);
				void InnerResourceRecycle(InnerReourceType type, COMPtr<ID3D12Resource> resource, leo::uint32 size);

				Fence& GetFence(Device::CommandType);
			private:
				DXGI::AdapterList adapter_list;

				std::shared_ptr<Device> device;
				std::shared_ptr<Display> display;

				std::shared_ptr<RayContext> ray_context;

				array<COMPtr<ID3D12GraphicsCommandList>,Device::CommandTypeCount> d3d_cmd_lists;
				array<std::mutex, Device::CommandTypeCount> cmd_list_mutexs;

				array<COMPtr<ID3D12CommandQueue>, Device::CommandTypeCount-1> d3d_cmd_queues;

				D3D12_VIEWPORT curr_viewport;
			public:
				static Context& Instance();
			};
		}
	}
}

#endif