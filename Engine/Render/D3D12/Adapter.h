#pragma once

#include "../IDevice.h"
#include "DXGIAdapter.h"
#include "Texture.h"
#include "ShaderCompose.h"
#include "GraphicsBuffer.hpp"
#include "InputLayout.hpp"
#include "GraphicsPipelineState.h"
#include "Display.h"
#include "PipleState.h"
#include "Fence.h"
#include "RootSignature.h"
#include "D3DCommandList.h"

namespace platform::Render::Effect {
	class Effect;
	class CopyEffect;
}

namespace platform_ex::Windows::D3D12 {
	class NodeDevice;

	namespace  Effect = platform::Render::Effect;
	using namespace platform::Render::IFormat;
	using platform::Render::SampleDesc;
	using platform::Render::ElementInitData;

	// Represents a set of linked D3D12 device nodes (LDA i.e 1 or more identical GPUs). In most cases there will be only 1 node, however if the system supports
	// SLI/Crossfire and the app enables it an Adapter will have 2 or more nodes. This class will own anything that can be shared
	// across LDA including: System Pool Memory,.Pipeline State Objects, Root Signatures etc.
	class Device final : platform::Render::Device {
	public:
		Device(DXGI::Adapter& InAdapter);

		ID3D12Device* operator->() lnoexcept;

		Texture1D* CreateTexture(uint16 width, uint8 num_mipmaps, uint8 array_size,
			EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const*>  init_data = nullptr) override;

		Texture2D* CreateTexture(uint16 width, uint16 height, uint8 num_mipmaps, uint8 array_size,
			EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const*>  init_data = nullptr) override;

		Texture3D* CreateTexture(const platform::Render::Texture3DInitializer& Initializer, std::optional<ElementInitData const*>  init_data = nullptr) override;

		TextureCube* CreateTextureCube(uint16 size, uint8 num_mipmaps, uint8 array_size,
			EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const*>  init_data = nullptr) override;

		platform::Render::Caps& GetCaps() override;

		ShaderCompose* CreateShaderCompose(std::unordered_map<platform::Render::ShaderType, const asset::ShaderBlobAsset*> pShaderBlob, platform::Render::Effect::Effect* pEffect) override;

		//\brief D3D12 Buffer 创建时没有BIND_FLAG
		GraphicsBuffer* CreateBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const*>  init_data = nullptr);

		GraphicsBuffer* CreateConstanBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const*>  init_data = nullptr) override;
		GraphicsBuffer* CreateVertexBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const*>  init_data = nullptr) override;
		GraphicsBuffer* CreateIndexBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<void const*>  init_data = nullptr) override;

		platform::Render::HardwareShader* CreateShader(const platform::Render::ShaderInitializer& initializer) override;

		leo::observer_ptr<RootSignature> CreateRootSignature(const QuantizedBoundShaderState& QBSS);

		PipleState* CreatePipleState(const platform::Render::PipleState& state) override;

		InputLayout* CreateInputLayout() override;

		GraphicsPipelineState* CreateGraphicsPipelineState(const platform::Render::GraphicsPipelineStateInitializer& initializer) override;

		ComputePipelineState* CreateComputePipelineState(platform::Render::ComputeHWShader* ComputeShader);

		UnorderedAccessView* CreateUnorderedAccessView(platform::Render::Texture2D* InTexture) override;

		platform::Render::Effect::CopyEffect& BiltEffect();

		platform::Render::InputLayout& PostProcessLayout();

		leo::observer_ptr<ID3D12DescriptorHeap> CreateDynamicCBVSRVUAVDescriptorHeap(uint32_t num);

		leo::observer_ptr<ID3D12PipelineState> CreateRenderPSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC&);


		ID3D12Device* GetDevice() const
		{
			return d3d_device.Get();
		}

		ID3D12Device5* GetRayTracingDevice()
		{
			return raytracing_device.Get();
		}

		NodeDevice* GetNodeDevice(uint32 Index) const
		{
			return Devices[Index];
		}

		void InitializeRayTracing();

		ManualFence& GetFrameFence() { return *frame_fence; }
	public:
		friend class Context;
		friend class RayContext;
		friend class RayDevice;

		enum CommandType {
			Command_Resource = 1,
			CommandTypeCount
		};

		//@{
		//\brief 使用者可以修改这些值满足特定需求
		lconstexpr static UINT const NUM_MAX_RENDER_TARGET_VIEWS = 1024 + Display::NUM_BACK_BUFFERS;
		lconstexpr static UINT const NUM_MAX_DEPTH_STENCIL_VIEWS = 128;
		lconstexpr static UINT const NUM_MAX_CBV_SRV_UAVS = 4 * 1024;

		//@}
	private:
		void FillCaps();

		void DeviceEx(ID3D12Device* device, ID3D12CommandQueue* cmd_queue, D3D_FEATURE_LEVEL feature_level);


		void CheckFeatureSupport(ID3D12Device* device);

		struct ResidencyPoolEntry
		{
			CLSyncPoint SyncPoint;
			std::vector<COMPtr<ID3D12Resource>> recycle_after_sync_residency_buffs;
			std::vector<std::pair<COMPtr<ID3D12Resource>, uint32_t>> recycle_after_sync_upload_buffs;
			std::vector<std::pair<COMPtr<ID3D12Resource>, uint32_t>> recycle_after_sync_readback_buffs;
		};

		ResidencyPoolEntry ResidencyPool;
		std::queue< ResidencyPoolEntry> ReclaimPool;
	private:
		DXGI::Adapter& adapter;

		//@{
		//\brief base object for swapchain
		COMPtr<ID3D12Device> d3d_device;
		COMPtr<ID3D12Device5> raytracing_device;
		//@}

		COMPtr<ID3D12DebugDevice> d3d_debug_device;

		D3D_FEATURE_LEVEL d3d_feature_level;

		//@{
		//\brief object for create object
		array<COMPtr<ID3D12CommandAllocator>, CommandTypeCount> d3d_cmd_allocators;
		//COMPtr<ID3D12CommandQueue> d3d_cmd_compute_quque;
		//COMPtr<ID3D12CommandQueue> d3d_cmd_copy_quque;

		array<COMPtr<ID3D12DescriptorHeap>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> d3d_desc_heaps;
		array<UINT, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> d3d_desc_incres_sizes;

		//@{
		//\brief cache Device Object for performance
		std::unordered_map<size_t, COMPtr<ID3D12DescriptorHeap>> cbv_srv_uav_heaps;
		//@}

		//@}

		platform::Render::Caps d3d_caps;

		std::unique_ptr<platform::Render::Effect::CopyEffect> bilt_effect;
		std::unique_ptr<InputLayout> postprocess_layout;

		std::unique_ptr<RootSignatureMap> root_signatures;

		std::unordered_map<size_t, COMPtr<ID3D12PipelineState>> graphics_psos;

		std::multimap<uint32_t, COMPtr<ID3D12Resource>> upload_resources;
		std::multimap<uint32_t, COMPtr<ID3D12Resource>> readback_resources;

		array<std::unique_ptr<Fence>, Device::CommandTypeCount> fences;

		ManualFence* frame_fence;

		//feature support
		D3D12_RESOURCE_BINDING_TIER ResourceBindingTier;
		D3D12_RESOURCE_HEAP_TIER ResourceHeapTier;

		D3D_ROOT_SIGNATURE_VERSION RootSignatureVersion;

		NodeDevice* Devices[1];

		D3DPipelineStateCache PipelineStateCache;
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


}
