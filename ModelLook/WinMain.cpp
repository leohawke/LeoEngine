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
#include <Core\Geometry.hpp>
#include <Core\RenderSync.hpp>
#include <Core\EffectLine.hpp>
#include <Core\Terrain.Tessation.Effect.hpp>
#include <Core\Terrain.TileRing.h>
#include <COM.hpp>

#include <TextureMgr.h>
#include <ShaderMgr.h>
#include <RenderStates.hpp>
#include <exception.hpp>


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

std::atomic<bool> renderAble = false;
std::atomic<bool> renderThreadRun = true;

std::mutex mSizeMutex;

float invNonlinearCameraSpeed(float);
const int MAX_OCTAVES = 15;
const int mDefaultRidgeOctaves = 3;			// This many ridge octaves is good for the moon - rather jagged.
const int mDefaultfBmOctaves = 3;
const int mDefaultTexTwistOctaves = 1;
const int mDefaultDetailNoiseScale = 20;
int mRidgeOctaves = mDefaultRidgeOctaves;
int mfBmOctaves = mDefaultfBmOctaves;
int mTexTwistOctaves = mDefaultTexTwistOctaves;
int mDetailNoiseScale = mDefaultDetailNoiseScale;
float mCameraSpeed = invNonlinearCameraSpeed(100);
int mPatchCount = 0;
bool mHwTessellation = true;
int mtessellatedTriWidth = 6;	// pixels on a triangle edge

const float CLIP_NEAR = 1, CLIP_FAR = 20000;
leo::float2 mScreenSize(800.f, 600.f);
const int COARSE_HEIGHT_MAP_SIZE = 1024;
const float WORLD_SCALE = 400;
const int VTX_PER_TILE_EDGE = 9;				// overlap => -2
const int TRI_STRIP_INDEX_COUNT = (VTX_PER_TILE_EDGE - 1) * (2 * VTX_PER_TILE_EDGE + 2);
const int QUAD_LIST_INDEX_COUNT = (VTX_PER_TILE_EDGE - 1) * (VTX_PER_TILE_EDGE - 1) * 4;
const int MAX_RINGS = 10;
int nRings = 0;
std::unique_ptr<leo::TileRing> mTileRings[MAX_RINGS];
float SNAP_GRID_SIZE = 0;

ID3D11ShaderResourceView* mHeightMapSRV = nullptr;
ID3D11RenderTargetView* mHeightMapRTV = nullptr;
ID3D11ShaderResourceView* mGradientMapSRV = nullptr;
ID3D11RenderTargetView* mGradientMapRTV = nullptr;
struct CommonParamsOnSet
{
	leo::XMFLOAT3 gTextureWorldOffset;	// Offset of fractal terrain in texture space.
	float     gDetailNoiseScale = 0.2;
	leo::XMFLOAT2    gDetailUVScale = leo::XMFLOAT2(1.f, 1.f);				// x is scale; y is 1/scale
	float  gCoarseSampleSpacing;
	float gfDisplacementHeight;
	const static leo::uint8 slot = 0;
};
leo::Effect::ShaderConstantBuffer<CommonParamsOnSet>* mpCBParams;
ID3D11Buffer* mTriStripIB = nullptr;
ID3D11Buffer* mQuadListIB = nullptr;
void ReleaseRes()
{
	leo::win::ReleaseCOM(mHeightMapSRV);
	leo::win::ReleaseCOM(mHeightMapRTV);
	leo::win::ReleaseCOM(mGradientMapSRV);
	leo::win::ReleaseCOM(mGradientMapRTV);
	leo::win::ReleaseCOM(mTriStripIB);
	leo::win::ReleaseCOM(mQuadListIB);

	for (auto & tile : mTileRings)
		tile.reset(nullptr);

	delete mpCBParams;
}

