/*! \file Engine\Render\D3D12\Display.h
\ingroup Engine
\brief 交换链相关逻辑封装。
*/

#ifndef LE_RENDER_D3D12_Display_h
#define LE_RENDER_D3D12_Display_h 1

#include <LBase/linttype.hpp>

#include "../IDisplay.h"

#include "d3d12_dxgi.h"
#include "FrameBuffer.h"
#include "Texture.h"
#include "View.h"

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {
			class Display :public platform::Render::Display {
			public:
				//todo Get Window's HWND by Other API.
				Display(IDXGIFactory4 *factory_4,ID3D12CommandQueue* cmd_queue,const platform::Render::DisplaySetting& setting = {},HWND = NULL);

				DefGetter(const lnothrow, UINT, Width, width)
				DefGetter(const lnothrow,UINT,Height,height)
				DefGetter(const lnothrow,std::shared_ptr<FrameBuffer>,FrameBuffer,frame_buffer)
				lconstexpr static UINT const NUM_BACK_BUFFERS = 3;

				void SwapBuffers() override;
				void WaitOnSwapBuffers() override;

				bool CheckHDRSupport();
			private:
				void CreateSwapChain(IDXGIFactory4* factory_4,ID3D12CommandQueue* cmd_queue);

				void UpdateFramewBufferView();
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
				EFormat depth_stencil_format;

				bool full_screen;
				bool sync_interval;

				UINT left, top, width, height;//Win32 Coord Space

				COMPtr<IDXGISwapChain3> swap_chain;

				std::array<std::shared_ptr<Texture2D>, NUM_BACK_BUFFERS> render_targets_texs;
				std::array<RenderTargetView*, NUM_BACK_BUFFERS> render_target_views;
				std::shared_ptr<Texture2D> depth_stencil;

				UINT back_buffer_index;
				std::shared_ptr<FrameBuffer> frame_buffer;
				HANDLE frame_waitable_object;

				platform::Render::StereoMethod stereo_method;

				DXGI_COLOR_SPACE_TYPE ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
			};
		}
	}
}

#endif

