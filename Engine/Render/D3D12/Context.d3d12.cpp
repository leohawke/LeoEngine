#include <LBase/pointer.hpp>
#include "Context.h"
#include "Display.h"
#include "Texture.h"

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
						auto handle = d3d_desc_heaps[Type]->GetCPUDescriptorHandleForHeapStart();
						handle.ptr += ((i + desc_heap_offset) * d3d_desc_incres_sizes[Type]);
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
				auto Offset = Handle.ptr - d3d_desc_heaps[Type]->GetCPUDescriptorHandleForHeapStart().ptr;
				auto index = Offset / d3d_desc_incres_sizes[Type];
				if (index >= desc_heap_offset)
					*(desc_heap_flag_iter + index - desc_heap_offset) = false;
			}

			Texture1D* Device::CreateTexture(uint16 width, uint8 num_mipmaps, uint8 array_size, EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data)
			{
				auto texture = std::make_unique<Texture1D>(width, num_mipmaps, array_size, format,access,sample_info);
				if (init_data.has_value())
					texture->HWResourceCreate(init_data.value());
				return texture.release();
			}

			Texture2D* Device::CreateTexture(uint16 width, uint16 height, uint8 num_mipmaps, uint8 array_size, EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data)
			{
				auto texture = std::make_unique<Texture2D>(width,height, num_mipmaps, array_size, format, access, sample_info);
				if (init_data.has_value())
					texture->HWResourceCreate(init_data.value());
				return texture.release();
			}

			Texture3D* Device::CreateTexture(uint16 width, uint16 height, uint16 depth, uint8 num_mipmaps, uint8 array_size, EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data)
			{
				auto texture = std::make_unique<Texture3D>(width,height,depth, num_mipmaps, array_size, format, access, sample_info);
				if (init_data.has_value())
					texture->HWResourceCreate(init_data.value());
				return texture.release();
			}

			TextureCube* Device::CreateTextureCube(uint16 size, uint8 num_mipmaps, uint8 array_size, EFormat format, uint32 access, SampleDesc sample_info, std::optional<ElementInitData const *>  init_data)
			{
				auto texture = std::make_unique<TextureCube>(size, num_mipmaps, array_size, format, access, sample_info);
				if (init_data.has_value())
					texture->HWResourceCreate(init_data.value());
				return texture.release();
			}

			ShaderCompose * platform_ex::Windows::D3D12::Device::CreateShaderCompose(std::unordered_map<ShaderCompose::Type, leo::observer_ptr<const asset::ShaderBlobAsset>> pShaderBlob, leo::observer_ptr<platform::Render::Effect::Effect> pEffect)
			{
				return std::make_unique<ShaderCompose>(pShaderBlob,pEffect).release();
			}

			GraphicsBuffer * Device::CreateConstantBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<ElementInitData const*> init_data)
			{
				return CreateBuffer(usage, access, size_in_byte,format, init_data);
			}

			GraphicsBuffer *Device::CreateBuffer(platform::Render::Buffer::Usage usage, leo::uint32 access, uint32 size_in_byte, EFormat format, std::optional<ElementInitData const*> init_data)
			{
				auto buffer = std::make_unique<GraphicsBuffer>(usage, access, (size_in_byte + 255)&~255, format);
				if (init_data.has_value())
					buffer->HWResourceCreate(init_data.value());
				return buffer.release();
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
					COMPtr_RefParam(d3d_cmd_allocators[Command_Render], IID_ID3D12CommandAllocator)));

				CheckHResult(d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
					COMPtr_RefParam(d3d_cmd_allocators[Command_Resource], IID_ID3D12CommandAllocator)));

				auto create_desc_heap = [&](D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT NumDescriptors,
					D3D12_DESCRIPTOR_HEAP_FLAGS Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, UINT NodeMask = 0)
				{
					D3D12_DESCRIPTOR_HEAP_DESC descriptor_desc = {
						Type,
						NumDescriptors,
						Flags,
						NodeMask
					};
					CheckHResult(d3d_device->CreateDescriptorHeap(&descriptor_desc,
						COMPtr_RefParam(d3d_desc_heaps[Type], IID_ID3D12DescriptorHeap)));
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

				FillCaps();
			}

			void Device::FillCaps() {
				d3d_caps.type =platform::Render::Caps::Type::D3D12;
				d3d_caps.max_texture_depth = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;

				std::pair<EFormat, DXGI_FORMAT> fmts[] =
				{
					std::make_pair(EF_A8, DXGI_FORMAT_A8_UNORM),
					std::make_pair(EF_R5G6B5, DXGI_FORMAT_B5G6R5_UNORM),
					std::make_pair(EF_A1RGB5, DXGI_FORMAT_B5G5R5A1_UNORM),
					std::make_pair(EF_ARGB4, DXGI_FORMAT_B4G4R4A4_UNORM),
					std::make_pair(EF_R8, DXGI_FORMAT_R8_UNORM),
					std::make_pair(EF_SIGNED_R8, DXGI_FORMAT_R8_SNORM),
					std::make_pair(EF_GR8, DXGI_FORMAT_R8G8_UNORM),
					std::make_pair(EF_SIGNED_GR8, DXGI_FORMAT_R8G8_SNORM),
					std::make_pair(EF_ARGB8, DXGI_FORMAT_B8G8R8A8_UNORM),
					std::make_pair(EF_ABGR8, DXGI_FORMAT_R8G8B8A8_UNORM),
					std::make_pair(EF_SIGNED_ABGR8, DXGI_FORMAT_R8G8B8A8_SNORM),
					std::make_pair(EF_A2BGR10, DXGI_FORMAT_R10G10B10A2_UNORM),
					std::make_pair(EF_SIGNED_A2BGR10, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM),
					std::make_pair(EF_R8UI, DXGI_FORMAT_R8_UINT),
					std::make_pair(EF_R8I, DXGI_FORMAT_R8_SINT),
					std::make_pair(EF_GR8UI, DXGI_FORMAT_R8G8_UINT),
					std::make_pair(EF_GR8I, DXGI_FORMAT_R8G8_SINT),
					std::make_pair(EF_ABGR8UI, DXGI_FORMAT_R8G8B8A8_UINT),
					std::make_pair(EF_ABGR8I, DXGI_FORMAT_R8G8B8A8_SINT),
					std::make_pair(EF_A2BGR10UI, DXGI_FORMAT_R10G10B10A2_UINT),
					std::make_pair(EF_R16, DXGI_FORMAT_R16_UNORM),
					std::make_pair(EF_SIGNED_R16, DXGI_FORMAT_R16_SNORM),
					std::make_pair(EF_GR16, DXGI_FORMAT_R16G16_UNORM),
					std::make_pair(EF_SIGNED_GR16, DXGI_FORMAT_R16G16_SNORM),
					std::make_pair(EF_ABGR16, DXGI_FORMAT_R16G16B16A16_UNORM),
					std::make_pair(EF_SIGNED_ABGR16, DXGI_FORMAT_R16G16B16A16_SNORM),
					std::make_pair(EF_R16UI, DXGI_FORMAT_R16_UINT),
					std::make_pair(EF_R16I, DXGI_FORMAT_R16_SINT),
					std::make_pair(EF_GR16UI, DXGI_FORMAT_R16G16_UINT),
					std::make_pair(EF_GR16I, DXGI_FORMAT_R16G16_SINT),
					std::make_pair(EF_ABGR16UI, DXGI_FORMAT_R16G16B16A16_UINT),
					std::make_pair(EF_ABGR16I, DXGI_FORMAT_R16G16B16A16_SINT),
					std::make_pair(EF_R32UI, DXGI_FORMAT_R32_UINT),
					std::make_pair(EF_R32I, DXGI_FORMAT_R32_SINT),
					std::make_pair(EF_GR32UI, DXGI_FORMAT_R32G32_UINT),
					std::make_pair(EF_GR32I, DXGI_FORMAT_R32G32_SINT),
					std::make_pair(EF_BGR32UI, DXGI_FORMAT_R32G32B32_UINT),
					std::make_pair(EF_BGR32I, DXGI_FORMAT_R32G32B32_SINT),
					std::make_pair(EF_ABGR32UI, DXGI_FORMAT_R32G32B32A32_UINT),
					std::make_pair(EF_ABGR32I, DXGI_FORMAT_R32G32B32A32_SINT),
					std::make_pair(EF_R16F, DXGI_FORMAT_R16_FLOAT),
					std::make_pair(EF_GR16F, DXGI_FORMAT_R16G16_FLOAT),
					std::make_pair(EF_B10G11R11F, DXGI_FORMAT_R11G11B10_FLOAT),
					std::make_pair(EF_ABGR16F, DXGI_FORMAT_R16G16B16A16_FLOAT),
					std::make_pair(EF_R32F, DXGI_FORMAT_R32_FLOAT),
					std::make_pair(EF_GR32F, DXGI_FORMAT_R32G32_FLOAT),
					std::make_pair(EF_BGR32F, DXGI_FORMAT_R32G32B32_FLOAT),
					std::make_pair(EF_ABGR32F, DXGI_FORMAT_R32G32B32A32_FLOAT),
					std::make_pair(EF_BC1, DXGI_FORMAT_BC1_UNORM),
					std::make_pair(EF_BC2, DXGI_FORMAT_BC2_UNORM),
					std::make_pair(EF_BC3, DXGI_FORMAT_BC3_UNORM),
					std::make_pair(EF_BC4, DXGI_FORMAT_BC4_UNORM),
					std::make_pair(EF_SIGNED_BC4, DXGI_FORMAT_BC4_SNORM),
					std::make_pair(EF_BC5, DXGI_FORMAT_BC5_UNORM),
					std::make_pair(EF_SIGNED_BC5, DXGI_FORMAT_BC5_SNORM),
					std::make_pair(EF_BC6, DXGI_FORMAT_BC6H_UF16),
					std::make_pair(EF_SIGNED_BC6, DXGI_FORMAT_BC6H_SF16),
					std::make_pair(EF_BC7, DXGI_FORMAT_BC7_UNORM),
					std::make_pair(EF_D16, DXGI_FORMAT_D16_UNORM),
					std::make_pair(EF_D24S8, DXGI_FORMAT_D24_UNORM_S8_UINT),
					std::make_pair(EF_D32F, DXGI_FORMAT_D32_FLOAT),
					std::make_pair(EF_ARGB8_SRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB),
					std::make_pair(EF_ABGR8_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB),
					std::make_pair(EF_BC1_SRGB, DXGI_FORMAT_BC1_UNORM_SRGB),
					std::make_pair(EF_BC2_SRGB, DXGI_FORMAT_BC2_UNORM_SRGB),
					std::make_pair(EF_BC3_SRGB, DXGI_FORMAT_BC3_UNORM_SRGB),
					std::make_pair(EF_BC7_SRGB, DXGI_FORMAT_BC7_UNORM_SRGB)
				};

				std::vector<EFormat> vertex_support_formats;
				std::vector<EFormat> texture_support_formats;
				std::unordered_map<EFormat,std::vector<SampleDesc>> rt_support_msaas;
				D3D12_FEATURE_DATA_FORMAT_SUPPORT fmt_support;
				for (size_t i = 0; i < sizeof(fmts) / sizeof(fmts[0]); ++i)
				{
					DXGI_FORMAT dxgi_fmt;
					if (IsDepthFormat(fmts[i].first))
					{
						switch (fmts[i].first)
						{
						case EF_D16:
							dxgi_fmt = DXGI_FORMAT_R16_TYPELESS;
							break;

						case EF_D24S8:
							dxgi_fmt = DXGI_FORMAT_R24G8_TYPELESS;
							break;

						case EF_D32F:
						default:
							dxgi_fmt = DXGI_FORMAT_R32_TYPELESS;
							break;
						}

						fmt_support.Format = dxgi_fmt;
						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER;
						fmt_support.Support2 = D3D12_FORMAT_SUPPORT2_NONE;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							vertex_support_formats.push_back(fmts[i].first);
						}

						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE1D;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							texture_support_formats.push_back(fmts[i].first);
						}
						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE2D;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							texture_support_formats.push_back(fmts[i].first);
						}
						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE3D;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							texture_support_formats.push_back(fmts[i].first);
						}
						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURECUBE;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							texture_support_formats.push_back(fmts[i].first);
						}
						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_SHADER_LOAD;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							texture_support_formats.push_back(fmts[i].first);
						}
						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							texture_support_formats.push_back(fmts[i].first);
						}
					}
					else
					{
						dxgi_fmt = fmts[i].second;

						fmt_support.Format = dxgi_fmt;
						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER;
						fmt_support.Support2 = D3D12_FORMAT_SUPPORT2_NONE;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							vertex_support_formats.push_back(fmts[i].first);
						}

						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE1D;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							texture_support_formats.push_back(fmts[i].first);
						}
						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE2D;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							texture_support_formats.push_back(fmts[i].first);
						}
						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE3D;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							texture_support_formats.push_back(fmts[i].first);
						}
						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURECUBE;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							texture_support_formats.push_back(fmts[i].first);
						}
						fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE;
						if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
						{
							texture_support_formats.push_back(fmts[i].first);
						}
					}

					bool rt_supported = false;
					fmt_support.Format = dxgi_fmt;
					fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
					fmt_support.Support2 = D3D12_FORMAT_SUPPORT2_NONE;
					if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
					{
						rt_supported = true;
					}
					fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET;
					if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
					{
						rt_supported = true;
					}
					fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL;
					if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
					{
						rt_supported = true;
					}

					if (rt_supported)
					{
						D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaa_quality_levels;
						msaa_quality_levels.Format = dxgi_fmt;

						UINT count = 1;
						while (count <= D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT)
						{
							msaa_quality_levels.SampleCount = count;
							if (SUCCEEDED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaa_quality_levels, sizeof(msaa_quality_levels))))
							{
								if (msaa_quality_levels.NumQualityLevels > 0)
								{
									rt_support_msaas[fmts[i].first].emplace_back(count, msaa_quality_levels.NumQualityLevels);
									count <<= 1;
								}
								else
								{
									break;
								}
							}
							else
							{
								break;
							}
						}
					}
				}

				d3d_caps.TextureFormatSupport = [formats=std::move(texture_support_formats)](EFormat format) {
					return std::find(formats.begin(),formats.end(),format) != formats.end();
				};

				d3d_caps.VertexFormatSupport = [formats = std::move(vertex_support_formats)](EFormat format) {
					return std::find(formats.begin(), formats.end(), format) != formats.end();
				};

				d3d_caps.RenderTargetMSAASupport = [formats = std::move(rt_support_msaas)](EFormat format,SampleDesc sample) {
					auto iter = formats.find(format);
					if (iter != formats.end()) {
						for (auto msaa : iter->second) {
							if ((sample.Count == msaa.Count) && (sample.Quality < msaa.Quality)) {
								return true;
							}
						}
					}
					return false;
				};
			}

			platform::Render::Caps& Device::GetCaps() {
				return d3d_caps;
			}

			platform::Render::Effect::BiltEffect * D3D12::Device::BiltEffect()
			{
				if (!bilt_effect)
					bilt_effect = std::make_unique<platform::Render::Effect::BiltEffect>("Bilt");
				return bilt_effect.get();
			}

			Context::Context()
				:adapter_list()
			{
#ifndef NDEBUG
				{
					COMPtr<ID3D12Debug> debug_ctrl;
					if (SUCCEEDED(D3D12::GetDebugInterface(COMPtr_RefParam(debug_ctrl, IID_ID3D12Debug)))) {
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

			void D3D12::Context::SyncCPUGPU(bool force)
			{
			}

			const COMPtr<ID3D12GraphicsCommandList> & D3D12::Context::GetCommandList(Device::CommandType index) const
			{
				return d3d_cmd_lists[index];
			}

			std::mutex & D3D12::Context::GetCommandListMutex(Device::CommandType index)
			{
				return cmd_list_mutexs[index];
			}

			void D3D12::Context::CommitCommandList(Device::CommandType)
			{
			}

			void D3D12::Context::Push(const platform::Render::PipleState &)
			{
				throw leo::unimplemented();
			}

			void Context::ContextEx(ID3D12Device * d3d_device, ID3D12CommandQueue * cmd_queue)
			{
				CheckHResult(d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
					device->d3d_cmd_allocators[Device::Command_Render].Get(), nullptr,
					COMPtr_RefParam(d3d_cmd_lists[Device::Command_Render], IID_ID3D12GraphicsCommandList)));

				CheckHResult(d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
					device->d3d_cmd_allocators[Device::Command_Resource].Get(), nullptr,
					COMPtr_RefParam(d3d_cmd_lists[Device::Command_Resource], IID_ID3D12GraphicsCommandList)));
			}

			void Context::CreateDeviceAndDisplay() {
				device = leo::make_shared<Device>(DefaultAdapter());
				ContextEx(device->d3d_device.Get(), device->d3d_cmd_queue.Get());
				DisplaySetting setting;
				display = leo::make_shared<Display>(GetDXGIFactory4(), device->d3d_cmd_queue.Get(), setting, g_hwnd);//test code
			}
			Context & Context::Instance()
			{
				static Context context;
				return context;
			}
		}
	}
}

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {
			bool Support() {
				return true;
			}
			platform::Render::Context& GetContext() {
				return Context::Instance();
			}
		}
	}
}