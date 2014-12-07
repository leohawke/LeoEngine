#define _CRTDBG_MAP_ALLOC
#include	<stdlib.h>
#include	<crtdbg.h>

#include <IndePlatform\platform.h>
#include <IndePlatform\Singleton.hpp>
#include <IndePlatform\ThreadSync.hpp>
#include <IndePlatform\clock.hpp>

#include <Core\Mesh.hpp>
#include <Core\Effect.h>
#include <Core\Camera.hpp>
#include <Core\RenderSync.hpp>
#include <Core\EffectLine.hpp>
#include <Core\Terrain.hpp>
#include <Core\Sky.hpp>
#include <Core\Skeleton.hpp>
#include <Core\MeshLoad.hpp>
#include <Core\EffectSkeleton.hpp>
#include <Core\\EngineConfig.h>
#include <COM.hpp>

#include <TextureMgr.h>
#include <ShaderMgr.h>
#include <RenderStates.hpp>
#include <exception.hpp>
#include <Input.h>


#include "Axis.hpp"

#include "window.hpp"
#include "COM.hpp"
#include "DeviceMgr.h"

#include <Commdlg.h>

#include <thread>
#include <atomic>
#include <mutex>
#include "resource.h"


leo::Event event;
std::unique_ptr<leo::Mesh> pMesh = nullptr;
std::unique_ptr<leo::Camera> pCamera = nullptr;
leo::Terrain<3,64,6>* pTerrain = nullptr;
std::unique_ptr<leo::Sky> pSky = nullptr;
std::unique_ptr<leo::SkeletonInstance[]> pSkeInstances = nullptr;
std::atomic<bool> renderAble = false;
std::atomic<bool> renderThreadRun = true;

std::mutex mSizeMutex;




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

void BuildRes();


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

	leo::EngineConfig::Read();
	leo::EngineConfig::Write();
	leo::DeviceMgr DeviceMgr;
	leo::OutputWindow win;

	auto clientSize = leo::EngineConfig::ClientSize();
	if (!win.Create(GetModuleHandle(nullptr), clientSize, L"Model LooK", 
		WS_BORDER | WS_SIZEBOX,0,
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
		if (DeviceMgr.GetDevice())
		{
			if (mHasDuration > mNeedDuration)
			{
				auto size = std::make_pair<leo::uint16, leo::uint16>(LOWORD(lParam), HIWORD(lParam));
				DeviceMgr.ReSize(size);
				mHasDuration = leo::clock::to_duration<>(0.f);
			}
			else
			{
				mHasDuration += leo::clock::now()-mTimePoint;
				mTimePoint = leo::clock::now();
			}
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	};

	win.BindMsgFunc(WM_SIZE, sizeproc);

	DeviceEvent();
	BuildRes();

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
			pMesh.reset(new leo::Mesh());
			if (pMesh->Load(GetOpenL3dFile(), leo::DeviceMgr().GetDevice()))
				renderAble = (true);
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
	
#if 0
	leo::ViewPort vp;
	UINT numVp = 1;
	D3D11_VIEWPORT dvp;
	DeviceMgr.GetDeviceContext()->RSGetViewports(&numVp, &dvp);
	vp.mHeight = dvp.Height;
	vp.mMaxDepth = dvp.MaxDepth;
	vp.mMinDepth = dvp.MinDepth;
	vp.mTLX = dvp.TopLeftX;
	vp.mTLY = dvp.TopLeftY;
	vp.mWindth = dvp.Width;
	leo::float4x4 proj;
	leo::XMStoreFloat4x4A((leo::XMFLOAT4X4A*)&proj, pCamera->Proj());
	

	auto mouseproc = [&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
		auto x = (float)GET_X_LPARAM(lParam);
		auto y = (float)GET_Y_LPARAM(lParam);
		leo::float4x4 inv;
		leo::XMVECTOR temp;
		auto ray = leo::Ray::Pick(vp, proj, leo::float2(x, y));
		for (auto i = 0; i != 10;++i){
			leo::XMStoreFloat4x4A((leo::XMFLOAT4X4A*)&inv, leo::XMMatrixInverse(&temp, pCamera->View())*leo::XMMatrixInverse(&temp, mBoxSqts[i].operator DirectX::XMMATRIX()));

			if (ray.Transform(inv).Normalize().Intersect(mBox.GetBoundingBox()).first)
				mBoxPicked[i] = true;
			else
				mBoxPicked[i] = false;
		}
		return 0;
	};
	win.BindMsgFunc(WM_LBUTTONDOWN, mouseproc);
	mBox.Color(leo::float4(1.f, 0.f, 0.f, 1.f));

	
#endif

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
		leo::win::KeysState::GetInstance()->Update();
	}
	renderThreadRun = false;
	updateThread.join();
	renderThread.join();
	
	leo::aligned_alloc<leo::Terrain<3, 64, 6>, 16> alloc;
	alloc.destroy(pTerrain);
	alloc.deallocate(pTerrain, 1);
	pMesh.reset(nullptr);
	//pTerrain.reset(nullptr);
	pSky.reset(nullptr);
	pSkeInstances.reset(nullptr);

	leo::global::Destroy();
#ifdef DEBUG
	leo::SingletonManger::GetInstance()->PrintAllSingletonInfo();
#endif
	leo::SingletonManger::GetInstance()->UnInstallAllSingleton();
	return 0;
}

