#include "Camera.h"
#include <cmath>

using namespace LeoEngine::Core;

void Camera::SetFrustum(leo::uint16 _width, leo::uint16 _height, const FrustumElement & frustum)
{
	width = _width;
	height = _height;

	frustum_elemnt = frustum;
}
