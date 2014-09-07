#include "MeshLoad.hpp"
#include "file.hpp"
#include "leomath.hpp"
#include "Core\Camera.hpp"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	auto f = [](float x, float z)
	{
		return 0.3f*(z*leo::sinr(0.1f*x) + x*leo::cosr(0.1f*z));
	};
	leo::MeshFile::terrainTol3d(f, std::make_pair(500, 500), std::make_pair(50, 50), L"terrain.l3d");
	return 0;
}