#include "IndePlatform\platform.h"
#include "Mgr.hpp"
#include "Input.h"
#include <fstream>


#include "COM.hpp"
#include "DirectXTex.h"
#include "file.hpp"
#include "exception.hpp"

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
		std::set<std::pair<unsigned int, unsigned int>> globalDXGISizeSet;

#if defined DEBUG || defined _DEBUG
		extern ID3D11Debug* globalD3DDebug = nullptr;
#endif
		//share by DeviceMgr and Window
		std::pair<unsigned int, unsigned int> globalWindowSize;
		float globalAspect;
		//WindowMgr
		win::HWND globalHwnd;
		//GUID
		std::unordered_map<std::size_t, wchar_t*> globalStringTable;
		//
		win::KeysState globalKeysState;
		void Init()
		{}
		void Destroy()
		{
			for (auto &str : globalStringTable)
			{
				free(str.second);
			}
		}
	}
}