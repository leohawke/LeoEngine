#define _CRTDBG_MAP_ALLOC
#include	<stdlib.h>
#include	<crtdbg.h>

#include "platform.h"

#include "Singleton.hpp"
#include "ThreadSync.hpp"
#include "clock.hpp"

#include <Core\Mesh.hpp>
#include <Core\Effect.h>
#include <Core\Camera.hpp>
#include <Core\RenderSync.hpp>
#include <Core\EffectLine.hpp>
#include <Core\EffectShadowMap.hpp>
#include <Core\Sky.hpp>
#include <Core\Skeleton.hpp>
#include <Core\MeshLoad.hpp>
#include <Core\EffectSkeleton.hpp>
#include <Core\EffectGBuffer.hpp>
#include <Core\\EngineConfig.h>
#include <Core\ShadowMap.hpp>
#include <Core\Vertex.hpp>
#include <Core\FileSearch.h>


#include <RenderSystem\Deferred.h>

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
#include <DirectXPackedVector.h>



leo::Event event;
std::unique_ptr<leo::Mesh> pModelMesh = nullptr;
std::unique_ptr<leo::Mesh> pTerrainMesh = nullptr;
std::unique_ptr<leo::Mesh> pBoxMesh = nullptr;
std::unique_ptr<leo::Mesh> pSphereMesh = nullptr;
std::unique_ptr<leo::UVNCamera> pCamera = nullptr;
std::unique_ptr<leo::CastShadowCamera> pShaderCamera;
std::unique_ptr<leo::Axis> pAxis = nullptr;

std::atomic<bool> renderAble = false;
std::atomic<bool> renderThreadRun = true;

std::mutex mSizeMutex;
std::mutex mRenderMutex;

ID3D11PixelShader* mSSAOPS = nullptr;

ID3D11Buffer* mSSAOPSCB = nullptr;
ID3D11ShaderResourceView* mSSAORandomVec = nullptr;


//用于显示GBuffer
ID3D11PixelShader* mGBufferPS = nullptr;

struct SSAO {
	leo::float4x4 gProj;
	leo::float4 gOffsetVectors[14];

	float    gOcclusionRadius = 5.5f;
	float    gOcclusionFadeStart = 2.0f;
	float    gOcclusionFadeEnd = 20.0f;
	float    gSurfaceEpsilon = 0.55f;
};
SSAO ssao;


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
	_CrtSetBreakAlloc(309);
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	leo::EngineConfig::Read(L"test.scheme");
	

	std::string path = "/config/test/bool/f";
	bool b = true;
	leo::EngineConfig::Read(path, b);


	path = "/config/test/double/";
	double d;
	leo::EngineConfig::Read(path + "0.0", d);

	path = "/config/test/string";
	leo::EngineConfig::Read(path,path);

	path += "vector/3";
	std::vector<std::string> strings;

	leo::EngineConfig::Read(path, strings);

	leo::float2 f2;
	path = "/config/test/f/2";
	leo::EngineConfig::Read(path, f2);

	leo::float3 f3;
	path = "/config/test/f/3";
	leo::EngineConfig::Read(path, f3);

	leo::float4 f4;
	path = "/config/test/f/4";
	leo::EngineConfig::Read(path, f4);

	leo::half2 h2{0.f,1.f};
	path = "/config/test/h/2";
	leo::EngineConfig::Read(path, h2);

	leo::half3 h3{ h2,leo::half(2.f) };
	path = "/config/test/h/3";
	leo::EngineConfig::Read(path, h3);

	leo::half4 h4{leo::half(3.f),h3};
	path = "/config/test/h/4";
	leo::EngineConfig::Read(path, h4);

	leo::EngineConfig::Write(L"test.scheme");

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
		std::lock_guard<std::mutex> lock(mRenderMutex);

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
		
	}
	renderThreadRun = false;
	updateThread.join();
	renderThread.join();
	
	

	
	leo::global::Destroy();
	ClearRes();
#ifdef DEBUG
	leo::SingletonManger::GetInstance()->PrintAllSingletonInfo();
#endif
	leo::SingletonManger::GetInstance()->UnInstallAllSingleton();


	
	return 0;
}

struct PointLight
{
	leo::float4 diffuse;
	leo::float4 position;//w : range
	leo::float4 att;//ignore w;
} pl;

ID3D11PixelShader* mPointLightPS = nullptr;

ID3D11Buffer* mPointLightPSCB = nullptr;

