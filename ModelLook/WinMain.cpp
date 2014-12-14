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
#include <Core\EffectShadowMap.hpp>
#include <Core\Terrain.hpp>
#include <Core\Sky.hpp>
#include <Core\Skeleton.hpp>
#include <Core\MeshLoad.hpp>
#include <Core\EffectSkeleton.hpp>
#include <Core\\EngineConfig.h>
#include <Core\ShadowMap.hpp>
#include <Core\Vertex.hpp>
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
std::unique_ptr<leo::Mesh> pModelMesh = nullptr;
std::unique_ptr<leo::Mesh> pTerrainMesh = nullptr;
std::unique_ptr<leo::Mesh> pBoxMesh = nullptr;
std::unique_ptr<leo::Mesh> pSphereMesh = nullptr;
std::unique_ptr<leo::UVNCamera> pCamera = nullptr;
std::unique_ptr<leo::CastShadowCamera> pShaderCamera;

std::atomic<bool> renderAble = false;
std::atomic<bool> renderThreadRun = true;

std::mutex mSizeMutex;

ID3D11Buffer* mFillScreenVB = nullptr;


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
	
	leo::EngineConfig::ShaderConfig::GetAllBlendStateName();
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
			pModelMesh.reset(new leo::Mesh());
			if (pModelMesh->Load(GetOpenL3dFile(), leo::DeviceMgr().GetDevice()))
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
	
	pModelMesh.reset(nullptr);
	pTerrainMesh.reset(nullptr);
	pBoxMesh.reset(nullptr);
	pSphereMesh.reset(nullptr);
	leo::win::ReleaseCOM(mFillScreenVB);
	leo::global::Destroy();
#ifdef DEBUG
	leo::SingletonManger::GetInstance()->PrintAllSingletonInfo();
#endif
	leo::SingletonManger::GetInstance()->UnInstallAllSingleton();

	leo::EngineConfig::Write();

	return 0;
}


void BuildRes()
{

	auto noise = [](float x, float z) {
		return 0.f;
	};

	//leo::MeshFile::terrainTol3d(noise, std::make_pair(10u, 15u), std::make_pair(30u, 45u), L"Resource/Terrain.l3d");
	//leo::MeshFile::meshdataTol3d(leo::helper::CreateBox(1.f, 1.f, 1.f), L"Resource/Box.l3d");
	//leo::MeshFile::meshdataTol3d(leo::helper::CreateSphere(1.f, 32,32), L"Resource/Sphere.l3d");

	pCamera = std::make_unique<leo::UVNCamera>();

	using leo::float3;

	auto Eye = float3(0.f,10.f,-10.f);
	auto At = float3(0.f,0.f,0.f);
	auto Up = float3(0.f,1.f, 0.f);

	pCamera->LookAt(Eye, At, Up);

	event.Wait();

	pCamera->SetFrustum(leo::default_param::frustum_fov, leo::DeviceMgr().GetAspect(), leo::default_param::frustum_near, leo::default_param::frustum_far);

	
	auto& pEffect =  leo::EffectNormalMap::GetInstance(leo::DeviceMgr().GetDevice());	

	leo::DirectionLight dirlight;
	dirlight.ambient = leo::float4(1.f, 1.f, 1.f, 1.f);
	dirlight.diffuse = leo::float4(1.f, 1.f, 1.f, 1.f);
	dirlight.specular = leo::float4(1.f, 1.f, 1.f, 32.f);
	dirlight.dir = leo::float4(-0.5773f, -0.57735f,0.57735f, 0.f);

	pEffect->Light(dirlight);

	leo::EffectNormalLine::GetInstance(leo::DeviceMgr().GetDevice());
	leo::ShadowMap::GetInstance(leo::DeviceMgr().GetDevice(), std::make_pair(2048u,2048u));
	leo::EffectPack::GetInstance(leo::DeviceMgr().GetDevice());
	leo::EffectShadowMap::GetInstance(leo::DeviceMgr().GetDevice());



	leo::Sphere mSphere{ leo::float3(0.0f, 0.0f, 0.0f),sqrtf(10.0f*10.0f + 15.0f*15.0f) };
	float3 dir{ -0.5773f, -0.57735f,0.57735f };
	pShaderCamera = std::make_unique<leo::CastShadowCamera>();
	pShaderCamera->SetSphereAndDir(mSphere, dir);

	pTerrainMesh.reset(new leo::Mesh());
	pSphereMesh.reset(new leo::Mesh());
	pBoxMesh.reset(new leo::Mesh());

	pTerrainMesh->Load(L"Resource/Terrain.l3d", leo::DeviceMgr().GetDevice());
	pSphereMesh->Load(L"Resource/Sphere.l3d", leo::DeviceMgr().GetDevice());
	pBoxMesh->Load(L"Resource/Box.l3d", leo::DeviceMgr().GetDevice());

	pTerrainMesh->Translation(leo::float3(0.f, -1.f, 0.f));
	pSphereMesh->Translation(leo::float3(1.f, 0.5f, 0.f));
	auto& vertices = leo::helper::CreateFullscreenQuad();

	D3D11_BUFFER_DESC vbDesc;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.ByteWidth = vertices.size() * sizeof(leo::Vertex::PostEffect);
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = vertices.data();

	try {
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateBuffer(&vbDesc, &subData, &mFillScreenVB));


		leo::dx::DebugCOM(mFillScreenVB, "ShadowMap::Draw");
	}
	Catch_DX_Exception

}

void Update(){
	while (renderThreadRun)
	{
		auto mBegin = leo::clock::now();

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


		//Build Shadow Map
		leo::ShadowMap::GetInstance().BeginShadowMap(devicecontext,*pShaderCamera);
		if (renderAble)
			pModelMesh->CastShadow(devicecontext);

		pTerrainMesh->CastShadow(devicecontext);
		pBoxMesh->CastShadow(devicecontext);
		pSphereMesh->CastShadow(devicecontext);

		leo::ShadowMap::GetInstance().EndShadowMap(devicecontext);

		auto& pPackEffect = leo::EffectPack::GetInstance();
		pPackEffect->SetDstRTV(dm.GetRenderTargetView());
		pPackEffect->SetPackSRV(leo::ShadowMap::GetInstance().GetDepthSRV());
		pPackEffect->Apply(devicecontext);
		devicecontext->IASetInputLayout(leo::ShaderMgr().CreateInputLayout(leo::InputLayoutDesc::PostEffect));
		UINT strides[] = { sizeof(leo::Vertex::PostEffect) };
		UINT offsets[] = { 0 };
		devicecontext->IASetVertexBuffers(0, 1, &mFillScreenVB, strides, offsets);
		devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		devicecontext->Draw(4, 0);

		pPackEffect->SetPackSRV(nullptr, devicecontext);

		//状态机回置
		leo::context_wrapper context(devicecontext);

		leo::DeviceMgr().GetSwapChain()->Present(0, 0);

		leo::RenderSync::GetInstance()->Present();

	}
}
