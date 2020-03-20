/*! \file Engine\Render\D3D12\Display.h
\ingroup Engine
\brief 交换链相关逻辑封装。
*/

#ifndef LE_RENDER_D3D12_Display_h
#define LE_RENDER_D3D12_Display_h 1

#include <LBase/linttype.hpp>
#include "d3d12_dxgi.h"
#include "FrameBuffer.h"
#include "Texture.h"
#include "../IDisplay.h"
#include "ResourceView.h"

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

				EFormat depth_stencil_format = EF_D16;
				EFormat color_format = EF_ARGB8;
			};

			class Display :public platform::Render::Display {
			public:
				//todo Get Window's HWND by Other API.
				Display(IDXGIFactory4 *factory_4,ID3D12CommandQueue* cmd_queue,const DisplaySetting& setting = {},HWND = NULL);

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
				std::array<std::shared_ptr<RenderTargetView>, NUM_BACK_BUFFERS> render_target_views;
				std::shared_ptr<Texture2D> depth_stencil;

				UINT back_buffer_index;
				std::shared_ptr<FrameBuffer> frame_buffer;
				HANDLE frame_waitable_object;

				StereoMethod stereo_method;

				DXGI_COLOR_SPACE_TYPE ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
			};
		}
	}
}

#endif