void BuildLight(ID3D11Device* device) {
	leo::ShaderMgr sm;
	auto  mPSBlob = sm.CreateBlob(leo::FileSearch::Search(L"PointLightPS.cso"));
	mPointLightPS = sm.CreatePixelShader(mPSBlob);


	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DEFAULT;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = 0;
	Desc.MiscFlags = 0;
	Desc.StructureByteStride = 0;
	Desc.ByteWidth = sizeof(PointLight);

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = &pl;
	subData.SysMemPitch = 0;
	subData.SysMemSlicePitch = 0;

	pl.diffuse = leo::float4(0.8f, 0.8f, 0.8f, 1.f);
	pl.att = leo::float4(0.f,1.f,0.f,1.f);
	const float Radius = 9;
	pl.position = leo::float4(Radius,0.f,0.f,8.f);

	leo::dxcall(device->CreateBuffer(&Desc, &subData, &mPointLightPSCB));
}
void ClearLight() {
	leo::win::ReleaseCOM(mPointLightPSCB);
}
void BuildRes()
{
	using leo::float3;

	event.Wait();
//Effect,Staic Instance	
#if 1
	//leo::EffectNormalLine::GetInstance(leo::DeviceMgr().GetDevice());
	leo::ShadowMap::GetInstance(leo::DeviceMgr().GetDevice(), std::make_pair(2048u, 2048u));
	leo::EffectPack::GetInstance(leo::DeviceMgr().GetDevice());
	
	leo::EffectShadowMap::GetInstance(leo::DeviceMgr().GetDevice());
	leo::EffectLine::GetInstance(leo::DeviceMgr().GetDevice());
	
	leo::DeferredResources::GetInstance();
	leo::EffectGBuffer::GetInstance(leo::DeviceMgr().GetDevice());
#endif
	pAxis = std::make_unique<leo::Axis>(leo::DeviceMgr().GetDevice());


	pTerrainMesh.reset(new leo::Mesh());
	pSphereMesh.reset(new leo::Mesh());
	pBoxMesh.reset(new leo::Mesh());

	pTerrainMesh->Load(L"Resource/Terrain.l3d", leo::DeviceMgr().GetDevice());
	pSphereMesh->Load(L"Resource/Sphere.l3d", leo::DeviceMgr().GetDevice());
	pBoxMesh->Load(L"Resource/skull.l3d", leo::DeviceMgr().GetDevice());
	pSphereMesh->Translation(leo::float3(-6.f, 6.5f, 0.f));

	pSphereMesh->Scale(5.f);
	pTerrainMesh->Scale(8.f);

	pBoxMesh->Scale(2.f);
	pBoxMesh->Translation(leo::float3(+0.f, -5.f, 0.0f));


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

	pCamera->LookAt(float3(0.f,10.f,-10.f), float3(0.f,0.f,0.f), float3(0.f, 1.f, 0.f));
	pCamera->SetFrustum(leo::default_param::frustum_fov, leo::DeviceMgr().GetAspect(), leo::default_param::frustum_near, leo::default_param::frustum_far);
	
	leo::DeferredResources::GetInstance().SetFrustum(*pCamera);
#endif
//SSAO Dependent
#if 1
	//SSAO ,GPU资源
	leo::ShaderMgr sm;
	auto  mPSBlob = sm.CreateBlob(leo::FileSearch::Search(L"SSAOPS.cso"));
	auto mGBufferBlob = sm.CreateBlob(leo::FileSearch::Search(L"GBufferPS.cso"));
	mGBufferPS = sm.CreatePixelShader(mGBufferBlob);
	mSSAOPS = sm.CreatePixelShader(mPSBlob);


	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DEFAULT;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = 0;
	Desc.MiscFlags = 0;
	Desc.StructureByteStride = 0;
	Desc.ByteWidth = sizeof(SSAO);

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = &ssao;
	subData.SysMemPitch = 0;
	subData.SysMemSlicePitch = 0;
	
	// 8 cube corners
	ssao.gOffsetVectors[0] = leo::float4(+1.0f, +1.0f, +1.0f, 0.0f);
	ssao.gOffsetVectors[1] = leo::float4(-1.0f, -1.0f, -1.0f, 0.0f);

	ssao.gOffsetVectors[2] = leo::float4(-1.0f, +1.0f, +1.0f, 0.0f);
	ssao.gOffsetVectors[3] = leo::float4(+1.0f, -1.0f, -1.0f, 0.0f);

	ssao.gOffsetVectors[4] = leo::float4(+1.0f, +1.0f, -1.0f, 0.0f);
	ssao.gOffsetVectors[5] = leo::float4(-1.0f, -1.0f, +1.0f, 0.0f);

	ssao.gOffsetVectors[6] = leo::float4(-1.0f, +1.0f, -1.0f, 0.0f);
	ssao.gOffsetVectors[7] = leo::float4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	ssao.gOffsetVectors[8] = leo::float4(-1.0f, 0.0f, 0.0f, 0.0f);
	ssao.gOffsetVectors[9] = leo::float4(+1.0f, 0.0f, 0.0f, 0.0f);

	ssao.gOffsetVectors[10] = leo::float4(0.0f, -1.0f, 0.0f, 0.0f);
	ssao.gOffsetVectors[11] = leo::float4(0.0f, +1.0f, 0.0f, 0.0f);

	ssao.gOffsetVectors[12] = leo::float4(0.0f, 0.0f, -1.0f, 0.0f);
	ssao.gOffsetVectors[13] = leo::float4(0.0f, 0.0f, +1.0f, 0.0f);

	for (auto & v : ssao.gOffsetVectors) {
		float s = (rand()*1.f  / RAND_MAX)*0.75f +0.25f;
		leo::save(v, leo::Multiply(load(v), s));
	}
	
	float data[] = { 0.5f,0.f,0.f,0.f,
					0.f,-0.5f,0.f,0.f,
					0.f,0.f,1.f,0.f,
					0.5f,0.5f,0.f,1.f };
	leo::float4x4 toTex{data};

	leo::save(ssao.gProj, 
		leo::Transpose(
		leo::Multiply(
			leo::load(pCamera->Proj()),
			load(toTex))));


	leo::dxcall(leo::DeviceMgr().GetDevice()->CreateBuffer(&Desc, &subData, &mSSAOPSCB));

	//mSSAORandomVec
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.SysMemPitch = 256 * sizeof(DirectX::PackedVector::XMCOLOR);

	DirectX::PackedVector::XMCOLOR color[256 * 256];
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			leo::float3 v(rand()*1.f/RAND_MAX, rand()*1.f / RAND_MAX, rand()*1.f / RAND_MAX);

			color[i * 256 + j] = DirectX::PackedVector::XMCOLOR(v.x, v.y, v.z, 0.0f);
		}
	}

	initData.pSysMem = color;

	ID3D11Texture2D* tex = 0;
	leo::dxcall(leo::DeviceMgr().GetDevice()->CreateTexture2D(&texDesc, &initData, &tex));

	leo::dxcall(leo::DeviceMgr().GetDevice()->CreateShaderResourceView(tex, 0, &mSSAORandomVec));

	// view saves a reference.
	leo::win::ReleaseCOM(tex);

	BuildLight(leo::DeviceMgr().GetDevice());
