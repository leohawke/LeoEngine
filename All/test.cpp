//内存检测,0
#define _CRTDBG_MAP_ALLOC
#include	<stdlib.h>
#include	<crtdbg.h>
//Win头,1
#include "Win.hpp"
#include "resource.h"
//平台独立,2
#include "IndePlatform\memory.hpp"
#include "IndePlatform\clock.hpp"
#include "IndePlatform\any.h"
#include "IndePlatform\bitseg.hpp"
#include "IndePlatform\Singleton.hpp"
//渲染相关,核心,3
#include "Core\Vertex.hpp"
#include "Core\mesh.hpp"
#include "Core\Water.hpp"
#include "Core\Camera.hpp"
#include "Core\BillBoard.hpp"
#include "Core\Effect.h"
#include "Core\Sky.hpp"
#include "Core\MeshLoad.hpp"
#include "Core\MeshLoad.hpp"
//Win相关,基本,4
#include "debug.hpp"
#include "RenderStates.hpp"//=>迁移
#include "LightBuffer.h"
#include "Input.h"
#include "Mgr.hpp"
#include "DeviceMgr.h"
#include "sexp.hpp"
#include "file.hpp"
#include "leomath.hpp"
//第3方库
#include "DirectXTex.h"
#include "scheme_helper.h"
//操作系统库
#include <d2d1.h>
#pragma comment(lib,"d2d1")
//标准库
#include <list>
#include <memory>
#include <array>
#include <fstream>
#include <thread>


ID3D11Buffer* g_VertexBuffer;
ID3D11Buffer* g_VertexConstBuffer;
ID3D11Buffer* g_IndexBuffer;

ID3D11VertexShader* g_VertexShader;
ID3D11PixelShader* g_PixelShader;

ID3D11HullShader* g_HullShader;
ID3D11DomainShader* g_DomainShader;
ID3D11InputLayout* g_InputLayout;

struct VCB
{
	leo::XMMATRIX mWorld;
	leo::XMMATRIX mView;
	leo::XMMATRIX mProj;
};

void ClearResource();

void BuildResource(const leo::DeviceMgr& dm);

void Render(const leo::DeviceMgr& dm);

void ResizeRenderRes(const leo::DeviceMgr& dm);


leo::Camera* camera = nullptr;
std::vector<leo::Mesh*> meshs;
leo::BillBoard* billboard = nullptr;
leo::Sky* sky = nullptr;

leo::effect_normalmap* effect = nullptr;

leo::effect_sprite* sprite_effect = nullptr;
leo::effect_sky* sky_effect = nullptr;
leo::effect_postrender* post_effect = nullptr;