inline void QuaternionToMatrix(const leo::float4& quaternion,leo::float4x4& matrix)
{
	auto x2 = quaternion.x*quaternion.x, y2 = quaternion.y*quaternion.y, z2 = quaternion.z*quaternion.z;
	auto xy = quaternion.x*quaternion.y, wz = quaternion.w*quaternion.z, wx = quaternion.w*quaternion.x;
	auto xz = quaternion.x*quaternion.z, yz = quaternion.y*quaternion.z, yw = quaternion.y*quaternion.w;

	matrix(0, 0) = 1 - 2 * (y2 + z2);	matrix(0, 1) = 2 * (xy + wz);	matrix(0, 2) = 2 * (xz - yw);
	matrix(1, 0) = 2 * (xy - wz);		matrix(1, 1) = 1 - 2 * (x2 + z2); matrix(1, 2) = 2 * (yz + wx);
	matrix(2, 0) = 2 * (xz + yw);		matrix(2, 1) = 2 * (yz - wx);	matrix(2, 2) = 1 - 2 * (x2 + y2);
}

inline leo::float4x4 QuaternionToMatrix(const leo::float4 & quaternion)
{
	leo::float4x4 matrix;
	::QuaternionToMatrix(quaternion, matrix);
	matrix(0, 3) = 0; matrix(1, 3) = 0; matrix(2, 3) = 0;
	matrix(3, 0) = 0; matrix(3, 1) = 0; matrix(3, 2) = 0;
	matrix(3, 3) = 1;
	return matrix;
}


void BuildRes()
{
	pCamera = std::make_unique<leo::UVNCamera>();

	using leo::float3;

	auto Eye = float3(0.f,10.f,-10.f);
	auto At = float3(0.f,0.f,0.f);
	auto Up = float3(0.f,1.f, 0.f);

	pCamera->LookAt(Eye, At, Up);

	event.Wait();

	pCamera->SetFrustum(leo::default_param::frustum_fov, leo::DeviceMgr().GetAspect(), leo::default_param::frustum_near, leo::default_param::frustum_far);
	pCamera->SetFrustum(leo::PROJECTION_TYPE::PERSPECTIVE);

	
	auto& pEffect =  leo::EffectNormalMap::GetInstance(leo::DeviceMgr().GetDevice());
	leo::EffectTerrain::GetInstance(leo::DeviceMgr().GetDevice());
	leo::EffectTerrainSO::GetInstance(leo::DeviceMgr().GetDevice());
	

	leo::DirectionLight dirlight;
	dirlight.ambient = leo::float4(1.f, 1.f, 1.f, 1.f);
	dirlight.diffuse = leo::float4(1.f, 1.f, 1.f, 1.f);
	dirlight.specular = leo::float4(1.f, 1.f, 1.f, 32.f);
	dirlight.dir = leo::float4(-0.5773f, -0.57735f,0.57735f, 0.f);

	pEffect->Light(dirlight);

	leo::EffectNormalLine::GetInstance(leo::DeviceMgr().GetDevice());
	leo::EffectSky::GetInstance(leo::DeviceMgr().GetDevice());
	leo::EffectSkeleton::GetInstance(leo::DeviceMgr().GetDevice())->Light(dirlight);
	//leo::EffectLine::GetInstance(leo::DeviceMgr().GetDevice());
	//leo::Axis::GetInstance(leo::DeviceMgr().GetDevice());


	struct TerrainFileHeader
	{
		float mChunkSize;
		std::uint32_t mHorChunkNum;
		std::uint32_t mVerChunkNum;
		wchar_t mHeightMap[leo::win::file::max_path];
	}mTerrainFileHeader;

	mTerrainFileHeader.mChunkSize = 24;
	mTerrainFileHeader.mHorChunkNum = 12;
	mTerrainFileHeader.mVerChunkNum = 12;
	wcscpy(mTerrainFileHeader.mHeightMap, L"Resource\\GaussianNoise256.jpg");

	{
		auto & pFile = leo::win::File::Open(L"Resource\\Test.Terrain", leo::win::File::TO_WRITE);
		pFile->Write(0, &mTerrainFileHeader, sizeof(mTerrainFileHeader));
	}
	leo::aligned_alloc<leo::Terrain<3, 64, 6>, 16> alloc;
	pTerrain = alloc.allocate(1);
	alloc.construct(pTerrain, leo::DeviceMgr().GetDevice(), L"Resource\\Test.Terrain");

	pSky = std::make_unique<leo::Sky>(leo::DeviceMgr().GetDevice(), L"Resource\\sunsetcube1024.dds");

	pSkeInstances = leo::make_unique<leo::SkeletonInstance[]>(3);

	auto skeData = leo::SkeletonData::Load(L"Resource\\soldier.l3d");
	pSkeInstances[0] = skeData;
	pSkeInstances[1] = skeData;
	pSkeInstances[2] = skeData;

	pSkeInstances[0].Scale(0.1f);
	pSkeInstances[1].Scale(0.05f);
	pSkeInstances[2].Scale(0.05f);

	pSkeInstances[0].Translation(float3(1.f,2.f,3.f));
	pSkeInstances[1].Translation(float3(3.f, 2.f, 1.f));
	pSkeInstances[2].Translation(float3(-5.f,1.f,5.f));



	//leo::XMMATRIX modelRot = leo::XMMatrixRotationY(leo::LM_PI);
	//leo::float4 quaternion;
	//save(quaternion,leo::XMQuaternionRotationMatrix(modelRot));
	//leo::float3 eulerangle = leo::QuaternionToEulerAngle(quaternion);
	//pSke->Rotation(quaternion);
	//pSke->Roll(eulerangle.x);
	//pSke->Pitch(eulerangle.y);
	//pSke->Yaw(eulerangle.z);
	//leo::DeviceMgr().GetDeviceContext()->RSSetState(leo::RenderStates().GetRasterizerState(L"WireframeRS"));
}