static void CreateAmplifiedHeights(ID3D11Device* device);
static void InitializeHeights(ID3D11DeviceContext* pContext)
{
	auto eye = pCamera->GetOrigin();
	eye.y = 0.f;
	if (SNAP_GRID_SIZE > 0)
	{
		eye.x = floorf(eye.x / SNAP_GRID_SIZE) * SNAP_GRID_SIZE;
		eye.z = floorf(eye.z / SNAP_GRID_SIZE)*SNAP_GRID_SIZE;
	}
	eye.x /= WORLD_SCALE;
	eye.y /= WORLD_SCALE;
	eye.z /= WORLD_SCALE;
	eye.z *= -1;
	mpCBParams->gTextureWorldOffset = leo::float3(eye.x, eye.y, eye.z);

	{
		static const D3D11_VIEWPORT vp = { 0, 0, COARSE_HEIGHT_MAP_SIZE, COARSE_HEIGHT_MAP_SIZE, 0.f, 1.f };
		leo::context_wrapper context(pContext);
		context.VSSetShader(leo::ShaderMgr().CreateVertexShader(L"Shader\\TerrainHeightVS.cso"), nullptr, 0);
		context.PSSetShader(leo::ShaderMgr().CreatePixelShader(L"Shader\\TerrainHeightPS.cso"), nullptr, 0);
		context.OMSetDepthStencilState(leo::RenderStates().GetDepthStencilState(L"NoDepthDSS"),0);
		context.RSSetState(leo::RenderStates().GetRasterizerState(L"NoCullRS"));
		mpCBParams->Update(context);
		context.PSSetConstantBuffers(0, 1, &mpCBParams->mBuffer);
		context.RSSetViewports(1, &vp);
		context->IASetInputLayout(nullptr);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		context.OMSetRenderTargets(1, &mHeightMapRTV, nullptr);
		context->Draw(4, 0);
	}
	{
		//do nothing,bug fix!
		leo::context_wrapper context(pContext);
	}
	{
		static const D3D11_VIEWPORT vp = { 0, 0, COARSE_HEIGHT_MAP_SIZE, COARSE_HEIGHT_MAP_SIZE, 0.f, 1.f };
		leo::context_wrapper context(pContext);
		auto LinearClamp = leo::RenderStates().GetSamplerState(L"LinearClamp");
		context.PSSetShaderResources(0, 1, &mHeightMapSRV);
		context.PSSetSamplers(0, 1, &LinearClamp);
		context.PSSetShader(leo::ShaderMgr().CreatePixelShader(L"Shader\\TerrainGradHeightPS.cso"), nullptr, 0);
		context.RSSetViewports(1, &vp);
		context.OMSetDepthStencilState(leo::RenderStates().GetDepthStencilState(L"NoDepthDSS"), 0);
		context.OMSetRenderTargets(1, &mGradientMapRTV, nullptr);
		context->Draw(4, 0);
	}
	{
		//do nothing,bug fix!
		leo::context_wrapper context(pContext);
	}
}

void DeviceEvent()
{
	while (!leo::DeviceMgr().GetDevice())
	{
		Sleep(0);
	}
	event.Set();
}

void TerrainRender(ID3D11DeviceContext* context,leo::Camera* pCamera);