decltype(leo::scheme::sexp::parse(L"")) config_sexp;


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	
	leo::StackAlloc<4096> stackalloc;
	auto pos = stackalloc.GetPos();
	AllocConsole();
	leo::ReSetStdIOtoFile(leo::which_stdio::ostream, L"CONOUT$");
	leo::DeviceMgr DeviceMgr;
	leo::OutputWindow win;
	
	config_sexp = leo::parse_file(S("config.scheme"));

	auto window_sexp = leo::scheme::sexp::find_sexp(L"window", config_sexp);

	auto width = static_cast<leo::uint16>(leo::expack_long(window_sexp, S("width")));
	auto height = static_cast<leo::uint16>(leo::expack_long(window_sexp, S("height")));


	auto clientSize  = std::make_pair(width, height);
	if (!win.Create(GetModuleHandle(NULL),clientSize,leo::expack_string(window_sexp,S("tile"))))
	{
		return 0;
	}

	
	DeviceMgr.CreateDevice(false, clientSize);

	//leo::GaussBlur::GetInstance(DeviceMgr.GetDevice(),clientSize);
	leo::dx::profiler::global_profiler.init(DeviceMgr.GetDevice(), DeviceMgr.GetDeviceContext());
	

	stdex::aligned_alloc<leo::effect_normalmap, 16> effect_alloc;

	effect = effect_alloc.allocate(1);
	new(effect)leo::effect_normalmap(DeviceMgr.GetDevice());
	sprite_effect = new leo::effect_sprite(DeviceMgr.GetDevice());
	sky_effect = new leo::effect_sky(DeviceMgr.GetDevice());
	post_effect = new leo::effect_postrender(DeviceMgr.GetDevice());

	leo::cbPerFrame m_cbperframe;


	leo::DirectionLight dirlight;
	dirlight.ambient = leo::float4(1.f, 1.f, 1.f, 1.f);
	dirlight.diffuse = leo::float4(1.f, 1.f, 1.f, 1.f);
	dirlight.specular = leo::float4(1.f, 1.f, 1.f, 32.f);
	dirlight.dir = leo::float4(0.0f, 1.0f, 5.0f, 0.f);

	m_cbperframe.gDirLight = dirlight;
	m_cbperframe.gPoiLight.ambient.w = 0.f;
	m_cbperframe.gSpoLight.ambient.w = 0.f;
	effect->PSSetConstantBuffer(m_cbperframe.slot, &m_cbperframe);



	//Test: LoadTexture2DSRV
	//texmanger.LoadTexture2DArraySRV(std::array<const wchar_t*, 4>({ L"Resource\\tree0.dds", L"Resource\\tree1.dds", L"Resource\\tree2.dds", L"Resource\\tree3.dds" }));
	//EndTest


	auto models_sexp = leo::scheme::sexp::find_sexp(L"models", config_sexp);
	auto models_num = leo::scheme::sexp::sexp_list_length(models_sexp);

	if (models_num <= 1)
		std::abort();//Raise_Error_Exception(ERROR_INVALID_TOKEN, "models无文件,程序直接退出");


	meshs.resize(models_num - 1);
	auto mesh_iter = meshs.begin();
	auto models_iter = models_sexp->next;
	while (models_iter)
	{
		auto file_sexp = leo::scheme::sexp::find_sexp(L"file", models_iter);
		auto sqt = leo::expack_SQT(models_iter);
		*mesh_iter = NEW("main") leo::Mesh(sqt);
		(*mesh_iter)->Load(file_sexp->next->value, DeviceMgr.GetDevice());
		models_iter = models_iter->next;
		++mesh_iter;
	}

	stdex::aligned_alloc<leo::BillBoard, 16> alloc;
	billboard = alloc.allocate(1);
	new(billboard)leo::BillBoard(leo::float3(1.f, 5.f, 10.f), leo::float2(16, 16), L"Resource\\tree1.dds", DeviceMgr.GetDevice());
	sky = new leo::Sky(DeviceMgr.GetDevice(), L"Resource\\snowcube1024.dds", 5000.f);
	/*
	
	BuildResource(DeviceMgr);
	
	
	// Run the message loop.
	*/

	auto cameraaddress = stackalloc.AllocWithAlign(sizeof(leo::Camera), 16);
	camera = new(cameraaddress)leo::Camera();
	BuildResource(DeviceMgr);
	MSG msg = {0};
	//DeviceMgr.ToggleFullScreen(true);
	HACCEL hAccel = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_ACCELER1));
	auto cmdmsgproc = [&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)-> LRESULT
	{
		switch (LOWORD(wParam))
		{
		case ID_ESC:
			SendMessage(hwnd, WM_DESTROY, 0, 0);
			break;
		case ID_DRAW_MODE:
			DeviceMgr.ToggleFullScreen(true);
			break;
		case ID_SELECT_MODE:
			DeviceMgr.ToggleFullScreen(false);
			break;
		default:
			break;
		}
		return 0;
	};
	win.BindMsgFunc(WM_COMMAND, cmdmsgproc);

	POINT lastMousepos;
	auto mouseproc = [&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto x = GET_X_LPARAM(lParam);;
		auto y = GET_Y_LPARAM(lParam);
		switch (uMsg)
		{
		case WM_MOUSEMOVE:
			if ((wParam & MK_LBUTTON) != 0)
			{
				float dx = leo::XMConvertToRadians(0.25f*(x - lastMousepos.x));
				float dy = leo::XMConvertToRadians(0.25f*(y - lastMousepos.y));
				camera->RotateY(dx);
				camera->Pitch(dy);
			}
			lastMousepos.x = x;
			lastMousepos.y = y;
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			lastMousepos.x = x;
			lastMousepos.y = y;
			SetCapture(hwnd);
			break;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			ReleaseCapture();
			break;
		}

		return 0;
	};

	auto sizeproc = [&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (DeviceMgr.GetDevice())
		{
			auto size = std::make_pair<int, int>(LOWORD(lParam), HIWORD(lParam));
			//ReleaseRenderRes();
			DeviceMgr.ReSize(size);
			//ResizeRenderRes(DeviceMgr);
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	};
	win.BindMsgFunc(WM_COMMAND, cmdmsgproc);
	win.BindMsgFunc(WM_SIZE, sizeproc);
	win.BindMsgFunc(WM_MOUSEMOVE, mouseproc);
	win.BindMsgFunc(WM_LBUTTONDOWN, mouseproc);
	win.BindMsgFunc(WM_RBUTTONDOWN, mouseproc);
	win.BindMsgFunc(WM_MBUTTONDOWN, mouseproc);
	win.BindMsgFunc(WM_LBUTTONUP, mouseproc);
	win.BindMsgFunc(WM_MBUTTONUP, mouseproc);
	win.BindMsgFunc(WM_RBUTTONUP, mouseproc);

	while (1)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, nullptr, 0, 0))
				goto QUIT;
			if (!TranslateAccelerator(win.GetHwnd(), hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		Render(DeviceMgr);
	}
	//leo::ReSetStdIOtoFile(leo::which_stdio::istream, L"CONIN$");
	FreeConsole();
	QUIT:
	leo::global::Destroy();
	win.Destroy();
	
	for (auto &mesh : meshs)
		delete mesh;

	camera->~Camera();
	billboard->~BillBoard();
	alloc.deallocate(billboard, 1);
	delete sky;

	effect->~effect_normalmap();
	effect_alloc.deallocate(effect, 1);
	delete sprite_effect;
	delete sky_effect;
	delete post_effect;


	ClearResource();

#ifdef DEBUG
	leo::SingletonManger::GetInstance()->PrintAllSingletonInfo();
#endif
	leo::SingletonManger::GetInstance()->UnInstallAllSingleton();	

	CoUninitialize();
	return 0;
}

