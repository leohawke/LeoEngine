#include "Light.hpp"
#include "Camera.hpp"
#include <leomathutility.hpp>
using namespace leo;

ops::Rect leo::CalcScissorRect(const PointLight & wPointLight, const Camera & camera)
{
	float4 centerWS = wPointLight.PointRange;
	centerWS.w = 1.f;

	auto centerWSvector = load(centerWS);
	auto ViewMatrixmatrix = load(camera.View());

	float4 centerVS;
	save(centerVS,Multiply(centerWSvector, ViewMatrixmatrix));
	return ops::Rect();
}
