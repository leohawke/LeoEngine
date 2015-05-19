#pragma once


#include <string>
#include <set>
#include <unordered_map>
#include <vector>
#include <string.h>
#include <memory>
#include <array>
#include "BaseMacro.h"
#include "Singleton.hpp"
#include "leoint.hpp"
#include "utility.hpp"
#include "RenderSystem\d3dx11.hpp"
#include "RenderSystem\DepthStencil.hpp"
#include "window.hpp"
#include "Core\COM.hpp"
namespace leo
{
	namespace win
	{
		class KeysState;
	}
	namespace global
	{
		///DeviceMgr
		extern ID3D11Device* globalD3DDevice;
		extern ID3D11DeviceContext* globalD3DContext;
		extern IDXGISwapChain* globalDXGISwapChain;
		extern std::unique_ptr<DepthStencil> globalDepthStencil;
		extern ID3D11RenderTargetView* globalD3DRenderTargetView;
		extern ID3D11Texture2D*	globalD3DRenderTargetTexture2D;
		extern std::set<std::pair<leo::uint16, leo::uint16>> globalDXGISizeSet;
		//share by DeviceMgr and Window
		extern std::pair<leo::uint16, leo::uint16> globalClientSize;

		extern float globalAspect;
		//WindowMgr
		extern win::HWND globalHwnd;


#if defined DEBUG || defined _DEBUG
		extern ID3D11Debug* globalD3DDebug;
#endif

		void Init();
		void Destroy();
	}
	
	
	
	

	class OutputWindow : public win::Window
	{
	public:
		using MsgFunction = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;
	public:
		bool Create(HINSTANCE hInstance,
			std::pair<leo::uint16,leo::uint16>& clientSize,
			LPCWSTR classname = L"OutputWindow",
			DWORD style = WS_BORDER,
			DWORD exStyle = 0,
			LPCWSTR iconRes = nullptr,
			LPCWSTR smalliconRes = nullptr
			);
	public:
		DefGetter(const _NOEXCEPT, win::HWND, Hwnd,global::globalHwnd);
		DefGetter(_NOEXCEPT, win::HWND&, Hwnd,global::globalHwnd);

		HINSTANCE	GetHinstance() const;

		BOOL		IsAlive() const;
		BOOL		IsMin() const;

		void		Destroy();

		void		BindMsgFunc(UINT message, MsgFunction msgFunction);

		std::pair<leo::uint16, leo::uint16> ClientSize() const lnothrow;
	public:
		virtual		LRESULT	WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		std::unordered_map<UINT, MsgFunction>	m_userMessages;			// User message map
		std::wstring m_regName;
	};
}