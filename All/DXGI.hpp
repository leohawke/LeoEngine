#include <dxgi.h>
#include <vector>
#pragma comment(lib,"dxgi.lib")
namespace dxgi
{
	std::vector<IDXGIAdapter1*> EnumerAdapters();
}