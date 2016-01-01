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

//second,Core and RenderSystem
#include <Core\Mesh.hpp>
#include <Core\Camera.hpp>
#include <Core\Terrain.hpp>
#include <Core\FileSearch.h>
#include <Core\EngineConfig.h>
#include <Core\RenderSync.hpp>
#include <Core\EffectQuad.hpp>
#include <Core\Light.hpp>
#include <Core\Skeleton.hpp>
#include <Core\Sky.hpp>
#include <Input.h>//to core!

//third,HUD
#include "HUD\HUDHostRenderer.h"
#include "HUD\HUDPanel.h"
#include "HUD\Label.h"

//Effect header
#include <Core\EffectGBuffer.hpp>
#include <RenderSystem\DeferredRender.hpp>
#include <RenderSystem\RenderStates.hpp>
#include "window.hpp"

//another
#include <Commdlg.h>
#include "resource.h"
#include <DirectXPackedVector.h>


#include "DeviceMgr.h"

leo::Event event;
std::vector<std::unique_ptr<leo::Mesh>> Models = {};
std::shared_ptr<leo::SkeletonData> pSkeletonData;
std::unique_ptr<leo::SkeletonInstance> pSkeletonModel;
std::unique_ptr<leo::UVNCamera> pCamera = nullptr;
std::unique_ptr<leo::CastShadowCamera> pShaderCamera;
std::unique_ptr<leo::DeferredRender> pRender = nullptr;
std::unique_ptr<leo::Terrain<>> pTerrain = nullptr;
std::unique_ptr<leo::Sky> pSky = nullptr;

std::shared_ptr<leo::HUD::HostRenderer> pHUDHostRender = nullptr;
std::unique_ptr<leo::HUD::Panel> pPanel = nullptr;

std::unique_ptr<leo::HUD::Label> pLabel = nullptr;

std::atomic<bool> renderAble = false;
std::atomic<bool> renderThreadRun = true;

std::mutex mSizeMutex;
std::mutex mRenderMutex;

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

