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
#include <Core\Terrain.hpp>
#include <Core\\EngineConfig.h>
#include <Core\ShadowMap.hpp>
#include <Core\Vertex.hpp>
#include <Core\FileSearch.h>
#include <Core\BilateralFilter.hpp>
#include <Core\Light.hpp>

#include <TextureMgr.h>
#include <RenderSystem\ShaderMgr.h>
#include <RenderSystem\RenderStates.hpp>
#include <RenderSystem\DeferredRender.hpp>
#include <exception.hpp>
#include <Input.h>


#include "Axis.hpp"

#include "window.hpp"

#include "DeviceMgr.h"

#include <Commdlg.h>

#include <thread>
#include <atomic>
#include <mutex>
#include "resource.h"
#include <DirectXPackedVector.h>



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

ID3D11PixelShader* mSSAOPS = nullptr;

ID3D11Buffer* mSSAOPSCB = nullptr;
ID3D11ShaderResourceView* mSSAORandomVec = nullptr;

ID3D11ShaderResourceView* mBlurSSAOSRV = nullptr;
ID3D11UnorderedAccessView* mBlurSSAOUAV = nullptr;

ID3D11ShaderResourceView* mBlurSwapSSAOSRV = nullptr;
ID3D11UnorderedAccessView* mBlurSwapSSAOUAV = nullptr;

ID3D11ComputeShader* mBlurSSAOCS = nullptr;

ID3D11ComputeShader* mBlurVerSSAOCS = nullptr;
ID3D11ComputeShader* mBlurHorSSAOCS = nullptr;

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

const D3D11_INPUT_ELEMENT_DESC static mLightVolumeVertexElement_Desc[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
//set rt
//set blendstate
//set srv,set sample

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
public:
	PointLightVolume(ID3D11Device* device) {
		auto meshdata = leo::helper::CreateSphere(16,16);

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

	void SetLightParams(const leo::PointLight& params,ID3D11DeviceContext* context);
	void SetCamera(const leo::Camera& params, ID3D11DeviceContext* context);

	void Draw(ID3D11DeviceContext* context);
};


void DeviceEvent()
{
	while (!leo::DeviceMgr().GetDevice())
	{
		Sleep(0);
	}
	event.Set();
}


void LightPreable() {

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




#if 0
	auto mouseproc = [&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		auto x = (float)GET_X_LPARAM(lParam);
		auto y = (float)GET_Y_LPARAM(lParam);
		leo::float4x4 inv;
		leo::XMVECTOR temp;
		auto ray = leo::Ray::Pick(vp, proj, leo::float2(x, y));
		for (auto i = 0; i != 10;++i) {
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
	leo::ShaderMgr sm;
	auto  mPSBlob = sm.CreateBlob(leo::FileSearch::Search(L"PointLightPS.cso"));
	mPointLightPS = sm.CreatePixelShader(mPSBlob);
	mDirectionalLightPS = sm.CreatePixelShader(leo::FileSearch::Search(L"DirectionalLightPS.cso"));

	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DEFAULT;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = 0;
	Desc.MiscFlags = 0;
	Desc.StructureByteStride = 0;
	Desc.ByteWidth = sizeof(leo::PointLight);

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = &pl;
	subData.SysMemPitch = 0;
	subData.SysMemSlicePitch = 0;

	pl.Diffuse = leo::float4(0.8f, 0.8f, 0.8f, 1.f);
	const float Radius = 9;
	//world origin in view speace
	auto origin = pCamera->GetOrigin();
	origin.x = -origin.x;
	origin.y = -origin.y;
	origin.z = -origin.z;
	pl.PositionRange = leo::float4(origin, 18.f);

	leo::dxcall(device->CreateBuffer(&Desc, &subData, &mPointLightPSCB));

	Desc.ByteWidth = sizeof(leo::DirectionalLight);
	subData.pSysMem = &dl;
	dl.Directional = leo::float3(0.f, 1.f, 0.f);
	dl.Diffuse = leo::float3(0.8f, 0.8f, 0.8f);

	leo::dxcall(device->CreateBuffer(&Desc, &subData, &mDirectionalLightPSCB));

	auto rect = leo::CalcScissorRect(pl, *pCamera);

	auto clientRect = leo::dx::ScissorRectFromNDCRect(rect, leo::DeviceMgr().GetClientSize());
}
void ClearLight() {
	leo::win::ReleaseCOM(mPointLightPSCB);
	leo::win::ReleaseCOM(mDirectionalLightPSCB);
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

	//leo::DeferredResources::GetInstance().SetFrustum(*pCamera);
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
		float s = (rand()*1.f / RAND_MAX)*0.75f + 0.25f;
		leo::save(v, leo::Multiply(load(v), s));
	}

	float data[] = { 0.5f,0.f,0.f,0.f,
					0.f,-0.5f,0.f,0.f,
					0.f,0.f,1.f,0.f,
					0.5f,0.5f,0.f,1.f };
	leo::float4x4 toTex{ data };

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
			leo::float3 v(rand()*1.f / RAND_MAX, rand()*1.f / RAND_MAX, rand()*1.f / RAND_MAX);

			color[i * 256 + j] = DirectX::PackedVector::XMCOLOR(v.x, v.y, v.z, 0.0f);
		}
	}

	initData.pSysMem = color;

	ID3D11Texture2D* tex = 0;
	leo::dxcall(leo::DeviceMgr().GetDevice()->CreateTexture2D(&texDesc, &initData, &tex));

	leo::dxcall(leo::DeviceMgr().GetDevice()->CreateShaderResourceView(tex, 0, &mSSAORandomVec));

	// view saves a reference.
	leo::win::ReleaseCOM(tex);

	D3D11_TEXTURE2D_DESC SSAOTexDesc;
#ifdef DEBUG
	SSAOTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
#else
	SSAOTexDesc.Format = DXGI_FORMAT_R32_FLOAT;;
#endif
	SSAOTexDesc.ArraySize = 1;
	SSAOTexDesc.MipLevels = 1;

	SSAOTexDesc.SampleDesc.Count = 1;
	SSAOTexDesc.SampleDesc.Quality = 0;

	SSAOTexDesc.Width = size.first / 2;
	SSAOTexDesc.Height = size.second / 2;

	SSAOTexDesc.Usage = D3D11_USAGE_DEFAULT;
	SSAOTexDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	SSAOTexDesc.CPUAccessFlags = 0;
	SSAOTexDesc.MiscFlags = 0;

	using leo::win::make_scope_com;
	{
		auto mTex = make_scope_com<ID3D11Texture2D>();
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateTexture2D(&SSAOTexDesc, nullptr, &mTex));
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateShaderResourceView(mTex, nullptr, &mBlurSSAOSRV));
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateUnorderedAccessView(mTex, nullptr, &mBlurSSAOUAV));
	}
	{
		auto mTex = make_scope_com<ID3D11Texture2D>();
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateTexture2D(&SSAOTexDesc, nullptr, &mTex));
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateShaderResourceView(mTex, nullptr, &mBlurSwapSSAOSRV));
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateUnorderedAccessView(mTex, nullptr, &mBlurSwapSSAOUAV));
	}

	leo::CompilerBilaterCS(7, L"BilateralFilterCS.cso");
	leo::CompilerBilaterCS(7, size, L"BilateralFilterVerCS.cso", L"BilateralFilterHorCS.cso");
	auto mBlurCSBlob = sm.CreateBlob(leo::FileSearch::Search(L"BilateralFilterCS.cso"));

	mBlurSSAOCS = sm.CreateComputeShader(mBlurCSBlob);

	mBlurHorSSAOCS = sm.CreateComputeShader(leo::FileSearch::Search(L"BilateralFilterHorCS.cso"));
	mBlurVerSSAOCS = sm.CreateComputeShader(leo::FileSearch::Search(L"BilateralFilterVerCS.cso"));

	BuildLight(leo::DeviceMgr().GetDevice());