void Render();

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
	leo::DeviceMgr DeviceMgr;
	leo::OutputWindow win;

	auto clientSize = std::make_pair<leo::uint16,leo::uint16>(800, 600);
	if (!win.Create(GetModuleHandle(nullptr), clientSize, L"Model LooK", 
		WS_BORDER | WS_SIZEBOX,0,
		MAKEINTRESOURCE(IDI_ICON1)))
	{
		return 0;
	}

	HACCEL hAccel = LoadAccelerators(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDR_ACCELERATOR1));


	

	DeviceMgr.CreateDevice(false, clientSize);

	auto mNeedDuration = leo::Clock::ToDuration(1.f);
	auto mHasDuration = leo::Clock::ToDuration(0.f);
	leo::Clock mSizeClock;
	auto sizeproc = [&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		leo::RenderSync::Block block;
		if (DeviceMgr.GetDevice())
		{
			if (mHasDuration > mNeedDuration)
			{
				auto size = std::make_pair<int, int>(LOWORD(lParam), HIWORD(lParam));
				DeviceMgr.ReSize(size);
				mHasDuration = leo::Clock::ToDuration(0.f);
			}
			else
			{
				mSizeClock.Update();
				mHasDuration += mSizeClock.GetDelta();
			}
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	};

	win.BindMsgFunc(WM_SIZE, sizeproc);

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
			if (pMesh){
				pMesh->Roll(-1.f);
			}
			break;
		case ID_ROLL_RIGHT:
			if (pMesh){
				pMesh->Roll(+1.f);
			}
			break;
		case ID_YAW_FRONT:
			if (pMesh){
				pMesh->Yaw(-1.f);
			}
			break;
		case ID_YAW_BACK:
			if (pMesh){
				pMesh->Yaw(+1.f);
			}
			break;
		case ID_PITCH_UP:
			if (pMesh){
				pMesh->Pitch(-1.f);
			}
			break;
		case ID_PITCH_DOWN:
			if (pMesh){
				pMesh->Pitch(+1.f);
			}
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

	DeviceEvent();
	BuildRes();

	std::thread renderThread(Render);

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
	renderThread.join();

	pMesh.reset(nullptr);

	ReleaseRes();
	leo::global::Destroy();
#ifdef DEBUG
	leo::SingletonManger::GetInstance()->PrintAllSingletonInfo();
#endif
	leo::SingletonManger::GetInstance()->UnInstallAllSingleton();
	return 0;
}

