#pragma once

#include "Mgr.hpp"
#include "string.hpp"
namespace leo
{
	namespace win
	{
		class ComputerInfo
		{
		public:
			struct CPUInfo
			{
				static void Init();
				enum OEM :std::uint8_t
				{
					Intel = 0, AMD = 1, OTHER = 2
				} oem;
				static const wchar_t OEMNAME[3][6];
				char model[18];// {L"unknown"};
				enum ARCH :std::uint8_t
				{
					X86,
					X64_86,
					X64,
				} arch;
				std::size_t hz{ 0 };
				std::wstring GetInfo() const
				{
					std::wstring info(OEMNAME[globalCpuInfo.oem]);
					switch (globalCpuInfo.arch)
					{
					case leo::win::ComputerInfo::CPUInfo::X86:
						info += L" (x86) ";
						break;
					case leo::win::ComputerInfo::CPUInfo::X64_86:
						info += L" (X64_86) ";
						break;
					case leo::win::ComputerInfo::CPUInfo::X64:
						info += L" (X64) ";
						break;
					default:
						info += L" (what the fucking) ";
						break;
					}
					info += leo::to_wstring(globalCpuInfo.model, 17);
					return info;
				}

			};
			static CPUInfo globalCpuInfo;
			struct RAMInfo
			{
				static void Init();
				std::size_t total;
				std::wstring GetInfo()
				{
					std::wstring info(L"Total Memory:");
					return info + std::to_wstring(globalRamInfo.total);
				}
			};
			static RAMInfo globalRamInfo;
			static void Init()
			{
				CPUInfo::Init();
				RAMInfo::Init();
			}
		};
	}

	///DeviceMgr
	class DeviceMgr
	{
	private:
		class Delegate : public Singleton<Delegate>
		{
		public:
			~Delegate();
		public:
			static const std::unique_ptr<Delegate>& GetInstance()
			{
				static auto mInstance = unique_raw(new Delegate());
				return mInstance;
			}
		};
	public:
		DeviceMgr()
		{
			Delegate::GetInstance();
		}
		using size_type = std::pair<leo::uint16, leo::uint16>;
	public:
		void ToggleFullScreen(bool flags);
		void ReSize(const size_type& size);
		const decltype(global::globalDXGISizeSet) & QuerySize();
		bool CreateDevice(bool fullscreen, const size_type& size);
		void DestroyDevice();
		DefGetter(const lnothrow, ID3D11Device*, Device, global::globalD3DDevice);
		DefGetter(const lnothrow, ID3D11DeviceContext*, DeviceContext, global::globalD3DContext);
		DefGetter(const lnothrow, IDXGISwapChain*, SwapChain, global::globalDXGISwapChain);
		DefGetter(const lnothrow, ID3D11Texture2D*, DepthStencilTexture, global::globalD3DDepthTexture);
		DefGetter(const lnothrow, ID3D11DepthStencilView*, DepthStencilView, global::globalD3DDepthStencilView);
		DefGetter(const lnothrow, ID3D11RenderTargetView*, RenderTargetView, global::globalD3DRenderTargetView);
		DefGetter(const lnothrow, ID3D11Texture2D*, RenderTargetTexture2D, global::globalD3DRenderTargetTexture2D);
		DefGetter(const lnothrow, float, Aspect, global::globalAspect);
		DefGetter(const lnothrow, size_type, ClientSize, global::globalClientSize);
	};
}