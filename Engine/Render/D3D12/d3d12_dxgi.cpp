#include "d3d12_dxgi.h"

namespace platform_ex {
	namespace Windows {
		namespace DXGI {
			HRESULT CreateFactory2(UINT Flags,REFIID riid, void** ppFactory) {
				return (LoadProc<decltype(::CreateDXGIFactory2)>(L"dxgi.dll", "CreateDXGIFactory2"))(Flags,riid, ppFactory);
			}

			HRESULT CreateFactory1(REFIID riid, void** ppFactory) {
				return (LoadProc<decltype(::CreateDXGIFactory1)>(L"dxgi.dll", "CreateDXGIFactory1"))(riid, ppFactory);
			}
		}
		namespace D3D12 {
			HRESULT CreateDevice(IUnknown* pAdapter,
				D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid,
				void** ppDevice) {
				return (*LoadProc<decltype(::D3D12CreateDevice)>(L"d3d12.dll", "D3D12CreateDevice"))(pAdapter,
					MinimumFeatureLevel, riid,
					ppDevice);
			}
			HRESULT GetDebugInterface(REFIID riid, void** ppvDebug) {
				return (*LoadProc<decltype(::D3D12GetDebugInterface)>(L"d3d12.dll", "D3D12GetDebugInterface"))(riid, ppvDebug);
			}
			HRESULT SerializeRootSignature(D3D12_ROOT_SIGNATURE_DESC const * pRootSignature,
				D3D_ROOT_SIGNATURE_VERSION Version, ID3DBlob** ppBlob, ID3DBlob** ppErrorBlob) {
				return (*LoadProc<decltype(::D3D12SerializeRootSignature)>(L"d3d12.dll", "D3D12SerializeRootSignature"))(pRootSignature,
					Version,
					ppBlob,
					ppErrorBlob);
			}
		}
	}
}