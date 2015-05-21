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
#include <Input.h>//to core!
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
std::unique_ptr<leo::Mesh> pModelMesh = nullptr;
std::unique_ptr<leo::UVNCamera> pCamera = nullptr;
std::unique_ptr<leo::CastShadowCamera> pShaderCamera;
std::unique_ptr<leo::DeferredRender> pRender = nullptr;
std::unique_ptr<leo::Terrain<>> pTerrain = nullptr;

std::atomic<bool> renderAble = false;
std::atomic<bool> renderThreadRun = true;

std::mutex mSizeMutex;
std::mutex mRenderMutex;

//用于显示GBuffer
ID3D11PixelShader* mGBufferPS = nullptr;

const D3D11_INPUT_ELEMENT_DESC static mLightVolumeVertexElement_Desc[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
//set rt
//set blendstate
//set srv,set sample

ID3D11SamplerState* mSamPoint = nullptr;

class PointLightVolume : public leo::DataAllocatedObject<leo::GeneralAllocPolicy>{
	ID3D11InputLayout* mLightVolumeVertexLayout = nullptr;
	

	ID3D11VertexShader* mLightVolumeVS = nullptr;

	struct TransfromMatrix {
		std::array<__m128, 4> WorldView;
		std::array<__m128, 4> Proj;
	} mVSCBParams;
	leo::win::unique_com<ID3D11Buffer>  mVSCB = nullptr;

	ID3D11PixelShader* mPointLightVolumePS = nullptr;

	leo::PointLight mPSCBParams;
	leo::win::unique_com<ID3D11Buffer>  mPSCB = nullptr;

	leo::win::unique_com<ID3D11Buffer> mPointVolumeVB = nullptr;
	leo::win::unique_com<ID3D11Buffer>  mPointVolumeIB = nullptr;

	UINT mIndexCount = 0;
public:
	PointLightVolume(ID3D11Device* device) {
		auto meshdata = leo::helper::CreateSphere(16,16);
		mIndexCount = meshdata.Indices.size();
		CD3D11_BUFFER_DESC vbDesc{ meshdata.Vertices.size()*sizeof(decltype(meshdata.Vertices)::value_type),
		D3D11_BIND_VERTEX_BUFFER,D3D11_USAGE_IMMUTABLE };

		D3D11_SUBRESOURCE_DATA resDesc;
		resDesc.pSysMem = &meshdata.Vertices[0];
		leo::dxcall(device->CreateBuffer(&vbDesc, &resDesc, &mPointVolumeVB));

		CD3D11_BUFFER_DESC ibDesc{ vbDesc };
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibDesc.ByteWidth = static_cast<leo::win::UINT> (sizeof(std::uint32_t)*meshdata.Indices.size());
		resDesc.pSysMem = &meshdata.Indices[0];
		leo::dxcall(device->CreateBuffer(&ibDesc, &resDesc, &mPointVolumeIB));

		CD3D11_BUFFER_DESC vscbDesc{sizeof(TransfromMatrix),D3D11_BIND_CONSTANT_BUFFER};

		leo::dxcall(device->CreateBuffer(&vscbDesc, nullptr, &mVSCB));

		CD3D11_BUFFER_DESC pscbDesc{ sizeof(leo::PointLight),D3D11_BIND_CONSTANT_BUFFER };

		leo::dxcall(device->CreateBuffer(&pscbDesc,nullptr,&mPSCB));

		leo::ShaderMgr sm;

		mLightVolumeVS = sm.CreateVertexShader(
			leo::FileSearch::Search(
				leo::EngineConfig::ShaderConfig::GetShaderFileName(L"pointlight", D3D11_VERTEX_SHADER)
				),
			nullptr,
			mLightVolumeVertexElement_Desc,
			leo::arrlen(mLightVolumeVertexElement_Desc),
			&mLightVolumeVertexLayout);

		mPointLightVolumePS = sm.CreatePixelShader(
			leo::FileSearch::Search(
				leo::EngineConfig::ShaderConfig::GetShaderFileName(L"pointlight", D3D11_PIXEL_SHADER)
				));
	}

	void SetLightParams(const leo::PointLight& params) {
		mPSCBParams = params;
	}
	void SetCamera(const leo::Camera& params) {
		leo::SQT scale{};
		scale.s = mPSCBParams.PositionRange.w;
		auto world = scale.operator std::array<__m128, 4U>();
		mVSCBParams.WorldView = leo::Transpose(leo::Multiply(world, load(params.View())));
		mVSCBParams.Proj = leo::Transpose(load(params.Proj()));
	}

	void Draw(ID3D11DeviceContext* context) {
		context->UpdateSubresource(mVSCB, 0, nullptr, &mVSCBParams, 0, 0);
		context->UpdateSubresource(mPSCB, 0, nullptr, &mPSCBParams, 0, 0);

		UINT strides[] = { sizeof(leo::float3) };
		UINT offsets[] = { 0 };

		context->IASetVertexBuffers(0, 1, &mPointVolumeVB,strides,offsets );
		context->IASetIndexBuffer(mPointVolumeIB, DXGI_FORMAT_R32_UINT, 0);
		context->IASetInputLayout(mLightVolumeVertexLayout);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->VSSetShader(mLightVolumeVS, nullptr, 0);
		context->VSSetConstantBuffers(0, 1, &mVSCB);
		context->PSSetShader(mPointLightVolumePS, nullptr, 0);
		context->PSSetConstantBuffers(0, 1, &mPSCB);
		auto rtv = pRender->GetLightRTV();
		context->OMSetRenderTargets(1, &rtv, nullptr);
		//忽略模板测试,忽略模板
		ID3D11ShaderResourceView* srvs[] = { pRender->GetLinearDepthSRV(),pRender->GetNormalAlphaSRV() };
		context->PSSetSamplers(0, 1, &mSamPoint);
		context->PSSetShaderResources(0, 2, srvs);

		context->DrawIndexed(mIndexCount, 0, 0);
		srvs[0] = nullptr;srvs[1] = nullptr;
		context->PSSetShaderResources(0, 2, srvs);

	}
};
std::unique_ptr<PointLightVolume> pPointLightVolueme = nullptr;
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
	_CrtSetBreakAlloc(309);
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	leo::EngineConfig::Read(L"config.scheme");
	leo::EngineConfig::ShaderConfig::GetAllBlendStateName();


	leo::DeviceMgr DeviceMgr;
	leo::OutputWindow win;

	auto clientSize = leo::EngineConfig::ClientSize();
	if (!win.Create(GetModuleHandle(nullptr), clientSize, L"Model LooK",
		WS_BORDER | WS_SIZEBOX, 0,
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


ID3D11PixelShader* mLinearizeDepthPS = nullptr;


void BuildQuad(ID3D11Device* device, const leo::CameraFrustum& frustum) {
	
	using namespace leo;

	ShaderMgr sm;

	mLinearizeDepthPS = sm.CreatePixelShader(
		FileSearch::Search(L"LinearizeDepthPS.cso")
		);
		leo::RenderStates sss;
	mSamPoint = sss.GetSamplerState(L"NearestClamp");
}
void ClearQuad() {
}

void BuildLight(ID3D11Device* device) {
	pPointLightVolueme = std::make_unique<PointLightVolume>(device);
	leo::PointLight pl;
	pl.Diffuse = leo::float3(0.8f, 0.7f, 0.6f);
	pl.PositionRange = leo::float4(0.f, 0.f, 0.f, 10.f);
	pPointLightVolueme->SetLightParams(pl);
	pPointLightVolueme->SetCamera(*pCamera);
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
	//leo::EffectNormalLine::GetInstance(leo::DeviceMgr().GetDevice());
	//leo::ShadowMap::GetInstance(leo::DeviceMgr().GetDevice(), std::make_pair(2048u, 2048u));
	//leo::EffectPack::GetInstance(leo::DeviceMgr().GetDevice());

	//leo::EffectShadowMap::GetInstance(leo::DeviceMgr().GetDevice());
	//leo::EffectLine::GetInstance(leo::DeviceMgr().GetDevice());

	//leo::DeferredResources::GetInstance();
	leo::EffectGBuffer::GetInstance(leo::DeviceMgr().GetDevice());
	leo::EffectQuad::GetInstance(device);
	//leo::EffectTerrain::GetInstance(leo::DeviceMgr().GetDevice());
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

	pCamera->LookAt(float3(0.f, 10.f, -10.f), float3(0.f, 0.f, 0.f), float3(0.f, 1.f, 0.f));
	pCamera->SetFrustum(leo::default_param::frustum_fov, leo::DeviceMgr().GetAspect(), leo::default_param::frustum_near, leo::default_param::frustum_far);

	leo::EffectQuad::GetInstance().SetFrustum(device, *pCamera);
	//leo::DeferredResources::GetInstance().SetFrustum(*pCamera);
#endif
	leo::ShaderMgr sm;
	auto mGBufferBlob = sm.CreateBlob(leo::FileSearch::Search(L"GBufferPS.cso"));

	mGBufferPS = sm.CreatePixelShader(mGBufferBlob);

	BuildLight(leo::DeviceMgr().GetDevice());

	BuildQuad(device, *pCamera);

	//pTerrain = std::make_unique<leo::Terrain<>>(leo::DeviceMgr().GetDevice(), L"Resource/Test.Terrain");
	pRender = std::make_unique<leo::DeferredRender>(device,size);
}

void ClearRes() {

	pModelMesh.reset(nullptr);
	pTerrain.reset(nullptr);
	pRender.reset(nullptr);
	pPointLightVolueme.reset(nullptr);
	
	ClearLight();
	ClearQuad();
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

		//cos(a+b) = cos(a) cos(b) - sin(a) sin(b) 
		//sin(a + b) = sin(a) cos(b) + sin(b) cos(a)
		//cosa = x ,sina = y
		//sinb = sin(pi/180)
		

		//leo::DeviceMgr().GetDeviceContext()->UpdateSubresource(mPointLightPSCB, 0, nullptr, &pl, 0, 0);


		static auto mBegin = leo::clock::now();

		auto mRunTime = leo::clock::duration_to<>(leo::clock::now() - mBegin);
		mBegin = leo::clock::now();
		if (mRunTime < 1 / 30.f)
			std::this_thread::sleep_for(leo::clock::to_duration<>(1 / 30.f - mRunTime));
	}
}

void DrawSSAO(ID3D11DeviceContext* context) {


	ID3D11ShaderResourceView* srv = nullptr;

	context->PSSetShader(mGBufferPS, nullptr, 0);
	context->PSSetShaderResources(0, 1, &srv);
	context->Draw(4, 0);
}


void LightPreable(ID3D11DeviceContext* context) {
}

void Render()
{
	event.Wait();

	while (renderThreadRun)
	{
		leo::DeviceMgr dm;

		pCamera->UpdateViewMatrix();

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
		float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.8f };
		auto rtv = dm.GetRenderTargetView();
		devicecontext->OMSetRenderTargets(1, &rtv, nullptr);
		devicecontext->ClearRenderTargetView(rtv, ClearColor);

		if (pRender) {
			pRender->OMSet(devicecontext,*leo::global::globalDepthStencil);
		}
		if (pModelMesh) {
			pModelMesh->Render(devicecontext, *pCamera);
		}
		if (pRender) {
			pRender->LinearizeDepth(devicecontext, *leo::global::globalDepthStencil, pCamera->mNear, pCamera->mFar);
		}
		//Light Pass
		LightPreable(devicecontext);
		pPointLightVolueme->SetCamera(*pCamera);
		pPointLightVolueme->Draw(devicecontext);
		//Shader Pass
		if (pRender) {
			pRender->UnBind(devicecontext, *leo::global::globalDepthStencil);
		}
		/*
		BlurSSAO(devicecontext, unsigned int(prevVP.Width), unsigned int(prevVP.Height));
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
			devicecontext->PSSetShaderResources(0, 1, &srvs[1]);
			devicecontext->Draw(4, 0);


			//左下绘制SSAO
			currvp.TopLeftY += currvp.Height;
			currvp.TopLeftX -= currvp.Width;
			devicecontext->RSSetViewports(1, &currvp);

			DrawSSAO(devicecontext);


		}
		*/

		leo::DeviceMgr().GetSwapChain()->Present(0, 0);

		leo::RenderSync::GetInstance()->Present();

	}
}
