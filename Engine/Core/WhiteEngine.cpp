#include "WhiteEngine.h"

using namespace LeoEngine::Core;

Camera * WhiteEngine::GetGraphicsPassCamera(const Camera & camera)
{
	return new Camera(camera);
}
