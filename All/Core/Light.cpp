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
	auto near = 0.f;//camera//can't get near?
	auto far = 1000.f;
	leftVS.z = clamp( near, far, leftVS.z);
	rightVS.z = clamp( near, far, rightVS.z);
	topVS.z = clamp(near, far, topVS.z);
	bottomVS.z = clamp( near, far, bottomVS.z);

	//Figure out the rectangle in clip-space by applying the
	//perspective transfrom. We assume that the perspective
	//transfrom is symmetrical with respect to X and Y
	auto ProjMatrix = camera.Proj();

	auto rectLeftCS = leftVS.x*ProjMatrix(0, 0) / leftVS.z;
	auto rectRightCS = rightVS.x*ProjMatrix(0, 0)/ rightVS.z;
	auto rectTopCS = topVS.y*ProjMatrix(1, 1) / topVS.z;
	auto rectBottomCS = bottomVS.y*ProjMatrix(1, 1) / bottomVS.z;

	//Clamp the rectangle to the screen extents
	rectLeftCS = clamp(-1.f, 1.f, rectLeftCS);
	rectRightCS = clamp(-1.f, 1.f, rectRightCS);
	rectTopCS = clamp(-1.f, 1.f, rectTopCS);
	rectBottomCS = clamp(-1.f, 1.f, rectBottomCS);

	//Now we convert to screen coordinates by applying the
	//viewport transfrom
	auto rectTopSS = rectTopCS*0.5f + 0.5f;
	auto rectRightSS = rectRightCS*0.5f + 0.5f;
	auto rectBottomSS = rectBottomCS*0.5f + 0.5f;
	auto rectLeftSS = rectLeftCS*0.5f + 0.5f;

	rectTopSS = 1.f - rectTopSS;
	rectBottomSS = 1.f - rectBottomSS;

	return ops::Rect(float4(rectTopSS,rectLeftSS,rectBottomSS,rectRightSS));
}