#endif

	//pTerrain = std::make_unique<leo::Terrain<>>(leo::DeviceMgr().GetDevice(), L"Resource/Test.Terrain");
	pRender = std::make_unique<leo::DeferredRender>(device,size);
}

void ClearRes() {
	leo::win::ReleaseCOM(mSSAOPSCB);

	pModelMesh.reset(nullptr);
	pTerrain.reset(nullptr);
	pRender.reset(nullptr);
	leo::win::ReleaseCOM(mSSAORandomVec);
	leo::win::ReleaseCOM(mBlurSSAOSRV);
	leo::win::ReleaseCOM(mBlurSSAOUAV);

	leo::win::ReleaseCOM(mBlurSwapSSAOSRV);
	leo::win::ReleaseCOM(mBlurSwapSSAOUAV);
	ClearLight();
}


void ReSize(std::pair<leo::uint16, leo::uint16> size) {
	//do many thing ,but 我不想写
	//改变AO资源的大小
	D3D11_TEXTURE2D_DESC SSAOTexDesc;
#ifdef DEBUG
	SSAOTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
#else
	SSAOTexDesc.Format = DXGI_FORMAT_R32_FLOAT;;
#endif
	SSAOTexDesc.ArraySize = 1;
	SSAOTexDesc.MipLevels = 1;

	SSAOTexDesc.SampleDesc.Count = 1;
	SSAOTexDesc.SampleDesc.Quality = 0;

	SSAOTexDesc.Width = size.first / 2;
	SSAOTexDesc.Height = size.second / 2;

	SSAOTexDesc.Usage = D3D11_USAGE_DEFAULT;
	SSAOTexDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	SSAOTexDesc.CPUAccessFlags = 0;
	SSAOTexDesc.MiscFlags = 0;

	using leo::win::make_scope_com;
	{
		auto mTex = make_scope_com<ID3D11Texture2D>();
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateTexture2D(&SSAOTexDesc, nullptr, &mTex));
		mBlurSSAOSRV->Release();
		mBlurSSAOUAV->Release();
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateShaderResourceView(mTex, nullptr, &mBlurSSAOSRV));
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateUnorderedAccessView(mTex, nullptr, &mBlurSSAOUAV));
	}
	{
		auto mTex = make_scope_com<ID3D11Texture2D>();
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateTexture2D(&SSAOTexDesc, nullptr, &mTex));
		mBlurSwapSSAOSRV->Release();
		mBlurSwapSSAOUAV->Release();
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateShaderResourceView(mTex, nullptr, &mBlurSwapSSAOSRV));
		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateUnorderedAccessView(mTex, nullptr, &mBlurSwapSSAOUAV));
	}

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
		auto cosa = pl.PositionRange.x / Radius;
		auto sina = pl.PositionRange.y / Radius;
		pl.PositionRange.x = cosa*cosb - sina*sinb;
		pl.PositionRange.y = sina*cosb + sinb*cosa;

		pl.PositionRange.x *= Radius;
		pl.PositionRange.y *= Radius;

		auto density = pl.PositionRange.y / Radius;
		leo::clamp(0.f, 1.f, density);


		pl.Diffuse = leo::float4(density, density, density, 1.f);

		//leo::DeviceMgr().GetDeviceContext()->UpdateSubresource(mPointLightPSCB, 0, nullptr, &pl, 0, 0);


		static auto mBegin = leo::clock::now();

		auto mRunTime = leo::clock::duration_to<>(leo::clock::now() - mBegin);
		mBegin = leo::clock::now();
		if (mRunTime < 1 / 30.f)
			std::this_thread::sleep_for(leo::clock::to_duration<>(1 / 30.f - mRunTime));
	}
}

