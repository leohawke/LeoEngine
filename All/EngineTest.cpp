#define _CRTDBG_MAP_ALLOC
#include	<stdlib.h>
#include	<crtdbg.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>

#include "scheme_helper.h"
#include "resource.h"
#include "Mgr.hpp"
#include "exception.hpp"
#include "COM.hpp"

decltype(leo::scheme::sexp::parse(L"")) config_sexp;
unsigned int m4xMsaaQuality = 0;
unsigned int mWidth = 0;
unsigned int mHeight = 0;
leo::OutputWindow mWin;

void OnResize();

bool InitDirect3D()
{
	using namespace leo;
	using namespace win;
	// Create the device and device context.
	auto size = std::make_pair(mWidth, mHeight);
	unsigned int createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	using size_type = std::pair<unsigned int, unsigned int>;

	IDXGIFactory* pFactory = nullptr;
	auto EnumerAdapters = [&]()
	{
		std::vector<IDXGIAdapter*> Adapters;
		IDXGIAdapter* pAdapter = nullptr;
		DXGI_ADAPTER_DESC adapterDesc;
		dxcall(CreateDXGIFactory(IID_IDXGIFactory, (void **)&pFactory));
		leo::dx::DebugCOM(pFactory, "DeviceManger::CreateDevice::pFactory");
		for (std::size_t i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			Adapters.push_back(pAdapter);
			pAdapter->GetDesc(&adapterDesc);
			LogPrintf(L"AdapterIndex: %u VendorId: %u ", i, adapterDesc.VendorId);
			LogPrintf(L"DeviceId: %u SubSysId: %u ", adapterDesc.DeviceId, adapterDesc.SubSysId);
			LogPrintf(L"Revision: %u\n\t", adapterDesc.Revision);
			LogPrintf(L"Description: %s\n\t", adapterDesc.Description);
			LogPrintf(L"DedicatedVideoMemory: %lu\n\t", adapterDesc.DedicatedVideoMemory);
			LogPrintf(L"DedicatedSystemMemory: %lu\n\t", adapterDesc.DedicatedSystemMemory);
			LogPrintf(L"SharedSystemMemory: %lu\n", adapterDesc.SharedSystemMemory);
		}
		return Adapters;
	};
	std::vector<IDXGIAdapter*> Adapters(EnumerAdapters());
	unsigned int createFlgas = 0;//这种flag永远不要出现在参数中,D3D11FEATURE待定
	//只使用默认输出
	DXGI_MODE_DESC defaultMode;
	auto EnumSizes = [&](IDXGIAdapter* pAdapter, DXGI_FORMAT format)
	{
		IDXGIOutput* pOutput = nullptr;
		if (pAdapter->EnumOutputs(0, &pOutput) == DXGI_ERROR_NOT_FOUND)
			Raise_DX_Exception(DXGI_ERROR_NOT_FOUND, "Don't have a monitor");
#if defined(DEBUG) | defined(_DEBUG)
		DXGI_OUTPUT_DESC outputDesc;
		pOutput->GetDesc(&outputDesc);
		LogPrintf(L"OUTPUT DevcieName: %s\n", outputDesc.DeviceName);
		LogPrintf(L"\tDesktopCoordinates:\n\t\tTop: %ld Left: %d Bottom: %ld Right: %ld\n",
			outputDesc.DesktopCoordinates.top, outputDesc.DesktopCoordinates.left,
			outputDesc.DesktopCoordinates.bottom, outputDesc.DesktopCoordinates.right);
#endif
		unsigned int numModes;
		pOutput->GetDisplayModeList(format, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
		LogPrintf(L"\tDisplayModelNums: %u\n", numModes);
		auto modeDescs = std::make_unique<DXGI_MODE_DESC[]>(numModes);
		pOutput->GetDisplayModeList(format, DXGI_ENUM_MODES_INTERLACED, &numModes, modeDescs.get());
		defaultMode = modeDescs[numModes - 1];
		for (unsigned int i = numModes; i != 0; --i)
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
	}
	if (FAILED(hr) || featureLevel < D3D_FEATURE_LEVEL_11_0)
		Raise_DX_Exception(E_FAIL, "The GPU don't support D3DX11");

	DXGI_SWAP_CHAIN_DESC sd;
	/*
	sd.BufferDesc.Width = mWidth;
	sd.BufferDesc.Height = mHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	*/
	// Use 4X MSAA? 
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m4xMsaaQuality-1;//this is very interesting!
	}
	//*/
	sd.BufferDesc = defaultMode;

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow  = mWin.GetHwnd();
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* dxgiDevice = 0;
	dxcall(global::globalD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	dxcall(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));
	IDXGIFactory * dxgiFactory = 0;
	dxcall(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	dxcall(dxgiFactory->CreateSwapChain(global::globalD3DDevice, &sd, &global::globalDXGISwapChain));
	dxcall(dxgiFactory->EnumAdapters(0, &dxgiAdapter));
	//dxcall(dxgiFactory->MakeWindowAssociation(mhMainWnd,DXGI_MWA_NO_WINDOW_CHANGES));

	leo::win::ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);
	//ReleaseCOM(pAdapter);
	ReleaseCOM(pFactory);
	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.

	leo::DeviceMgr DeviceMgr;
	//ReleaseRenderRes();
	DeviceMgr.ReSize(size);

	return true;
}

