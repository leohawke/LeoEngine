#pragma once

#include "Common.h"
#include "d3d12_dxgi.h"
#include "DescriptorCache.h"
#include <vector>

namespace platform_ex::Windows::D3D12
{
	class CommandContext;

	class NodeDevice : public SingleNodeGPUObject,public AdapterChild
	{
	public:
		NodeDevice(GPUMaskType InGpuMask,D3D12Adapter* InAdapter);

		void Initialize();

		void CreateCommandContexts();

		ID3D12Device* GetDevice();


		CommandContext& GetDefaultCommandContext()
		{
			return *CommandContextArray[0];
		}
	private:
		void SetupAfterDeviceCreation();
	private:

		std::vector<CommandContext*> CommandContextArray;

		GlobalOnlineHeap GlobalSamplerHeap;
		GlobalOnlineHeap GlobalViewHeap;
	};

}