void Render(const leo::DeviceMgr& dm)
{
	using leo::Clock;
	static leo::Clock sysclock;
	sysclock.Update();
	static auto timecount = Clock::DurationTo(sysclock.GetDelta());
	
	timecount += Clock::DurationTo(sysclock.GetDelta());
	static auto count = 0LU;
	
	if (timecount > 5)
	{
		
		LogPrintf(L"FPS: %f,Delta Time: %f\n", (float)count / timecount, Clock::DurationTo(sysclock.GetDelta()));
		timecount = 0;
		count = 0;
	}
	count++;
	/*
	static leo::clock sysclock;
	static auto newcount = sysclock.GetElapse().count();
	static decltype(newcount) oldcount{};
	static decltype(newcount) deltacount{};
	//60FPS
	/*do
	{
		sysclock.update();
		newcount = sysclock.GetElapse().count();
		deltacount = newcount - oldcount;
		Sleep(0);
	} while (deltacount < 16666666);
	

	auto stackpos = stackalloc.GetPos();
	auto* pblockbuff = stackalloc.Alloc(sizeof(leo::dx::profileblock));
	auto* pblock = new(pblockbuff)leo::dx::profileblock(L"Draw Trian");*/

	//auto rendertarget = dm.GetRenderTargetView();
	//devicecontext->OMSetRenderTargets(1, &rendertarget, dm.GetDepthStencilView());
	leo::win::KeysState::GetInstance()->Update();

	if (GetAsyncKeyState('W') & 0X8000)
		camera->Walk(+0.02f);

	if (GetAsyncKeyState('S') & 0X8000)
		camera->Walk(-0.02f);

	if (GetAsyncKeyState('A') & 0X8000)
		camera->Strafe(-0.02f);

	if (GetAsyncKeyState('D') & 0X8000)
		camera->Strafe(+0.02f);

	auto devicecontext = dm.GetDeviceContext();
	float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.8f };
	devicecontext->ClearRenderTargetView(dm.GetRenderTargetView(), ClearColor);
	devicecontext->ClearDepthStencilView(dm.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

	camera->UpdateViewMatrix();
	//sky->Render(devicecontext, *camera, *sky_effect);
	//meshs[0]->Render(devicecontext, camera->View(), camera->Proj());
	if (GetAsyncKeyState('Y') & 0X8000)
		meshs[1]->Yaw(+0.02f);
	if (GetAsyncKeyState('R') & 0X8000)
		meshs[1]->Roll(1.f);
	if (GetAsyncKeyState('P') & 0X8000)
		meshs[1]->Pitch(+0.02f);
	meshs[1]->Render(devicecontext,*camera,*effect);
	//billboard->Render(devicecontext, *camera,*sprite_effect);

	//auto& gb = leo::GaussBlur::GetInstance();
	//gb->Mono();
	//gb->Render(devicecontext, *camera, *post_effect);
	
	
	leo::dx::profiler::global_profiler.endframe();

	dm.GetSwapChain()->Present(0, 0);
}

void BuildResource(const leo::DeviceMgr& dm)
{
	using namespace leo;
	// Initialize the view matrix
	auto camera_sexp = leo::scheme::sexp::find_sexp(L"camera", config_sexp);

	auto eye_sexp = leo::scheme::sexp::find_sexp(L"eye", camera_sexp);
	auto at_sexp = leo::scheme::sexp::find_sexp(L"at", camera_sexp);
	auto up_sexp = leo::scheme::sexp::find_sexp(L"up", camera_sexp);
	auto Eye = expack_float3(camera_sexp, S("eye"));
	auto At = expack_float3(camera_sexp, S("at"));
	auto Up = expack_float3(camera_sexp, S("up"));

	camera->LookAt(Eye, At, Up);
	
	camera->SetFrustum(def::frustum_fov, dm.GetAspect(), def::frustum_near, def::frustum_far);
	camera->SetFrustum(leo::PROJECTION_TYPE::PERSPECTIVE);

}

void ClearResource()
{
}