void BuildRes()
{
	pCamera = std::make_unique<leo::Camera>();

	using leo::float3;

	auto Eye = float3(0.f,0.f,0.f);
	auto At = float3(0.f,0.f,1.f);
	auto Up = float3(0.f,1.f, 0.f);

	pCamera->LookAt(Eye, At, Up);

	event.Wait();

	pCamera->SetFrustum(leo::def::frustum_fov, leo::DeviceMgr().GetAspect(), leo::def::frustum_near, leo::def::frustum_far);
	pCamera->SetFrustum(leo::PROJECTION_TYPE::PERSPECTIVE);

	auto& pEffect =  leo::EffectNormalMap::GetInstance(leo::DeviceMgr().GetDevice());
	

	leo::DirectionLight dirlight;
	dirlight.ambient = leo::float4(1.f, 1.f, 1.f, 1.f);
	dirlight.diffuse = leo::float4(1.f, 1.f, 1.f, 1.f);
	dirlight.specular = leo::float4(1.f, 1.f, 1.f, 32.f);
	dirlight.dir = leo::float4(0.0f, 1.0f, 5.0f, 0.f);

	pEffect->Light(dirlight);

	leo::EffectNormalLine::GetInstance(leo::DeviceMgr().GetDevice());
	leo::EffectLine::GetInstance(leo::DeviceMgr().GetDevice());
	leo::Axis::GetInstance(leo::DeviceMgr().GetDevice());

	leo::DeviceMgr().GetDeviceContext()->RSSetState(leo::RenderStates().GetRasterizerState(L"WireframeRS"));

	auto& mTessation = leo::TerrainTessationEffect::GetInstance(leo::DeviceMgr().GetDevice());

	leo::TextureMgr tm;

	
	mTessation->SetTerrainColorTextures(tm.LoadTextureSRV(L"Resource\\TerrainTessellation\\LunarSurface1.dds"), tm.LoadTextureSRV(L"Resource\\TerrainTessellation\\LunarMicroDetail1.dds"));
	mTessation->SetNoiseTexture(tm.LoadTextureSRV(L"Resource\\GaussianNoise256.jpg"));
	mTessation->SetDetailNoiseTexture(tm.LoadTextureSRV(L"Resource\\fBm5Octaves.dds"));
	mTessation->SetDetailNoiseGradTexture(tm.LoadTextureSRV(L"Resource\\fBm5OctavesGrad.dds"));
	
	CreateAmplifiedHeights(leo::DeviceMgr().GetDevice());

#ifdef DEBUG
	//hardware
	mTessation->SetTriWidth(2 * mtessellatedTriWidth, nullptr);

	mTessation->SetShowTiles(false, nullptr);
	mTessation->SetDebugShowPatches(false, nullptr);
#endif

	mTessation->SetDetailNoiseScale(0.001f*mDetailNoiseScale,nullptr);
	mTessation->SetCoarseSampleSpacing(WORLD_SCALE * mTileRings[nRings - 1]->outerWidth() / COARSE_HEIGHT_MAP_SIZE, nullptr);

	mTessation->SetFractalOctaves(leo::float3(mRidgeOctaves*1.f, mfBmOctaves*1.f, mTexTwistOctaves*1.f), nullptr);
	const float DETAIL_UV_SCALE = std::powf(2.f, std::max(mRidgeOctaves, mTexTwistOctaves) + mfBmOctaves - 4.f);
	mTessation->SetDetailUVScale(leo::float2(DETAIL_UV_SCALE, 1.f / DETAIL_UV_SCALE), nullptr);
	
	//TileRing
	int widths[] = { 0, 16, 16, 16, 16 };
	nRings = sizeof(widths) / sizeof(widths[0]) - 1;
	assert(nRings < MAX_RINGS);

	float tileWidth = 0.125f;
	for (auto i = 0; i != nRings; ++i)
	{
		mTileRings[i] = std::make_unique<leo::TileRing>(leo::DeviceMgr().GetDevice(), widths[i] / 2, widths[i + 1], tileWidth);
		tileWidth *= 2.f;
	}

	const int PATCHES_PER_TILE_EDGE = VTX_PER_TILE_EDGE - 1;
	SNAP_GRID_SIZE = WORLD_SCALE * mTileRings[nRings - 1]->tileSize() / PATCHES_PER_TILE_EDGE;
	//CreateTriIB
	{
		D3D11_SUBRESOURCE_DATA initData;

		auto index = 0;
		auto pIndices = std::make_unique<unsigned long[]>(TRI_STRIP_INDEX_COUNT);

		for (auto y = 0; y != VTX_PER_TILE_EDGE - 1; ++y)
		{
			const int rowStart = y*VTX_PER_TILE_EDGE;
			for (auto x = 0; x != VTX_PER_TILE_EDGE; ++x)
			{
				pIndices[index++] = rowStart + x;
				pIndices[index++] = rowStart + x + VTX_PER_TILE_EDGE;
			}

			pIndices[index] = pIndices[index - 1];
			++index;
			pIndices[index++] = rowStart + VTX_PER_TILE_EDGE;
		}
		assert(index == TRI_STRIP_INDEX_COUNT);

		initData.pSysMem = pIndices.get();
		D3D11_BUFFER_DESC iBufferDesc =
		{
			sizeof(unsigned long) * TRI_STRIP_INDEX_COUNT,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_INDEX_BUFFER,
			0, 0
		};

		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateBuffer(&iBufferDesc, &initData, &mTriStripIB));
		leo::dx::DebugCOM(mTriStripIB, L"mTriStripIB");
	}
	{
		D3D11_SUBRESOURCE_DATA initData;

		auto index = 0;
		auto pIndices = std::make_unique<unsigned long[]>(QUAD_LIST_INDEX_COUNT);

		for (auto y = 0; y != VTX_PER_TILE_EDGE - 1; ++y)
		{
			const int rowStart = y*VTX_PER_TILE_EDGE;
			for (auto x = 0; x != VTX_PER_TILE_EDGE-1; ++x)
			{
				pIndices[index++] = rowStart + x;
				pIndices[index++] = rowStart + x + VTX_PER_TILE_EDGE;
				pIndices[index++] = rowStart + x + VTX_PER_TILE_EDGE + 1;
				pIndices[index++] = rowStart + x + 1;
			}
		}
		assert(index == QUAD_LIST_INDEX_COUNT);

		initData.pSysMem = pIndices.get();
		D3D11_BUFFER_DESC iBufferDesc =
		{
			sizeof(unsigned long) * QUAD_LIST_INDEX_COUNT,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_INDEX_BUFFER,
			0, 0
		};

		leo::dxcall(leo::DeviceMgr().GetDevice()->CreateBuffer(&iBufferDesc, &initData, &mQuadListIB));
		leo::dx::DebugCOM(mQuadListIB, L"mQuadListIB");
	}
}