void Update(){
	while (renderThreadRun)
	{
		auto mBegin = leo::clock::now();

		if (GetAsyncKeyState('W') & 0X8000) 
			pSkeInstances[0].Translation(leo::float3(0.f, 0.f, -0.05f));

		if (GetAsyncKeyState('S') & 0X8000)
			pSkeInstances[0].Translation(leo::float3(0.0f, 0.f, +0.05f));

		if (GetAsyncKeyState('A') & 0X8000)
			pSkeInstances[0].Translation(leo::float3(+0.05f, 0.f, 0.f));

		if (GetAsyncKeyState('D') & 0X8000)
			pSkeInstances[0].Translation(leo::float3(-0.05f, 0.f, 0.f));

		pCamera->UpdateViewMatrix();


		leo::clock::GameClock::Update(leo::clock::ProgramClock::GetElapse());
		leo::clock::ProgramClock::Reset();

		pSkeInstances[0].Update();
		//pSkeInstances[1].Update();
		//pSkeInstances[2].Update();

		static const auto begin_call_back = [&]() {
			pSkeInstances[0].BeginCurrentAni();
		};

		static const auto end_call_back = [&]() {
			pSkeInstances[0].EndCurrentAni();
		};

		static leo::win::KeyDown mBeignEvent('W', begin_call_back);
		static leo::win::KeyUp mEndEvent('W', end_call_back);


		

		auto mRunTime =leo::clock::duration_to<>(leo::clock::now()-mBegin);
		if (mRunTime < 1 / 30.f)
			std::this_thread::sleep_for(leo::clock::to_duration<>(1 / 30.f - mRunTime));
	}
}


void Render()
{
	event.Wait();

	while (renderThreadRun)
	{
		leo::DeviceMgr dm;

		
		leo::RenderSync::GetInstance()->Sync();

		auto devicecontext = dm.GetDeviceContext();
		float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.8f };
		devicecontext->ClearRenderTargetView(dm.GetRenderTargetView(), ClearColor);
		devicecontext->ClearDepthStencilView(dm.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

		if (renderAble)
			pMesh->Render(devicecontext, *pCamera);
		
		//leo::Axis::GetInstance()->Render(devicecontext, *pCamera);

		

		pSky->Render(devicecontext, *pCamera);
		pTerrain->Render(devicecontext, *pCamera);

		auto& pos = pSkeInstances[0].Pos();
		auto y = pTerrain->GetHeight(leo::float2(pos.x, pos.z)) - pos.y;
		pSkeInstances[0].Translation(leo::float3(0.f,y, 0.f));

		pSkeInstances[0].Render(*pCamera);

		//pSkeInstances[1].Render(*pCamera);

		//pSkeInstances[2].Render(*pCamera);


		


		leo::DeviceMgr().GetSwapChain()->Present(0, 0);

		leo::RenderSync::GetInstance()->Present();

	}
}