std::wstring GetOpenL3dFile()
{
	wchar_t szPathName[MAX_PATH] = {};
	OPENFILENAME ofn;
	std::memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetForegroundWindow();

	ofn.lpstrTitle = L"选择模型文件";
	ofn.lpstrFilter = L"L3D Files\0*.l3d\0\0";

	ofn.lpstrFile = szPathName;
	ofn.nMaxFile = sizeof(szPathName);

	//以当前目录为初始选择目录
	wchar_t szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	ofn.lpstrInitialDir = szCurDir;

	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetOpenFileName(&ofn))
	{
		return std::wstring(szPathName);
	}
	return std::wstring();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);


	leo::EngineConfig::Read(L"config.scheme");
	leo::EngineConfig::ShaderConfig::GetAllBlendStateName();


	leo::DeviceMgr DeviceMgr;
	leo::OutputWindow win;

	auto clientSize = leo::EngineConfig::ClientSize();
	if (!win.Create(GetModuleHandle(nullptr), clientSize, L"Model LooK",
		WS_BORDER, 0,
		MAKEINTRESOURCE(IDI_ICON1)))
	{
		return 0;
	}

	HACCEL hAccel = LoadAccelerators(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDR_ACCELERATOR1));

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

	auto pModelMesh = std::make_unique<leo::Mesh>();
	pModelMesh->Load(L"Resource/Sphere.l3d", leo::DeviceMgr().GetDevice());
	pModelMesh->Translation(leo::float3(0.f,0.f,3.f));
	pModelMesh->Scale(2.f);
	Models.push_back(std::move(pModelMesh));

	auto cmdmsgproc = [&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)-> LRESULT
	{
		switch (LOWORD(wParam))
		{
		case ID_QUIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		case ID_OPEN:
		{
			leo::RenderSync::Block block;
			renderAble = false;
			auto pModelMesh = std::make_unique<leo::Mesh>();
			if (pModelMesh->Load(GetOpenL3dFile(), leo::DeviceMgr().GetDevice())) {
				renderAble = (true);
				pModelMesh->Scale(8.f);
				Models.push_back(std::move(pModelMesh));
			}
		}
		break;
		case ID_ROLL_LEFT:
			pCamera->Roll(-1.f);
			break;
		case ID_ROLL_RIGHT:
			pCamera->Roll(+1.f);
			break;
		case ID_YAW_FRONT:
			pCamera->Yaw(-1.f);
			break;
		case ID_YAW_BACK:
			pCamera->Yaw(+1.f);
			break;
		case ID_PITCH_UP:
			pCamera->Pitch(-1.f);
			break;
		case ID_PITCH_DOWN:
			pCamera->Pitch(+1.f);
			break;
		case ID_WALK_GO:
			pCamera->Walk(+1.f);
			break;
		case ID_WALK_BACK:
			pCamera->Walk(-1.f);
			break;
		case ID_NORMALLINE:
			if (!leo::EffectConfig::GetInstance()->NormalLine())
			{
				bool t = renderAble;
				renderAble = false;
				leo::EffectConfig::GetInstance()->NormalLine(true);
				renderAble = t;
			}
			else
			{
				leo::EffectConfig::GetInstance()->NormalLine(false);
			}
			break;
		default:
			break;
		}
		return 0;
	};
	win.BindMsgFunc(WM_COMMAND, cmdmsgproc);

	std::thread renderThread(Render);
	std::thread updateThread(Update);
	while (true)
	{
		::MSG msg{ nullptr, 0, 0, 0, 0, { 0, 0 } };
		if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0)
		{
			if (msg.message == WM_QUIT)
				break;
			if (!TranslateAccelerator(win.GetHwnd(), hAccel, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
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
	#ifdef DEBUG
	leo::SingletonManger::GetInstance()->PrintAllSingletonInfo();
	#endif
	leo::SingletonManger::GetInstance()->UnInstallAllSingleton();
	DeviceMgr.DestroyDevice();



	return 0;
}


void BuildLight(ID3D11Device* device) {

	auto mPointLight = std::make_shared<leo::PointLightSource>();
	mPointLight->Position(leo::float3(0.f, 0.f,0.f));
	mPointLight->Range(6.f);
	mPointLight->Diffuse(leo::float3(0.8f, 0.7f, 0.6f));
	mPointLight->FallOff(leo::float3(1.f, 0.05f, 0.01f));
	pRender->AddLight(mPointLight);

	auto mSpotLight = std::make_shared<leo::SpotLightSource>();
	mSpotLight->InnerAngle(leo::LM_RPD * 10);
	mSpotLight->OuterAngle(leo::LM_RPD * 55);
	mSpotLight->Diffuse(leo::float3(0.9f, 0.2f, 0.2f));
	mSpotLight->Directional(leo::float3(0.f, 0.707f, 0.707f));
	mSpotLight->FallOff(leo::float3(0.f, 0.1f, 0.1f));
	mSpotLight->Position(leo::float3(0.f, 0.f, -3.f));
	mSpotLight->Range(6.f);
	//pRender->AddLight(mSpotLight);

	auto mDirLight = std::make_shared<leo::DirectionalLightSource>();
	mDirLight->Directional(leo::float3(0.f, -1.f, 0.f));
	mDirLight->Diffuse(leo::float3(1.9f, 1.9f, 1.1f));
	//pRender->AddLight(mDirLight);
}
void ClearLight() {
}

void BuildRes(std::pair<leo::uint16, leo::uint16> size)
{
	using leo::float3;
	auto device = leo::DeviceMgr().GetDevice();

	event.Wait();
	//Effect,Staic Instance	
	#if 1
	leo::EffectGBuffer::GetInstance(device);
	leo::EffectQuad::GetInstance(device);
	leo::EffectSky::GetInstance(device);
	leo::EffectTerrain::GetInstance(device);
	#endif


		//Camera Set	
	#if 1
	leo::Sphere mSphere{ leo::float3(0.0f, 0.0f, 0.0f),sqrtf(10.0f*10.0f + 15.0f*15.0f) };
	float3 dir{ -0.5773f, -0.57735f,0.57735f };
	pCamera = std::make_unique<leo::UVNCamera>();
	pShaderCamera = std::make_unique<leo::CastShadowCamera>();
	pShaderCamera->SetSphereAndDir(mSphere, dir);




	leo::float3 pos;
	save(pos, leo::Multiply(leo::Splat(-2.f*mSphere.GetRadius()), leo::load(dir)));
	pos.x = -pos.x;
	pos.z = -pos.z;

	pCamera->LookAt(float3(0.f, 12.f, -24.f), float3(0.f, 0.f, 0.f), float3(0.f, 1.f, 0.f));
	pCamera->SetFrustum(leo::default_param::frustum_fov, leo::DeviceMgr().GetAspect(), leo::default_param::frustum_near, leo::default_param::frustum_far);

	leo::EffectQuad::GetInstance().SetFrustum(device, *pCamera);
	#endif

	pRender = std::make_unique<leo::DeferredRender>(device, size);

	pSky = std::make_unique<leo::Sky>(device, L"Resource\\snowcube1024.dds");
	//pTerrain = std::make_unique<leo::Terrain<>>(device, L"Resource\\Test.Terrain");
	pSkeletonData = leo::SkeletonData::Load(L"Resource\\soldier.l3d");
	pSkeletonModel = std::make_unique<leo::SkeletonInstance>(pSkeletonData);
	BuildLight(leo::DeviceMgr().GetDevice());

	pPanel = std::make_unique<leo::HUD::Panel>(leo::HUD::Size(size.first,size.second));
	pHUDHostRender = std::make_shared<leo::HUD::HostRenderer>(*pPanel);
	pPanel->SetRenderer(pHUDHostRender);

	pLabel = leo::HUD::MakeLabel("xiaxian baka");
	pLabel->SetVisible(true);
	*pPanel += *pLabel;
}

void ClearRes() {

	for (auto& prt : Models)
		prt.reset(nullptr);
	pTerrain.reset(nullptr);
	pSky.reset(nullptr);
	pRender.reset(nullptr);

	pHUDHostRender.reset();
	pPanel.reset();
	pSkeletonModel.reset();
	pSkeletonData.reset();

	ClearLight();
}

void ReSize(std::pair<leo::uint16, leo::uint16> size) {
	//do many thing ,but 我不想写


	if (pRender) {
		pRender->ReSize(leo::DeviceMgr().GetDevice(), size);
	}
}

void Update() {
	while (renderThreadRun)
	{

		leo::win::KeysState::GetInstance()->Update();

		std::lock_guard<std::mutex> lock(mRenderMutex);
		const float Radius = 9;
		const float cosb = std::cos(1.f / leo::LM_DPR);
		const float sinb = std::sin(1.f / leo::LM_DPR);

		auto total_mesh = Models.size();
		auto theta = leo::LM_TWOPI / total_mesh;
		auto i = 0u;
		for (auto & pModelMesh : Models) {
			leo::float3 pos(2 * leo::sinr(theta*i), 2 * leo::cosr(theta*i), 3);
			//pModelMesh->t = pos;
			++i;
		}

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

			for (auto & pModelMesh : Models) {
				pModelMesh->Render(devicecontext, *pCamera);
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
		if (pSky) {
			pSky->Render(devicecontext, *pCamera);
		}
		
		//pHUDHostRender->Render({static_cast<leo::uint16>(lastVp.Width),static_cast<leo::uint16>(lastVp.Height)});

		leo::DeviceMgr().GetSwapChain()->Present(0, 0);

		leo::RenderSync::GetInstance()->Present();

	}
}

