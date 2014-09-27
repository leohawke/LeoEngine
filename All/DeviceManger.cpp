#include "IndePlatform\platform.h"

#include <memory>


#include "DeviceMgr.h"
#include "exception.hpp"
#include "window.hpp"
#include "COM.hpp"


#include <dxgi.h>
#pragma comment(lib,"dxgi.lib")
namespace leo
{
	namespace win
	{
		const wchar_t ComputerInfo::CPUInfo::OEMNAME[3][6] = { L"Intel",L"AMD\0\0",L"FUCK!" };
		ComputerInfo::CPUInfo ComputerInfo::globalCpuInfo;
		void ComputerInfo::CPUInfo::Init()
		{
			SYSTEM_INFO systemInfo;
			GetNativeSystemInfo(&systemInfo);
			switch (systemInfo.wProcessorArchitecture)
			{
			case(PROCESSOR_ARCHITECTURE_AMD64) :
				ComputerInfo::globalCpuInfo.arch = ARCH::X64_86;
				break;
			case(PROCESSOR_ARCHITECTURE_IA64) :
				ComputerInfo::globalCpuInfo.arch = ARCH::X64;
				break;
			case(PROCESSOR_ARCHITECTURE_INTEL) :
				ComputerInfo::globalCpuInfo.arch = ARCH::X86;
				break;
			default:
				ComputerInfo::globalCpuInfo.arch = ARCH::X64_86;
				break;
			}

			char CPUString[0x20] = { 0 };
			char CPUBrandString[0x40] = {0};
			int cpuInfo[4] = {-1};
			int infotype{0};
			__cpuid(cpuInfo, 0);
			
			const int intelmask = 0X756E6547 & 0X6C65746E & 0X49656E69;
			const int amdmask = 0X68747541 & 0X444D14163 & 0X69746E65;
			const int mask = cpuInfo[1] & cpuInfo[2] & cpuInfo[3];
			switch (mask)
			{
			case intelmask:
				ComputerInfo::globalCpuInfo.oem = OEM::Intel;
				break;
			case amdmask:
				ComputerInfo::globalCpuInfo.oem = OEM::AMD;
				break;
			default:
				ComputerInfo::globalCpuInfo.oem = OEM::OTHER;
				break;
			}

			__cpuid(cpuInfo, 1);
			int nSteppinID = cpuInfo[0] & 0x0f;
			int nModel = (cpuInfo[0] >> 4) & 0xf;
			int nFamily = (cpuInfo[0] >> 8) & 0xf;
			
			using std::memcpy;

			__cpuid(cpuInfo, 0x80000002);
			memcpy(CPUBrandString, cpuInfo, sizeof(cpuInfo));
			__cpuid(cpuInfo, 0x80000003);
			memcpy(CPUBrandString+16, cpuInfo, sizeof(cpuInfo));
			__cpuid(cpuInfo, 0x80000004);
			memcpy(CPUBrandString+32, cpuInfo, sizeof(cpuInfo));

			char hzchar[5] = {0};
			auto clac = [](char c)
			{
				if (c == 'G')
					return 1024 * 1024 * 1024;
				return 0;
			};
			switch (ComputerInfo::globalCpuInfo.oem)
			{
			case OEM::Intel:
				memcpy(ComputerInfo::globalCpuInfo.model, CPUBrandString + 16, 17);
				memcpy(hzchar, CPUBrandString + 40,4);
				ComputerInfo::globalCpuInfo.hz =static_cast<std::size_t>(atof(hzchar)*clac(CPUBrandString[44]));
				ComputerInfo::globalCpuInfo.model[17] = '\0';
				break;
			default:
				break;
			}
		}
		ComputerInfo::RAMInfo ComputerInfo::globalRamInfo;
		void ComputerInfo::RAMInfo::Init()
		{
			MEMORYSTATUS memorystatus;
			GlobalMemoryStatus(&memorystatus);
			ComputerInfo::globalRamInfo.total = memorystatus.dwTotalPhys;
		}
	}