#endif

}
void ClearRes() {
	leo::win::ReleaseCOM(mSSAOPSCB);

	pModelMesh.reset(nullptr);
	pTerrainMesh.reset(nullptr);
	pBoxMesh.reset(nullptr);
	pSphereMesh.reset(nullptr);

	leo::win::ReleaseCOM(mSSAORandomVec);

	ClearLight();
}


void Update(){
	while (renderThreadRun)
	{
		
		leo::win::KeysState::GetInstance()->Update();

		std::lock_guard<std::mutex> lock(mRenderMutex);
		const float Radius = 9;
		const float cosb = std::cos(1.f/leo::LM_DPR);
		const float sinb = std::sin(1.f /leo::LM_DPR);

		//cos(a+b) = cos(a) cos(b) - sin(a) sin(b) 
		//sin(a + b) = sin(a) cos(b) + sin(b) cos(a)
		//cosa = x ,sina = y
		//sinb = sin(pi/180)
		auto cosa = pl.position.x/Radius;
		auto sina = pl.position.y/Radius;
		pl.position.x = cosa*cosb - sina*sinb;
		pl.position.y = sina*cosb + sinb*cosa;

		pl.position.x *= Radius;
		pl.position.y *= Radius;

		auto density = pl.position.y/ Radius;
		leo::clamp(0.f, 1.f, density);


		pl.diffuse = leo::float4(density, density, density, 1.f);

		leo::DeviceMgr().GetDeviceContext()->UpdateSubresource(mPointLightPSCB, 0, nullptr, &pl, 0, 0);

		static auto mBegin = leo::clock::now();

		auto mRunTime = leo::clock::duration_to<>(leo::clock::now() - mBegin);
		mBegin = leo::clock::now();
		if (mRunTime < 1 / 30.f)
			std::this_thread::sleep_for(leo::clock::to_duration<>(1 / 30.f - mRunTime));
	}
}

