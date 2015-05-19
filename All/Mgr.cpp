#include "platform.h"
#include "Mgr.hpp"



namespace leo
{
	namespace global
	{
		///DeviceMgr
		extern ID3D11Device* globalD3DDevice = nullptr;
		extern ID3D11DeviceContext* globalD3DContext = nullptr;
		extern IDXGISwapChain* globalDXGISwapChain = nullptr;
		extern ID3D11Texture2D* globalD3DDepthTexture = nullptr;
		extern ID3D11DepthStencilView* globalD3DDepthStencilView = nullptr;
		extern ID3D11RenderTargetView* globalD3DRenderTargetView = nullptr;
		extern ID3D11Texture2D*	globalD3DRenderTargetTexture2D = nullptr;
		extern std::unique_ptr<DepthStencil> globalDepthStencil = nullptr;


		std::set<std::pair<leo::uint16, leo::uint16>> globalDXGISizeSet;
#if defined DEBUG || defined _DEBUG
		extern ID3D11Debug* globalD3DDebug = nullptr;
#endif
		//share by DeviceMgr and Window
		extern std::pair<leo::uint16, leo::uint16> globalClientSize = {};

		extern float globalAspect = 1.66f;
		//WindowMgr
		extern win::HWND globalHwnd = static_cast<win::HWND>(INVALID_HANDLE_VALUE);
		
		void Init()
		{}
		void Destroy()
		{
		}
	}
}