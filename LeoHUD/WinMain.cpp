#define _CRTDBG_MAP_ALLOC
#include	<stdlib.h>
#include	<crtdbg.h>

//first,Indeplafrom
#include <thread>
#include <atomic>
#include <mutex>
#include <platform.h>
#include <ThreadSync.hpp>
#include <clock.hpp>

#include <Core\EngineConfig.h>
#include <Core\RenderSync.hpp>
#include <Core\Camera.hpp>
#include <Core\EffectGBuffer.hpp>
#include <Core\EffectQuad.hpp>


#include "HUD\HUDHostRenderer.h"
#include "HUD\HUDPanel.h"
#include "HUD\Label.h"
#include "HUD\HUDBrush.h"

#include <RenderSystem\DeferredRender.hpp>
#include <RenderSystem\ShaderMgr.h>
#include <RenderSystem\RenderStates.hpp>
#include <RenderSystem\D3D11\D3D11Texture.hpp>
#include "DeviceMgr.h"

#include "FreeTypeTest.h"

leo::Event event;
std::mutex mRenderMutex;

std::unique_ptr<leo::UVNCamera> pCamera = nullptr;

std::shared_ptr<leo::HUD::HostRenderer> pHUDHostRender = nullptr;
std::unique_ptr<leo::HUD::Panel> pPanel = nullptr;

std::unique_ptr<leo::HUD::Label> pLabel = nullptr;
std::unique_ptr<leo::DeferredRender> pRender = nullptr;


void DeviceEvent()
{
	while (!leo::DeviceMgr().GetDevice())
	{
		Sleep(0);
	}
	event.Set();
}

void Render();
void Update();

void BuildRes(std::pair<leo::uint16, leo::uint16> size);

void ReSize(std::pair<leo::uint16, leo::uint16> size);

void ClearRes();

std::atomic<bool> renderThreadRun = true;


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);


	leo::EngineConfig::Read(L"config.scheme");
	leo::EngineConfig::ShaderConfig::GetAllBlendStateName();


	leo::DeviceMgr DeviceMgr;
	leo::OutputWindow win;

	auto clientSize = leo::EngineConfig::ClientSize();
	if (!win.Create(GetModuleHandle(nullptr), clientSize, L"Leo HUD",
		WS_BORDER | WS_SYSMENU, 0
		))
	{
		return 0;
	}

	DeviceMgr.CreateDevice(false, clientSize);

	auto mNeedDuration = leo::clock::to_duration<>(1.f);
	auto mHasDuration = leo::clock::to_duration<>(0.f);
	auto mTimePoint = leo::clock::now();
	auto sizeproc = [&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		leo::RenderSync::Block block;
		std::lock_guard<std::mutex> lock(mRenderMutex);

		if (DeviceMgr.GetDevice())
		{
			if (mHasDuration > mNeedDuration)
			{
				auto size = std::make_pair<leo::uint16, leo::uint16>(LOWORD(lParam), HIWORD(lParam));
				DeviceMgr.ReSize(size);
				ReSize(size);
				mHasDuration = leo::clock::to_duration<>(0.f);
			}
			else
			{
				mHasDuration += leo::clock::now() - mTimePoint;
				mTimePoint = leo::clock::now();
			}
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	};

	win.BindMsgFunc(WM_SIZE, sizeproc);

	DeviceEvent();
	BuildRes(clientSize);

	auto cmdmsgproc = [&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)-> LRESULT
	{
	/*	switch (LOWORD(wParam))
		{

		}*/
		return 0;
	};
	win.BindMsgFunc(WM_COMMAND, cmdmsgproc);

	std::thread renderThread(Render);
	std::thread updateThread(Update);
	while (true)
	{
		::MSG msg{ nullptr, 0, 0, 0, 0,{ 0, 0 } };
		if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0)
		{
			if (msg.message == WM_QUIT)
				break;

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
			::WaitMessage();
	}
	renderThreadRun = false;
	updateThread.join();
	renderThread.join();



	leo::EngineConfig::Write();
	leo::global::Destroy();
	ClearRes();


	return 0;
}