	DeviceMgr::Delegate::~Delegate()
	{
		reinterpret_cast<DeviceMgr*>(0)->DestroyDevice();
	}


	//ComputerInfo DeviceManger::computerinfo;
	bool DeviceMgr::CreateDevice(bool fullscreen,const size_type& size)
	{
		IDXGIFactory* pFactory = nullptr;
		auto EnumerAdapters = [&]()
		{
			std::vector<IDXGIAdapter*> Adapters;
			IDXGIAdapter* pAdapter = nullptr;
			DXGI_ADAPTER_DESC adapterDesc;
			dxcall(CreateDXGIFactory(IID_IDXGIFactory, (void **)&pFactory));
			leo::dx::DebugCOM(pFactory, "DeviceManger::CreateDevice::pFactory");
			for (win::uint i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				leo::dx::DebugCOM(pAdapter, "DeviceManger::CreateDevice::EnumerAdapters::pAdapter");
				Adapters.push_back(pAdapter);
				pAdapter->GetDesc(&adapterDesc);
				LogPrintf(L"AdapterIndex: %u VendorId: %u ", i, adapterDesc.VendorId);
				LogPrintf(L"DeviceId: %u SubSysId: %u ", adapterDesc.DeviceId, adapterDesc.SubSysId);
				LogPrintf(L"Revision: %u\n\t", adapterDesc.Revision);
				LogPrintf(L"Description: %ls\n\t", adapterDesc.Description);
				LogPrintf(L"DedicatedVideoMemory: %lu\n\t", adapterDesc.DedicatedVideoMemory);
				LogPrintf(L"DedicatedSystemMemory: %lu\n\t", adapterDesc.DedicatedSystemMemory);
				LogPrintf(L"SharedSystemMemory: %lu\n", adapterDesc.SharedSystemMemory);
			}
			return Adapters;
		};
		std::vector<IDXGIAdapter*> Adapters(EnumerAdapters());
		UINT createFlgas = 0;//这种flag永远不要出现在参数中,D3D11FEATURE待定
#if defined(DEBUG) | defined(_DEBUG)
		createFlgas |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		//只使用默认输出
		DXGI_MODE_DESC defaultMode;
		auto EnumSizes = [&](IDXGIAdapter* pAdapter, DXGI_FORMAT format)
		{
			IDXGIOutput* pOutput = nullptr;
			if (pAdapter->EnumOutputs(0, &pOutput) == DXGI_ERROR_NOT_FOUND)
				Raise_DX_Exception(DXGI_ERROR_NOT_FOUND, "Don't have a monitor");
			leo::dx::DebugCOM(pOutput, "DeviceManger::CreateDevice::EnumSizes::pOutput");
#if defined(DEBUG) | defined(_DEBUG)
			DXGI_OUTPUT_DESC outputDesc;
			pOutput->GetDesc(&outputDesc);
			LogPrintf(L"OUTPUT DevcieName: %s\n", outputDesc.DeviceName);
			LogPrintf(L"\tDesktopCoordinates:\n\t\tTop: %ld Left: %d Bottom: %ld Right: %ld\n",
				outputDesc.DesktopCoordinates.top, outputDesc.DesktopCoordinates.left,
				outputDesc.DesktopCoordinates.bottom, outputDesc.DesktopCoordinates.right);
#endif
			UINT numModes;
			pOutput->GetDisplayModeList(format, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
			LogPrintf(L"\tDisplayModelNums: %u\n", numModes);
			auto modeDescs = std::make_unique<DXGI_MODE_DESC[]>(numModes);
			pOutput->GetDisplayModeList(format, DXGI_ENUM_MODES_INTERLACED, &numModes, modeDescs.get());
			defaultMode = modeDescs[numModes - 1];
			for (UINT i = numModes; i != 0; --i)
			{
				size_type  modesize{ std::make_pair(modeDescs[i - 1].Width, modeDescs[i - 1].Height) };
				if (modesize == size)
					defaultMode = modeDescs[i - 1];
				LogPrintf(L"\t\t ModelsIndex: %2u Width: %4u Height: %4u ", numModes - i, modesize.first, modesize.second);
				LogPrintf(L"RefreshRate.Numerator: %2u RefreshRate.Denominator: %u\n",
					modeDescs[i - 1].RefreshRate.Numerator,
					modeDescs[i - 1].RefreshRate.Denominator
					);
				global::globalDXGISizeSet.insert(modesize);
			}
			leo::win::ReleaseCOM(pOutput);
		};
		D3D_FEATURE_LEVEL featureLevel;
		HRESULT hr = E_FAIL;
		for (auto &adapter : Adapters)
		{
			hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN,
				nullptr,
				createFlgas,
				nullptr, 0,
				D3D11_SDK_VERSION,
				&global::globalD3DDevice,
				&featureLevel,
				&global::globalD3DContext);
			if (SUCCEEDED(hr) && featureLevel >= D3D_FEATURE_LEVEL_11_0)
			{
				EnumSizes(adapter, DXGI_FORMAT_R8G8B8A8_UNORM);
				break;
			}
			else
			{
				if (SUCCEEDED(hr)){
					leo::win::ReleaseCOM(global::globalD3DDevice);
					leo::win::ReleaseCOM(global::globalD3DContext);
				}
			}
		}
		if (FAILED(hr) || featureLevel < D3D_FEATURE_LEVEL_11_0)
			Raise_DX_Exception(E_FAIL, "The GPU don't support D3DX11");
		leo::dx::DebugCOM(global::globalD3DDevice, "global::globalD3DDevice");
		leo::dx::DebugCOM(global::globalD3DContext, "global::globalD3DContext");
#if defined DEBUG || defined _DEBUG
		try{
			//ID3D11Device : RefCount += 1
			dxcall(global::globalD3DDevice->QueryInterface(&global::globalD3DDebug));
		}
		Catch_DX_Exception
#endif
		
		
		DXGI_SWAP_CHAIN_DESC sd;

		sd.Windowed = true;

		sd.BufferDesc = defaultMode;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		// Check 4X MSAA quality support for our back buffer format.
		// All Direct3D 11 capable devices support 4X MSAA for all render 
		// target formats, so we only need to check quality support.
		UINT m4xMsaaQuality = 0;
		dxcall(global::globalD3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality));