void ComputeSSAO(ID3D11DeviceContext* context ) {
	ID3D11RenderTargetView* mMRTs[] = { leo::DeferredResources::GetInstance().GetSSAORTV(),nullptr };
	context->OMSetRenderTargets(2, mMRTs, nullptr);
	float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.8f };
	context->ClearRenderTargetView(mMRTs[0], ClearColor);

	context->PSSetShader(mSSAOPS, nullptr, 0);

	context->PSSetConstantBuffers(0, 1, &mSSAOPSCB);
	context->PSSetShaderResources(2, 1, &mSSAORandomVec);

	context->Draw(4, 0);
}

void DrawSSAO(ID3D11DeviceContext* context) {
	

	auto srv = leo::DeferredResources::GetInstance().GetSSAOSRV();
	context->PSSetShader(mGBufferPS, nullptr, 0);
	context->PSSetShaderResources(0, 1, &srv);
	context->Draw(4, 0);
}

void DrawLight(ID3D11DeviceContext* context) {

	context->PSSetShader(mPointLightPS, nullptr, 0);
	context->PSSetConstantBuffers(0, 1, &mPointLightPSCB);

	auto srv = leo::DeferredResources::GetInstance().GetSSAOSRV();
	context->PSSetShaderResources(2, 1, &srv);
	context->Draw(4, 0);
	srv = nullptr;
	context->PSSetShaderResources(2, 1, &srv);

}


void Render()
{
	event.Wait();

	while (renderThreadRun)
	{
		leo::DeviceMgr dm;

		
		leo::RenderSync::GetInstance()->Sync();
		std::lock_guard<std::mutex> lock(mRenderMutex);

		auto devicecontext = dm.GetDeviceContext();
		
#if 0
		//Build Shadow Map
	
		//leo::ShadowMap::GetInstance().BeginShadowMap(devicecontext,*pShaderCamera);
		if (renderAble)
			pModelMesh->CastShadow(devicecontext);

		//pTerrainMesh->CastShadow(devicecontext);
		//pBoxMesh->CastShadow(devicecontext);
		//pSphereMesh->CastShadow(devicecontext);

		//leo::ShadowMap::GetInstance().EndShadowMap(devicecontext);
#endif
		auto& defereed = leo::DeferredResources::GetInstance();
		
		
		defereed.OMSet();
		//pAxis->Render(devicecontext, *pCamera);
		//pTerrainMesh->Render(devicecontext, *pCamera);
		//pSphereMesh->Render(devicecontext, *pCamera);
		pBoxMesh->Render(devicecontext, *pCamera);

		defereed.IASet();

		D3D11_VIEWPORT prevVP;
		UINT num = 1;
		devicecontext->RSGetViewports(&num, &prevVP);
		D3D11_VIEWPORT currvp = prevVP;
		currvp.Height = prevVP.Height / 2;
		currvp.Width = prevVP.Width / 2;
		devicecontext->RSSetViewports(1, &currvp);
		ComputeSSAO(devicecontext);

		float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.8f };
		auto rtv = dm.GetRenderTargetView();
		devicecontext->OMSetRenderTargets(1, &rtv, nullptr);
		devicecontext->ClearRenderTargetView(rtv, ClearColor);

		
		//绘制延迟Buff
		{
			//右下,最终图像
			currvp.TopLeftX += currvp.Width;
			currvp.TopLeftY += currvp.Height;
			devicecontext->RSSetViewports(1, &currvp);
			DrawLight(devicecontext);

			//
			auto srvs = defereed.GetSRVs();
			devicecontext->PSSetShader(mGBufferPS, nullptr, 0);
			//左上,法线
			currvp.TopLeftX -= currvp.Width;
			currvp.TopLeftY -= currvp.Height;
			devicecontext->RSSetViewports(1, &currvp);
			devicecontext->PSSetShaderResources(0, 1, &srvs[0]);
			devicecontext->Draw(4, 0);

			//右上 绘制颜色
			currvp.TopLeftX += currvp.Width;
			devicecontext->RSSetViewports(1, &currvp);
			devicecontext->PSSetShaderResources(0,1,&srvs[1]);
			devicecontext->Draw(4, 0);


			//左下绘制SSAO
			currvp.TopLeftY += currvp.Height;
			currvp.TopLeftX -= currvp.Width;
			devicecontext->RSSetViewports(1, &currvp);

			DrawSSAO(devicecontext);


			leo::context_wrapper context(devicecontext);
		}

		defereed.UnIASet();
		devicecontext->RSSetViewports(1, &prevVP);


		leo::DeviceMgr().GetSwapChain()->Present(0, 0);

		leo::RenderSync::GetInstance()->Present();

	}
}
