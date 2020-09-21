/*! \file Engine\Render\D3D12\Context.h
\ingroup Engine
*/
#ifndef LE_RENDER_D3D12_Context_h
#define LE_RENDER_D3D12_Context_h 1

#include "../IContext.h"
#include "Adapter.h"
#include "RayContext.h"
#include "CommandContext.h"
#include <unordered_map>

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {

			class RootSignature;
			class RootSignatureMap;

			class CommandContext;

			class Context : public platform::Render::Context {
			public:
				DefGetter(override, Device&, Device, *device);

				void Render(platform::Render::CommandList& CmdList, const Effect::Effect& effect, const Effect::Technique& tech, const platform::Render::InputLayout& layout) override;

				void BeginFrame() override;
				void EndFrame() override;

				Display& GetDisplay() override;

				RayContext& GetRayContext() override;

				CommandContext* GetDefaultCommandContext() override;

				void AdvanceFrameFence() override;
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

				void CommitCommandList(Device::CommandType);
				friend class Device;

				void ClearPSOCache();

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

				D3D12_VIEWPORT curr_viewport;

				struct
				{
					ID3D12PipelineState* CurrentPipelineStateObject;
					ID3D12RootSignature* CurrentRootSignature;


					ID3D12DescriptorHeap* CurrentSamplerHeap;

					struct
					{
						D3D12_GPU_VIRTUAL_ADDRESS CurrentGPUVirtualAddress[ShaderType::NumStandardType][MAX_CBS];
					} CBVCache;
				} RenderPSO;
			public:
				static Context& Instance();
			};

			Device& GetDevice();
		}
	}
}

#endif