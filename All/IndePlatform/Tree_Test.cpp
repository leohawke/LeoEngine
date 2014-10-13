//ÄÚ´æ¼ì²â,0
#define _CRTDBG_MAP_ALLOC
#include	<stdlib.h>
#include	<crtdbg.h>
#include "Tree.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	leo::QuadTree<int> mQuad(leo::float4(0.f, 0.f, 100.f, 100.f));

	mQuad.Insert(1,leo::float2(1.f, 1.f));
	mQuad.Insert(2, leo::float2(3.f, 3.f));
	mQuad.Insert(3, leo::float2(10.f, 10.f));
	mQuad.Insert(4, leo::float2(30.f, 30.f));
	mQuad.Insert(1, leo::float2(-1.f, -1.f));
	mQuad.Insert(1, leo::float2(-3.f, -3.f));
	mQuad.Insert(1, leo::float2(-10.f, -10.f));
	mQuad.Insert(1, leo::float2(-30.f, -30.f));

	return 0;
}