static void CreateAmplifiedHeights(ID3D11Device* device)
{
	CD3D11_TEXTURE2D_DESC desc;
	desc.Width = COARSE_HEIGHT_MAP_SIZE;
	desc.Height = COARSE_HEIGHT_MAP_SIZE;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	CD3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	SRVDesc.Format = desc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	SRVDesc.Texture2D.MostDetailedMip = 0;

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format = desc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;

	// No initial data here - it's initialized by deformation.
	ID3D11Texture2D* pTex = nullptr;
	leo::dxcall(device->CreateTexture2D(&desc, nullptr, &pTex));

	
	leo::dxcall(device->CreateShaderResourceView(pTex, &SRVDesc, &mHeightMapSRV));
	leo::dxcall(device->CreateRenderTargetView(pTex, &RTVDesc, &mHeightMapRTV));
	leo::dx::DebugCOM(mHeightMapRTV, L"mHeightMapRTV");
	leo::dx::DebugCOM(mHeightMapSRV, L"mHeightMapSRV");
	leo::win::ReleaseCOM(pTex);

	desc.Format = DXGI_FORMAT_R16G16_FLOAT;

	// No initial data here - it's initialized by deformation.
	leo::dxcall(device->CreateTexture2D(&desc, nullptr, &pTex));

	
	SRVDesc.Format = RTVDesc.Format = desc.Format;
	leo::dxcall(device->CreateShaderResourceView(pTex, &SRVDesc, &mGradientMapSRV));
	leo::dxcall(device->CreateRenderTargetView(pTex, &RTVDesc, &mGradientMapRTV));
	leo::dx::DebugCOM(mGradientMapSRV, L"mGradientMapSRV");
	leo::dx::DebugCOM(mGradientMapRTV, L"mGradientMapRTV");
	leo::win::ReleaseCOM(pTex);

	mpCBParams = new leo::Effect::ShaderConstantBuffer<CommonParamsOnSet>(device);
}

void Render()
{
	event.Wait();
	while (renderThreadRun)
	{
		leo::DeviceMgr dm;

		if (GetAsyncKeyState('W') & 0X8000)
			pCamera->Walk(+0.2f);

		if (GetAsyncKeyState('S') & 0X8000)
			pCamera->Walk(-0.2f);

		if (GetAsyncKeyState('A') & 0X8000)
			pCamera->Strafe(-0.2f);

		if (GetAsyncKeyState('D') & 0X8000)
			pCamera->Strafe(+0.2f);

		pCamera->UpdateViewMatrix();

		leo::RenderSync::GetInstance()->Sync();

		auto devicecontext = dm.GetDeviceContext();
		float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.8f };
		devicecontext->ClearRenderTargetView(dm.GetRenderTargetView(), ClearColor);
		devicecontext->ClearDepthStencilView(dm.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

		if (renderAble)
			pMesh->Render(devicecontext, *pCamera);
		
		leo::Axis::GetInstance()->Render(devicecontext, *pCamera);

		TerrainRender(devicecontext, pCamera.get());

		leo::DeviceMgr().GetSwapChain()->Present(0, 0);

		leo::RenderSync::GetInstance()->Present();

		std::this_thread::sleep_for(std::chrono::milliseconds(0));
	}
}

