#include "Light.hpp"
#include "Camera.hpp"
#include <leomathutility.hpp>
using namespace leo;

ops::Rect leo::CalcScissorRect(const PointLight & wPointLight, const Camera & camera)
{
	//Create a bounding sphere for the light,based on the position
	//and range
	auto centerWS = wPointLight.PointRange;
	auto radius = centerWS.w;

	centerWS.w = 1.f;

	auto centerWSvector = load(centerWS);
	auto ViewMatrixmatrix = load(camera.View());

	float4 centerVS;
	//Transfrom the sphere center to view space
	save(centerVS,Multiply(centerWSvector, ViewMatrixmatrix));

	//Figure out the four points at the top,bottom,left, and
	//right of the sphere
	auto topVS = centerVS;
	topVS.y += radius;

	auto bottomVS = centerVS;
	bottomVS.y -= radius;

	auto leftVS = centerVS;
	leftVS.x -= radius;

	auto rightVS = centerVS;
	rightVS.y += radius;

	//Figure out whether we want to use the top and right from quad
	//tangent to the front of the sphere, or the back or the sphere
	leftVS.z = leftVS.x < 0.f ? leftVS.z - radius:leftVS.z+radius;
	rightVS.z = rightVS.x < 0.f ? rightVS.z + radius : rightVS.z - radius;
	topVS.z = topVS.x < 0.f ? topVS.z + radius : topVS.z - radius;
	bottomVS.z = bottomVS.x < 0.f ? bottomVS.z - radius : bottomVS.z + radius;

	//Clamp the z coordinate to the clip planes
	auto near = camera.g//can't get near?
	leftVS
	return ops::Rect();
}
