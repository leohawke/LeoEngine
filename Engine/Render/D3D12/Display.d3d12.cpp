#include "Display.h"
#include "Convert.h"
#include "Context.h"
#include <LFramework/LCLib/Platform.h>



using namespace platform_ex::Windows::D3D12;
using namespace platform_ex;

using std::make_shared;

Display::Display(IDXGIFactory4 * factory_4, ID3D12CommandQueue* cmd_queue, const DisplaySetting& setting, HWND hWnd)
	:hwnd(hWnd), frame_buffer(std::make_shared<FrameBuffer>()), frame_waitable_object(nullptr)
{
	full_screen = setting.full_screen;
	sync_interval = setting.sync_interval;

	//todo support WM_SIZE Message YSLib ?

	if (full_screen) {
		left = top = 0;
		width = setting.screen_width;
		height = setting.screen_height;
	}
	else {
		//TODO:change this code login to *Window Class
		RECT r;
		GetClientRect(hwnd, &r);
		POINT sp = { r.left,r.top };
		ClientToScreen(hwnd, &sp);
		left = sp.x;
		top = sp.y;
		width = r.right - r.left;
		height = r.bottom - r.top;
	}
	back_format = Convert(setting.color_format);
	depth_stencil_format = setting.depth_stencil_format;

	stereo_method = setting.stereo_method;
	stereo_feature = factory_4->IsWindowedStereoEnabled();

	//TODO:
	/*
	BOOL allow_tearing = FALSE;
	if(SUCCEEDED(factory_5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,&allow_tearing,sizeof(allow_tearing))))
	tearing_allow = allow_tearing;
	*/

	//todo rotate support
	//std::swap(width,height);

	auto stereo = (Stereo_LCDShutter == setting.stereo_method) && stereo_feature;
	factory_4->RegisterStereoStatusWindow(hwnd, WM_SIZE, &stereo_cookie);

	sc_desc.Width = width; sc_desc.Height = height;
	sc_desc.Format = back_format;
	sc_desc.Stereo = stereo;
	sc_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sc_desc.BufferCount = NUM_BACK_BUFFERS;
	sc_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	sc_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
	if (stereo) {
		sc_desc.SampleDesc.Count = 1;
		sc_desc.SampleDesc.Quality = 0;
		sc_desc.Scaling = DXGI_SCALING_NONE;
		sc_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	}
	else
	{
		sc_desc.SampleDesc.Count = std::min(static_cast<leo::uint32>(D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT), setting.sample_count);
		sc_desc.SampleDesc.Quality = setting.sample_quality;
		sc_desc.Scaling = DXGI_SCALING_STRETCH;
		sc_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	}

	//TODO
	/*
	if(tearing_fature)
	sc_desc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	*/

	//@{
	//TODO Match Adapter Mode
	sc_fs_desc.RefreshRate.Numerator = 60;
	sc_fs_desc.RefreshRate.Denominator = 1;
	//@}

	sc_fs_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sc_fs_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sc_fs_desc.Windowed = !full_screen;

	CreateSwapChain(factory_4, cmd_queue);

	back_buffer_index = swap_chain->GetCurrentBackBufferIndex();
	UpdateFramewBufferView();
}

bool platform_ex::Windows::D3D12::Display::CheckHDRSupport()
{
	uint32 ColorSpaceSupport = 0;
	if (SUCCEEDED(swap_chain->CheckColorSpaceSupport(DXGI_HDR_ColorSpace, &ColorSpaceSupport)))
	{
		return ColorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT;
	}
	return false;
}

void platform_ex::Windows::D3D12::Display::SwapBuffers()
{
	if (swap_chain) {
		auto rt_tex = render_targets_texs[back_buffer_index].get();

		TransitionResource(Context::Instance().GetDefaultCommandContext()->CommandListHandle, rt_tex, D3D12_RESOURCE_STATE_PRESENT, 0);

		Context::Instance().CommitCommandContext();

		bool allow_tearing = tearing_allow;
#ifdef LF_Hosted
		UINT const present_flags = allow_tearing ? DXGI_PRESENT_ALLOW_TEARING : 0;
#endif
		CheckHResult(swap_chain->Present(0, present_flags));

		back_buffer_index = swap_chain->GetCurrentBackBufferIndex();

		platform::Render::RenderTarget view;
		view.Texture = render_targets_texs[back_buffer_index].get();

		frame_buffer->Attach(FrameBuffer::Target0, view);
	}
}

void platform_ex::Windows::D3D12::Display::WaitOnSwapBuffers()
{
	if (swap_chain) {
		::WaitForSingleObjectEx(frame_waitable_object, 1000, true);
	}
}

void Display::CreateSwapChain(IDXGIFactory4 *factory_4, ID3D12CommandQueue* cmd_queue)
{
	COMPtr<IDXGISwapChain1>  swap_chain1 = nullptr;
	CheckHResult(factory_4->CreateSwapChainForHwnd(cmd_queue, hwnd,
		&sc_desc, &sc_fs_desc, nullptr, &swap_chain1.GetRef()));

	CheckHResult(swap_chain1->QueryInterface(IID_IDXGISwapChain3, reinterpret_cast<void**>(&swap_chain.GetRef())));

	if (frame_waitable_object != nullptr)
		::CloseHandle(frame_waitable_object);

	frame_waitable_object = swap_chain->GetFrameLatencyWaitableObject();
}

void Display::UpdateFramewBufferView()
{
	//TODO
	swap_chain->SetRotation(DXGI_MODE_ROTATION_IDENTITY);

	UINT rt_tex_index = 0;
	for (auto& rt_tex : render_targets_texs) {
		COMPtr<ID3D12Resource> pResources = nullptr;
		swap_chain->GetBuffer(rt_tex_index, COMPtr_RefParam(pResources, IID_ID3D12Resource));
		rt_tex = make_shared<Texture2D>(pResources);
		render_target_views[rt_tex_index] = new RenderTargetView(GetDefaultNodeDevice(),rt_tex->CreateRTVDesc(0,1,0),*rt_tex);

		rt_tex->SetNumRenderTargetViews(1);
		rt_tex->SetRenderTargetViewIndex(render_target_views[rt_tex_index], 0);

		++rt_tex_index;
	}

	platform::Render::RenderTarget view;
	view.Texture = render_targets_texs[0].get();

	frame_buffer->Attach(FrameBuffer::Target0, view);


	auto stereo = (Stereo_LCDShutter == stereo_method) && stereo_feature;

	if (depth_stencil_format != EF_Unknown) {
		platform::Render::ElementInitData initData;
		initData.clear_value = &platform::Render::ClearValueBinding::DepthOne;
		depth_stencil = leo::share_raw(Context::Instance().GetDevice().CreateTexture(
			static_cast<leo::uint16>(GetWidth()),
			static_cast<leo::uint16>(GetHeight()),
			1u,
			stereo ? 2u : 1u,
			depth_stencil_format,
			EA_GPURead | EA_GPUWrite,
			render_targets_texs[0]->GetSampleInfo(),
			&initData
		));

		depth_stencil->SetDepthStencilView(new DepthStencilView(GetDefaultNodeDevice(), depth_stencil->CreateDSVDesc(0, 1, 0), *depth_stencil, IsStencilFormat(depth_stencil_format)),0);
	}

	if (depth_stencil_format != EF_Unknown) {
		platform::Render::DepthRenderTarget view;
		view.Texture = depth_stencil.get();

		frame_buffer->Attach(FrameBuffer::DepthStencil, view);
		if (stereo)
		{
			throw leo::unsupported();
		}
	}
}
