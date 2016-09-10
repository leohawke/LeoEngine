/*! \file Engine\Render\D3D12\Display.h
\ingroup Engine
\brief 交换链相关逻辑封装。
*/

#ifndef LE_RENDER_D3D12_Display_h
#define LE_RENDER_D3D12_Display_h 1

#include <LBase/linttype.hpp>
#include "d3d12_dxgi.h"

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {

			enum StereoMethod {
				Stereo_None,
				Stereo_LCDShutter,
			};

			//\brief 默认数值为默认设置
			struct DisplaySetting {
				leo::uint32 sample_count = 1;
				leo::uint32 sample_quality = 0;

				bool full_screen = false;
				bool sync_interval = true;

				leo::uint32  screen_width,screen_height;//属于用户设置

				StereoMethod stereo_method = Stereo_None;
			};

			class Display {
			public:
				//todo Get Window's HWND by Other API.
				Display(IDXGIFactory4 *factory_4,ID3D12CommandQueue* cmd_queue,const DisplaySetting& setting = {},HWND = NULL);

				lconstexpr static UINT const NUM_BACK_BUFFERS = 3;
			private:
				HRESULT CreateSwapChain(IDXGIFactory4* factory_4,ID3D12CommandQueue* cmd_queue);
			private:
				HWND hwnd;

				struct {
					bool stereo_feature = false;
					bool tearing_allow = false;
					DWORD stereo_cookie;
				};

				DXGI_SWAP_CHAIN_DESC1 sc_desc;
				DXGI_SWAP_CHAIN_FULLSCREEN_DESC sc_fs_desc;

				DXGI_FORMAT back_format;
				DXGI_FORMAT depth_stencil_format;

				bool full_screen;
				bool sync_interval;

				UINT left, top, width, height;//Win32 Coord Space

				COMPtr<IDXGISwapChain3> swap_chain;
			};
		}
	}
}

#endif

