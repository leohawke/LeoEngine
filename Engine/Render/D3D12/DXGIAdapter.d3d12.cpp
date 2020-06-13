#include "DXGIAdapter.h"
#include <LFramework/Win32/LCLib/NLS.h>

using namespace platform;

namespace platform_ex {
	namespace Windows {
		namespace DXGI {
			Adapter::Adapter()
				:adapter_id(0)
			{
			}
			Adapter::Adapter(leo::uint32 adapter_id_, COMPtr<IDXGIAdapter1> const & adapter_)
				: adapter_id(adapter_id_)
			{
				Set(adapter_);
			}
			string Adapter::Description() const
			{
				return WCSToMBCS(adapter_desc.Description);
			}
			size_t Adapter::NumVideoMode() const
			{
				return video_modes.size();
			}
			VideoMode Adapter::GetVideoMode(size_t index) const
			{
				return video_modes[index];
			}
			HRESULT Adapter::Set(COMPtr<IDXGIAdapter1> const & adapter_)
			{
				adapter = adapter_;
				adapter->GetDesc1(&adapter_desc);
				video_modes.resize(0);

				COMPtr<IDXGIAdapter3> adapter3;
				auto hr = adapter->QueryInterface(COMPtr_RefParam(adapter3,IID_IDXGIAdapter3));
				if (adapter3 != nullptr)
				{
					DXGI_ADAPTER_DESC2 desc2;
					adapter3->GetDesc2(&desc2);
					memcpy(adapter_desc.Description, desc2.Description, sizeof(desc2.Description));
					adapter_desc.VendorId = desc2.VendorId;
					adapter_desc.DeviceId = desc2.DeviceId;
					adapter_desc.SubSysId = desc2.SubSysId;
					adapter_desc.Revision = desc2.Revision;
					adapter_desc.DedicatedVideoMemory = desc2.DedicatedVideoMemory;
					adapter_desc.DedicatedSystemMemory = desc2.DedicatedSystemMemory;
					adapter_desc.SharedSystemMemory = desc2.SharedSystemMemory;
					adapter_desc.AdapterLuid = desc2.AdapterLuid;
					adapter_desc.Flags = desc2.Flags;
				}
				return hr;
			}
			HRESULT Adapter::Enumerate()
			{
				vector<DXGI_FORMAT> formats = {
					DXGI_FORMAT_R8G8B8A8_UNORM,
					DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
					DXGI_FORMAT_B8G8R8A8_UNORM,
					DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
					//DXGI_FORMAT_R10G10B10A2_UNORM
				};

				UINT i = 0;
				IDXGIOutput* output = nullptr;
				while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
				{
					if (output)
					{
						for (auto format : formats)
						{
							UINT num = 0;
							output->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &num, 0);
							if (num > 0)
							{
								std::vector<DXGI_MODE_DESC> mode_descs(num);
								output->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &num, &mode_descs[0]);

								for (auto mode_desc : mode_descs)
								{
									VideoMode video_mode{ mode_desc.Width, mode_desc.Height,
										mode_desc.Format };

									// 如果找到一个新模式, 加入模式列表
									if (std::find(video_modes.begin(), video_modes.end(), video_mode) == video_modes.end())
									{
										video_modes.push_back(video_mode);
									}
								}
							}
						}

						output->Release();
						output = nullptr;
					}

					++i;
				}
				std::sort(video_modes.begin(), video_modes.end());
				return i > 0 ? S_OK : DXGI_ERROR_NOT_FOUND;
			}

			bool Adapter::CheckHDRSupport()
			{
				bool bSupportsHDROutput = false;

				UINT i = 0;
				IDXGIOutput* output = nullptr;
				while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
				{
					if (output)
					{
						COMPtr<IDXGIOutput6> Output6;
						if (SUCCEEDED(output->QueryInterface(COMPtr_RefParam(Output6, IID_IDXGIOutput6))))
						{
							DXGI_OUTPUT_DESC1 OutputDesc;
							Output6->GetDesc1(&OutputDesc);
							const bool bDisplaySupportsHDROutput = (OutputDesc.ColorSpace == D3D12::DXGI_HDR_ColorSpace);

							if (bDisplaySupportsHDROutput)
							{
								bSupportsHDROutput = true;

								LF_TraceRaw(Descriptions::Informative, "HDR output is supported on adapter %u(Description:%s), display %u", adapter_id, i, adapter_desc.Description);
								LF_TraceRaw(Descriptions::Informative, "\tMinLuminance = %f", OutputDesc.MinLuminance);
								LF_TraceRaw(Descriptions::Informative, "\tMaxLuminance = %f", OutputDesc.MaxLuminance);
								LF_TraceRaw(Descriptions::Informative, "\tMaxFullFrameLuminance = %f",OutputDesc.MaxFullFrameLuminance);

								break;
							}
						}

						output->Release();
						output = nullptr;
					}

					++i;
				}
				return bSupportsHDROutput;
			}

			AdapterList::AdapterList()
				:current_iterator(end())
			{
				UINT dxgi_factory_flags = 0;
#ifndef NDEBUG
				dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

				CheckHResult(CreateFactory2(dxgi_factory_flags,COMPtr_RefParam(dxgi_factory_4,IID_IDXGIFactory4)));
				CheckHResult(Enumerate());
			}

			Adapter & AdapterList::CurrentAdapter()
			{
				LAssertNonnull(current_iterator->NumVideoMode());
				return *current_iterator;
			}

			HRESULT AdapterList::Enumerate()
			{
				UINT adapter_i = 0;
				IDXGIAdapter1 *  dxgi_adapter = nullptr;
				while (dxgi_factory_4->EnumAdapters1(adapter_i, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND)
				{
					if (dxgi_adapter && SUCCEEDED(D3D12::CreateDevice(dxgi_adapter, D3D_FEATURE_LEVEL_11_0,
						IID_ID3D12Device,nullptr))) {
						emplace_back(adapter_i, dxgi_adapter);
						//check result not important
						back().Enumerate();
					}
					++adapter_i;
				}
				if (adapter_i == 0 || empty())
					return DXGI_ERROR_NOT_FOUND;

				current_iterator = begin();

				return S_OK;
			}
		}
	}
}