void Render(const leo::DeviceMgr& dm)
{
	auto rtv = dm.GetRenderTargetView();
	auto dsv = dm.GetDepthStencilView();
	auto context = dm.GetDeviceContext();

	float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.8f };

	context->ClearRenderTargetView(rtv, ClearColor);
	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	leo::dxcall(dm.GetSwapChain()->Present(0, 0));
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	leo::ReSetStdIOtoFile(leo::which_stdio::ostream, L"CONOUT$");
	leo::DeviceMgr DeviceMgr;
	

	config_sexp = leo::parse_file(S("config.scheme"));

	auto window_sexp = leo::scheme::sexp::find_sexp(L"window", config_sexp);

	if (!mWin.Create(GetModuleHandle(NULL), leo::expack_string(window_sexp, S("tile"))))
	{
		return 0;
	}
	HACCEL hAccel = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_ACCELER1));

	mWidth = leo::expack_long(window_sexp, S("window"));
	mHeight = leo::expack_long(window_sexp, S("height"));
#ifndef LEO_ENGINE
	DeviceMgr.CreateDevice(false, std::make_pair(mWidth, mHeight));
#else
	InitDirect3D();
#endif


	auto cmdmsgproc = [&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)-> LRESULT
	{
		switch (LOWORD(wParam))
		{
		case ID_ESC:
			SendMessage(hwnd, WM_DESTROY, 0, 0);
			break;
		case ID_DRAW_MODE:
			DeviceMgr.ToggleFullScreen(true);
			break;
		case ID_SELECT_MODE:
			DeviceMgr.ToggleFullScreen(false);
			break;
		default:
			break;
		}
		return 0;
	};
	auto sizeproc = [&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (DeviceMgr.GetDevice())
		{
			auto size = std::make_pair<int, int>(LOWORD(lParam), HIWORD(lParam));
			//ReleaseRenderRes();
			DeviceMgr.ReSize(size);
			//ResizeRenderRes(DeviceMgr);
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	};
	mWin.BindMsgFunc(WM_COMMAND, cmdmsgproc);
	mWin.BindMsgFunc(WM_SIZE, sizeproc);
	// Run the message loop.
	MSG msg = { 0 };
	while (1)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, nullptr, 0, 0))
				goto QUIT;
			if (!TranslateAccelerator(mWin.GetHwnd(), hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		Render(DeviceMgr);
	}
	//leo::ReSetStdIOtoFile(leo::which_stdio::istream, L"CONIN$");
	FreeConsole();
QUIT:
	leo::global::Destroy();
	mWin.Destroy();
	return 0;
}