void ComputeSSAO(ID3D11DeviceContext* context) {
	ID3D11RenderTargetView* mMRTs[] = { nullptr,nullptr };
	context->OMSetRenderTargets(2, mMRTs, nullptr);
	float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.8f };
	context->ClearRenderTargetView(mMRTs[0], ClearColor);

	context->PSSetShader(mSSAOPS, nullptr, 0);

	context->PSSetConstantBuffers(0, 1, &mSSAOPSCB);
	context->PSSetShaderResources(2, 1, &mSSAORandomVec);

	context->Draw(4, 0);
}

void BlurSSAO(ID3D11DeviceContext* context, unsigned width, unsigned height) {
	ID3D11ShaderResourceView* srv = nullptr;

	context->CSSetShader(mBlurHorSSAOCS, nullptr, 0);
	context->CSSetShaderResources(0, 1, &srv);
	context->CSSetUnorderedAccessViews(0, 1, &mBlurSwapSSAOUAV, nullptr);//swapUAV

	context->Dispatch(width, 1, 1);

	ID3D11UnorderedAccessView* mUAV = nullptr;
	ID3D11ShaderResourceView* mSRV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &mUAV, nullptr);
	context->CSSetShaderResources(0, 1, &mSRV);

	context->CSSetShader(mBlurVerSSAOCS, nullptr, 0);
	context->CSSetShaderResources(0, 1, &mBlurSwapSSAOSRV);//swapSRV
	context->CSSetUnorderedAccessViews(0, 1, &mBlurSSAOUAV, nullptr);
	context->Dispatch(1, height, 1);
	context->CSSetUnorderedAccessViews(0, 1, &mUAV, nullptr);
	context->CSSetShaderResources(0, 1, &mSRV);



	using leo::win::make_scope_com;

	auto mSSAORes = make_scope_com<ID3D11Resource>();
	auto mBlurSSAORes = make_scope_com<ID3D11Resource>();
	srv->GetResource(&mSSAORes);
	mBlurSSAOSRV->GetResource(&mBlurSSAORes);
	context->CopyResource(mSSAORes, mBlurSSAORes);

}

void DrawSSAO(ID3D11DeviceContext* context) {


	ID3D11ShaderResourceView* srv = nullptr;

	context->PSSetShader(mGBufferPS, nullptr, 0);
	context->PSSetShaderResources(0, 1, &srv);
	context->Draw(4, 0);
}

void DrawLight(ID3D11DeviceContext* context) {

	context->PSSetShader(mDirectionalLightPS, nullptr, 0);
	context->PSSetConstantBuffers(0, 1, &mDirectionalLightPSCB);

	ID3D11ShaderResourceView* srv = nullptr;

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
		/*
		auto& defereed = leo::DeferredResources::GetInstance();


		defereed.OMSet();

		pTerrain->Render(devicecontext, *pCamera);

		defereed.IASet();

		D3D11_VIEWPORT prevVP;
		UINT num = 1;
		devicecontext->RSGetViewports(&num, &prevVP);
		D3D11_VIEWPORT currvp = prevVP;
		currvp.Height = prevVP.Height / 2;
		currvp.Width = prevVP.Width / 2;
		devicecontext->RSSetViewports(1, &currvp);
		ComputeSSAO(devicecontext);
		*/
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
		//Light Pass
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

		defereed.UnIASet();
		devicecontext->RSSetViewports(1, &prevVP);
		*/

		leo::DeviceMgr().GetSwapChain()->Present(0, 0);

		leo::RenderSync::GetInstance()->Present();

	}
}
