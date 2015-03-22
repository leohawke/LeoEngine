#include "Core\MeshLoad.hpp"
#include "file.hpp"
#include "leomath.hpp"
#include "Core\Camera.hpp"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	leo::MeshFile::d3dTol3d(L"skull.d3d",L"skull.l3d");
	return 0;
}