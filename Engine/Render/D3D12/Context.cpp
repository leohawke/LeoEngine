#include <LBase/pointer.hpp>
#include "Context.h"
#include "Display.h"

#define TEST_CODE 1
#if TEST_CODE
extern HWND g_hwnd;
#endif

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {
			Device::Device(DXGI::Adapter & adapter)
			{
				std::vector<D3D_FEATURE_LEVEL> feature_levels = {
					D3D_FEATURE_LEVEL_12_1 ,
					D3D_FEATURE_LEVEL_12_0 ,
					D3D_FEATURE_LEVEL_11_1 ,
					D3D_FEATURE_LEVEL_11_0 };

				for (auto level : feature_levels) {
					ID3D12Device* device = nullptr;
					if (SUCCEEDED(D3D12::CreateDevice(adapter.Get(),
						level, IID_ID3D12Device, reinterpret_cast<void**>(&device)))) {

						D3D12_COMMAND_QUEUE_DESC queue_desc =
						{
							D3D12_COMMAND_LIST_TYPE_DIRECT, //Type
							0, //Priority
							D3D12_COMMAND_QUEUE_FLAG_NONE, //Flags
							0 //NodeMask
						};

						ID3D12CommandQueue* cmd_queue;
						CheckHResult(device->CreateCommandQueue(&queue_desc,
							IID_ID3D12CommandQueue, reinterpret_cast<void**>(&cmd_queue)));

						D3D12_FEATURE_DATA_FEATURE_LEVELS req_feature_levels;
						req_feature_levels.NumFeatureLevels = static_cast<UINT>(feature_levels.size());
						req_feature_levels.pFeatureLevelsRequested = &feature_levels[0];
						device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &req_feature_levels, sizeof(req_feature_levels));

						DeviceEx(device, cmd_queue, req_feature_levels.MaxSupportedFeatureLevel);

						auto desc = adapter.Description();
						char const * feature_level_str;
						switch (req_feature_levels.MaxSupportedFeatureLevel)
						{
						case D3D_FEATURE_LEVEL_12_1:
							feature_level_str = " D3D_FEATURE_LEVEL_12_1";
							break;

						case D3D_FEATURE_LEVEL_12_0:
							feature_level_str = " D3D_FEATURE_LEVEL_12_0";
							break;

						case D3D_FEATURE_LEVEL_11_1:
							feature_level_str = " D3D_FEATURE_LEVEL_11_0";
							break;

						case D3D_FEATURE_LEVEL_11_0:
							feature_level_str = " D3D_FEATURE_LEVEL_11_1";
							break;

						default:
							feature_level_str = " D3D_FEATURE_LEVEL_UN_0";
							break;
						}
						Trace(Notice, "%s Adapter Description:%s\n", lfname, desc.c_str());
						
						//todo if something

						break;
					}
				}
				
			}

			D3D12_CPU_DESCRIPTOR_HANDLE Device::AllocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type)
			{
				leo::pointer_iterator<bool> desc_heap_flag_iter = nullptr;
				size_t desc_heap_flag_num = 0;
				size_t desc_heap_offset = 0;
				switch (Type) {
				case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
					desc_heap_flag_iter = rtv_heap_flag.data();
					desc_heap_flag_num = rtv_heap_flag.size();
					desc_heap_offset = Display::NUM_BACK_BUFFERS;
					break;
				case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
					desc_heap_flag_iter = dsv_heap_flag.data();
					desc_heap_flag_num = dsv_heap_flag.size();
					desc_heap_offset = 2;
					break;
				case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
					desc_heap_flag_iter = cbv_srv_uav_heap_flag.data();
					desc_heap_flag_num = cbv_srv_uav_heap_flag.size();
					break;
				}
				for (auto i = 0; i != desc_heap_flag_num; ++i) {
					if (!*(desc_heap_flag_iter + i)) {
						*(desc_heap_flag_iter + i) = true;
						auto handle = GetCPUDescriptorHandleForHeapStart(d3d_desc_heaps[Type].Get());
						handle.ptr += ((i+ desc_heap_offset) * d3d_desc_incres_sizes[Type]);
						return handle;
					}
				}
				LAssert(false, "Not Enough Space");
				throw;
			}

			void Device::DeallocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
			{
				leo::pointer_iterator<bool> desc_heap_flag_iter = nullptr;
				size_t desc_heap_flag_num = 0;
				size_t desc_heap_offset = 0;
				switch (Type) {
				case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
					desc_heap_flag_iter = rtv_heap_flag.data();
					desc_heap_flag_num = rtv_heap_flag.size();
					desc_heap_offset = Display::NUM_BACK_BUFFERS;
					break;
				case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
					desc_heap_flag_iter = dsv_heap_flag.data();
					desc_heap_flag_num = dsv_heap_flag.size();
					desc_heap_offset = 2;
					break;
				case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
					desc_heap_flag_iter = cbv_srv_uav_heap_flag.data();
					desc_heap_flag_num = cbv_srv_uav_heap_flag.size();
					break;
				}
				auto Offset = Handle.ptr - GetCPUDescriptorHandleForHeapStart(d3d_desc_heaps[Type].Get()).ptr;
				auto index = Offset / d3d_desc_incres_sizes[Type];
				if(index >= desc_heap_offset)
					*(desc_heap_flag_iter + index- desc_heap_offset) = false;
			}

			ID3D12Device*  Device::operator->() lnoexcept {
				return d3d_device.Get();
			}

			void Device::DeviceEx(ID3D12Device * device, ID3D12CommandQueue * cmd_queue, D3D_FEATURE_LEVEL feature_level)
			{
				d3d_device = device;
				d3d_cmd_queue = cmd_queue;
				d3d_feature_level = feature_level;

				CheckHResult(d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
					COMPtr_RefParam(d3d_cmd_allocators[Command_Render],IID_ID3D12CommandAllocator)));

				auto create_desc_heap=[&](D3D12_DESCRIPTOR_HEAP_TYPE Type,UINT NumDescriptors,
					D3D12_DESCRIPTOR_HEAP_FLAGS Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,UINT NodeMask = 0)
				{
					D3D12_DESCRIPTOR_HEAP_DESC descriptor_desc = {
						Type,
						NumDescriptors,
						Flags,
						NodeMask
					};
					CheckHResult(d3d_device->CreateDescriptorHeap(&descriptor_desc,
						COMPtr_RefParam(d3d_desc_heaps[Type],IID_ID3D12DescriptorHeap)));
					d3d_desc_incres_sizes[Type] = d3d_device->GetDescriptorHandleIncrementSize(Type);
				};

				create_desc_heap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NUM_MAX_RENDER_TARGET_VIEWS);
				create_desc_heap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, NUM_MAX_DEPTH_STENCIL_VIEWS);
				create_desc_heap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NUM_MAX_CBV_SRV_UAVS);

				rtv_heap_flag.fill(false);
				dsv_heap_flag.fill(false);
				cbv_srv_uav_heap_flag.fill(false);

				D3D12_SHADER_RESOURCE_VIEW_DESC null_srv_desc;
				null_srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				null_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				null_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				null_srv_desc.Texture2D.MipLevels = 1;
				null_srv_desc.Texture2D.MostDetailedMip = 0;
				null_srv_desc.Texture2D.PlaneSlice = 0;
				null_srv_desc.Texture2D.ResourceMinLODClamp = 0;
				null_srv_handle = AllocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				d3d_device->CreateShaderResourceView(nullptr, &null_srv_desc, null_srv_handle);

				D3D12_UNORDERED_ACCESS_VIEW_DESC null_uav_desc;
				null_uav_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				null_uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				null_uav_desc.Texture2D.MipSlice = 0;
				null_uav_desc.Texture2D.PlaneSlice = 0;
				null_uav_handle = AllocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				d3d_device->CreateUnorderedAccessView(nullptr, nullptr, &null_uav_desc, null_uav_handle);
			}

			Context::Context()
				:adapter_list()
			{
#ifndef NDEBUG
				{
					COMPtr<ID3D12Debug> debug_ctrl;
					if (SUCCEEDED(D3D12::GetDebugInterface(COMPtr_RefParam(debug_ctrl,IID_ID3D12Debug)))) {
						LAssertNonnull(debug_ctrl);
						debug_ctrl->EnableDebugLayer();
					}
				}
#endif
#if TEST_CODE
				CreateDeviceAndDisplay();
#endif
			}

			DXGI::Adapter & Context::DefaultAdapter()
			{
				return adapter_list.CurrentAdapter();
			}

			void Context::ContextEx(ID3D12Device * d3d_device, ID3D12CommandQueue * cmd_queue)
			{
				CheckHResult(d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
					device->d3d_cmd_allocators[Device::Command_Render].Get(), nullptr,
					COMPtr_RefParam(d3d_cmd_lists[Device::Command_Render], IID_ID3D12GraphicsCommandList)));
			}

			void Context::CreateDeviceAndDisplay() {
				device = leo::make_shared<Device>(DefaultAdapter());
				ContextEx(device->d3d_device.Get(), device->d3d_cmd_queue.Get());
				DisplaySetting setting;
				display = leo::make_shared<Display>(GetDXGIFactory4(),device->d3d_cmd_queue.Get(),setting,g_hwnd);//test code
			}
			Context & Context::Instance()
			{
				static Context context;
				return context;
			}
		}
	}
}