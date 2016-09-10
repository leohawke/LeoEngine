/*! \file Engine\Render\d3d12_dxgi.h
\ingroup Engine
\brief 包装需要动态载入的函数和声明相关接口。
*/
#ifndef LE_RENDER_D3D12_d3d12_dxgi_h
#define LE_RENDER_D3D12_d3d12_dxgi_h 1

#include "../../Win32/COM.h"

#include <UniversalDXSDK/d3d12.h>
#include <UniversalDXSDK/dxgi1_5.h>

namespace platform_ex {
	namespace Windows {
		/*
		\note 桌面平台这些函数是直接通过LoadProc来实现
		\warning 引擎对于这些函数不会也不能频繁调用,无视LoadProc的开销
		\warning 若相应模块未事先载入 抛出Win32Exception
		\todo UWP支持
		*/
		namespace DXGI {
			using namespace leo;

			HRESULT CreateFactory1(REFIID riid, void** ppFactory);
		}
		namespace D3D12 {
			using namespace leo;

			HRESULT CreateDevice(IUnknown* pAdapter,
				D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid,
				void** ppDevice);
			HRESULT GetDebugInterface(REFIID riid, void** ppvDebug);
			HRESULT SerializeRootSignature(D3D12_ROOT_SIGNATURE_DESC const * pRootSignature,
				D3D_ROOT_SIGNATURE_VERSION Version, ID3D10Blob** ppBlob, ID3D10Blob** ppErrorBlob);
		}
	}
}

#endif