void TerrainRender(ID3D11DeviceContext* con, leo::Camera* pCamera)
{
	static bool b = false;
	leo::call_once(b, InitializeHeights, con);

	auto& mTessation = leo::TerrainTessationEffect::GetInstance(leo::DeviceMgr().GetDevice());

	mTessation->SetCoarseHeightMap(mHeightMapSRV);
	mTessation->SetCoarseGradientMap(mGradientMapSRV);
	{
		auto eye = pCamera->GetOrigin();
		eye.y = 0.f;
		if (SNAP_GRID_SIZE > 0)
		{
			eye.x = floorf(eye.x / SNAP_GRID_SIZE) * SNAP_GRID_SIZE;
			eye.z = floorf(eye.z / SNAP_GRID_SIZE)*SNAP_GRID_SIZE;
		}
		eye.x /= WORLD_SCALE;
		eye.y /= WORLD_SCALE;
		eye.z /= WORLD_SCALE;
		eye.z *= -1;
		mTessation->SetTexureOffset(leo::float3(eye.x, eye.y, eye.z), nullptr);
		
		
		

		
		auto mScale = leo::XMMatrixScaling(WORLD_SCALE, WORLD_SCALE, WORLD_SCALE);

		auto pos = pCamera->GetOrigin();
		float snappedX = pos.x, snappedZ = pos.z;
		if (SNAP_GRID_SIZE > 0)
		{
			snappedX = floorf(snappedX / SNAP_GRID_SIZE) * SNAP_GRID_SIZE;
			snappedZ = floor(snappedZ / SNAP_GRID_SIZE) * SNAP_GRID_SIZE;
		}
		const float dx = pos.x - snappedX;
		const float dz = pos.z = snappedZ;
		snappedX = pos.x - 2 * dx;
		snappedZ = pos.z - 2 * dz;
		auto mTrans = leo::XMMatrixTranslation(snappedX, 0, snappedZ);

		auto mWorld = mScale*mTrans;

		const auto mView = pCamera->View();
		auto mWorldView = mWorld*mView;
		auto mWorldViewProj = mWorld*pCamera->ViewProj();

		//branch to do
		mTessation->SetWolrdViewProj(mWorldViewProj, nullptr);
		mTessation->SetLodWorldView(mWorldView, nullptr);

		const auto mProj = pCamera->Proj();
		mTessation->SetProj(mProj,nullptr);

		auto cullingEye = pos;
		cullingEye.x -= snappedX;
		cullingEye.z -= snappedZ;
		mTessation->SetEyePos(leo::float3(cullingEye.x, cullingEye.y, cullingEye.z), nullptr);

		auto dir = pCamera->GetLook();

		mTessation->SetEyeDir(leo::float3(dir.x, dir.y, dir.z), nullptr);
	}

	con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	con->IASetIndexBuffer(mQuadListIB, DXGI_FORMAT_R32_UINT, 0);
	
	for (int i = 0; i != nRings; ++i)
	{
		auto& pRing = mTileRings[i];
		pRing->IASet(con);

		//g_HeightMapVar->SetResource(g_pHeightMapSRV);
		//g_GradientMapVar->SetResource(g_pGradientMapSRV);
		mTessation->SetTileSize(pRing->tileSize(),nullptr);

		// Need to apply the pass after setting its vars.
		mTessation->Apply(con);

		// Instancing is used: one tiles is one instance and the index buffer describes all the 
		// NxN patches within one tile.
		const int nIndices = QUAD_LIST_INDEX_COUNT;
		con->DrawIndexedInstanced(nIndices, pRing->nTiles(), 0, 0, 0);
	}
}

float nonlinearCameraSpeed(float f)
{
	// Make the slider control logarithmic.
	return pow(10.0f, f / 100.0f);	// f [1,400]
}

float invNonlinearCameraSpeed(float f)
{
	// Make the slider control logarithmic.
	return 100.0f * (logf(f) / logf(10.0f));
}