		//Warning: if not use MSAA,the VSGraphicDebug will throw thre exceptions
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = m4xMsaaQuality-1;

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.OutputWindow = global::globalHwnd;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;

		dxcall(pFactory->CreateSwapChain(global::globalD3DDevice, &sd, &global::globalDXGISwapChain));
		leo::dx::DebugCOM(global::globalDXGISwapChain, "global::globalDXGISwapChain");
		pFactory->MakeWindowAssociation(global::globalHwnd, DXGI_MWA_NO_ALT_ENTER);
		
		for (auto &x : Adapters)
			leo::win::ReleaseCOM(x);
		leo::win::ReleaseCOM(pFactory);

		ReSize(size);
		return true;
	}

	void DeviceMgr::DestroyDevice()
	{
		//assert(global::globalDXGISwapChain);
		global::globalD3DContext->Flush();
		global::globalD3DContext->ClearState();
		global::globalDXGISwapChain->SetFullscreenState(false, nullptr);
		leo::win::ReleaseCOM(global::globalD3DRenderTargetTexture2D);
		leo::win::ReleaseCOM(global::globalD3DRenderTargetView);
		leo::win::ReleaseCOM(global::globalD3DDepthTexture);
		leo::win::ReleaseCOM(global::globalD3DDepthStencilView);
		leo::win::ReleaseCOM(global::globalDXGISwapChain);
		leo::win::ReleaseCOM(global::globalD3DContext);
		leo::win::ReleaseCOM(global::globalD3DDevice);
#if defined DEBUG || defined _DEBUG
		if (global::globalD3DDebug){
			DebugPrintf("Please Ignoring The Infomation:\n \t\t'D3D11 WARNING: Live ID3D11Device at, Refcount: 2 [ STATE_CREATION WARNING #441: LIVE_DEVICE]'\n");
			DebugPrintf("If You Want Know Reason,View %s file %d line\n", __FILE__, __LINE__);
			//	Call ID3D11Device::QueryInterface(ID3D11Debug*) will Add ID3D11Device::RefCount,But We Need D3D11Debug::ReportLiveDeviceObjects
			//Inteface Show Important Infomation,It's mean RefCount is 1,In fact ,is 2.Maybe It's a bug
			global::globalD3DDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
		}
		leo::win::ReleaseCOM(global::globalD3DDebug);
#endif
	}

	void DeviceMgr::ReSize(const size_type& size)
	{
		global::globalD3DContext->OMSetRenderTargets(0, nullptr, nullptr);
		leo::win::ReleaseCOM(global::globalD3DRenderTargetTexture2D);
		leo::win::ReleaseCOM(global::globalD3DRenderTargetView);
		leo::win::ReleaseCOM(global::globalD3DDepthTexture);
		leo::win::ReleaseCOM(global::globalD3DDepthStencilView);

		try{
			dxcall(global::globalDXGISwapChain->ResizeBuffers(1, size.first, size.second, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

			dxcall(global::globalDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&global::globalD3DRenderTargetTexture2D)));
			leo::dx::DebugCOM(global::globalD3DRenderTargetTexture2D, "global::globalD3DRenderTargetTexture2D");

			dxcall(global::globalD3DDevice->CreateRenderTargetView(global::globalD3DRenderTargetTexture2D, 0, &global::globalD3DRenderTargetView));
			leo::dx::DebugCOM(global::globalD3DRenderTargetView, "global::globalD3DRenderTargetView");

			DXGI_SWAP_CHAIN_DESC swapDesc;
			dxcall(global::globalDXGISwapChain->GetDesc(&swapDesc));
			global::globalAspect = (float)swapDesc.BufferDesc.Width / swapDesc.BufferDesc.Height;

			D3D11_TEXTURE2D_DESC depthStencilDesc;
			depthStencilDesc.Width = swapDesc.BufferDesc.Width;
			depthStencilDesc.Height = swapDesc.BufferDesc.Height;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.ArraySize = 1;
			depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

			//4X MSAA? --must match swap chain MSAA values.
			depthStencilDesc.SampleDesc.Count = swapDesc.SampleDesc.Count;
			depthStencilDesc.SampleDesc.Quality = swapDesc.SampleDesc.Quality;


			depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
			depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depthStencilDesc.CPUAccessFlags = 0;
			depthStencilDesc.MiscFlags = 0;

			dxcall(global::globalD3DDevice->CreateTexture2D(&depthStencilDesc, 0, &global::globalD3DDepthTexture));
			leo::dx::DebugCOM(global::globalD3DDepthTexture, "global::globalD3DDepthTexture");

			dxcall(global::globalD3DDevice->CreateDepthStencilView(global::globalD3DDepthTexture, 0, &global::globalD3DDepthStencilView));
			leo::dx::DebugCOM(global::globalD3DDepthStencilView, "global::globalD3DDepthStencilView");

			global::globalD3DContext->OMSetRenderTargets(1, &global::globalD3DRenderTargetView, global::globalD3DDepthStencilView);

			D3D11_VIEWPORT vp;
			vp.Height = (float)swapDesc.BufferDesc.Height;
			vp.Width = (float)swapDesc.BufferDesc.Width;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			vp.MaxDepth = 1.0f;
			vp.MinDepth = 0.0f;
			global::globalD3DContext->RSSetViewports(1, &vp);
		}
		Catch_DX_Exception
	}

	void DeviceMgr::ToggleFullScreen(bool flags)
	{
		try{
			dxcall(global::globalDXGISwapChain->SetFullscreenState(flags, nullptr));
		}
		Catch_DX_Exception
	}
}