void BuildRes(std::pair<leo::uint16, leo::uint16> size)
{
	auto device = leo::DeviceMgr().GetDevice();

	event.Wait();
	//Effect,Staic Instance	
#if 1
	leo::EffectGBuffer::GetInstance(device);
	leo::EffectQuad::GetInstance(device);
#endif


	//Camera Set	
#if 1
	leo::Sphere mSphere{ leo::float3(0.0f, 0.0f, 0.0f),sqrtf(10.0f*10.0f + 15.0f*15.0f) };
	leo::float3 dir{ -0.5773f, -0.57735f,0.57735f };
	pCamera = std::make_unique<leo::UVNCamera>();




	leo::float3 pos;
	save(pos, leo::Multiply(leo::Splat(-2.f*mSphere.GetRadius()), leo::load(dir)));
	pos.x = -pos.x;
	pos.z = -pos.z;

	pCamera->LookAt(leo::float3(0.f, 12.f, -24.f), leo::float3(0.f, 0.f, 0.f), leo::float3(0.f, 1.f, 0.f));
	pCamera->SetFrustum(leo::default_param::frustum_fov, leo::DeviceMgr().GetAspect(), leo::default_param::frustum_near, leo::default_param::frustum_far);

	leo::EffectQuad::GetInstance().SetFrustum(device, *pCamera);
#endif

	pRender = std::make_unique<leo::DeferredRender>(device, size);

	pPanel = std::make_unique<leo::HUD::Panel>(leo::HUD::Size(size.first, size.second));
	pPanel->Background = leo::HUD::SolidBrush(leo::Drawing::ColorSpace::Yellow);
	pHUDHostRender = std::make_shared<leo::HUD::HostRenderer>(*pPanel);
	pPanel->SetRenderer(pHUDHostRender);

	pLabel = leo::HUD::MakeLabel("xiaxian baka");
	pLabel->SetVisible(true);
	*pPanel += *pLabel;

}




void ReSize(std::pair<leo::uint16, leo::uint16> size) {
	//do many thing ,but ÎÒ²»ÏëÐ´

	if (pRender) {
		pRender->ReSize(leo::DeviceMgr().GetDevice(), size);
	}
}

void Update() {
	while (renderThreadRun)
	{

		//std::lock_guard<std::mutex> lock(mRenderMutex);
		
		static auto mBegin = leo::clock::now();

		auto mRunTime = leo::clock::duration_to<>(leo::clock::now() - mBegin);
		mBegin = leo::clock::now();
		if (mRunTime < 1 / 30.f)
			std::this_thread::sleep_for(leo::clock::to_duration<>(1 / 30.f - mRunTime));
	}
}

void Render()
{
	event.Wait();
	static auto mBegin = leo::clock::now();
	while (renderThreadRun)
	{
		auto mRunTime = leo::clock::duration_to<>(leo::clock::now() - mBegin);
		mBegin = leo::clock::now();
		float dt = mRunTime;
		leo::DeviceMgr dm;

		pCamera->UpdateViewMatrix();

		leo::RenderSync::GetInstance()->Sync();
		std::lock_guard<std::mutex> lock(mRenderMutex);

		auto devicecontext = dm.GetDeviceContext();

		static const float rgba[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		devicecontext->ClearRenderTargetView(leo::global::globalD3DRenderTargetView, rgba);

		D3D11_VIEWPORT lastVp;
		UINT numVP = 1;
		devicecontext->RSGetViewports(&numVP, &lastVp);
		//DeferredRender
		{
			if (pRender) {
				pRender->OMSet(devicecontext, *leo::global::globalDepthStencil);
			}

			if (pRender) {
				pRender->UnBind(devicecontext, *leo::global::globalDepthStencil);
				pRender->LinearizeDepth(devicecontext, *leo::global::globalDepthStencil, pCamera->mNear, pCamera->mFar);
				pRender->LightPass(devicecontext, *leo::global::globalDepthStencil, *pCamera);
				pRender->ShadingPass(devicecontext, *leo::global::globalDepthStencil);
				pRender->PostProcess(devicecontext, leo::global::globalD3DRenderTargetView, dt);
			}
		}
		devicecontext->RSSetViewports(1, &lastVp);

		//forward render
		devicecontext->OMSetRenderTargets(1, &leo::global::globalD3DRenderTargetView, *leo::global::globalDepthStencil);
	
		pHUDHostRender->Render();

		leo::DeviceMgr().GetSwapChain()->Present(0, 0);

		leo::RenderSync::GetInstance()->Present();
	}
}

void ClearRes() {
	pRender.reset(nullptr);

	pHUDHostRender.reset();
	pPanel.reset();

#ifdef DEBUG
	leo::SingletonManger::GetInstance()->PrintAllSingletonInfo();
#endif
	leo::SingletonManger::GetInstance()->UnInstallAllSingleton();
	leo::DeviceMgr().DestroyDevice();
}