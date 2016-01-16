#include "Light.hpp"
#include "Camera.hpp"
#include <leomathutility.hpp>
#include <Singleton.hpp>

using namespace leo;

#ifdef near
#undef near
#undef far
#endif

ops::Rect leo::CalcScissorRect(const PointLight & wPointLight, const Camera & camera)
{
	//Create a bounding sphere for the light,based on the position
	//and range
	auto centerWS =float4( wPointLight.Position,1.f);
	auto radius = wPointLight.FallOff_Range.w;


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
	auto near = camera.mNear;
	auto far = camera.mFar;
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

leo::LightSource::LightSource(light_type type)
	:_type(type)
{
}

leo::LightSource::light_type leo::LightSource::Type() const
{
	return _type;
}

const float3 & leo::LightSource::Position() const
{
	return mPos;
}

float leo::LightSource::Range() const
{
	return mRange;
}

const float3 & leo::LightSource::Diffuse() const
{
	return mDiffuse;
}

void leo::LightSource::Position(const float3 & pos)
{
	mPos = pos;
}

void leo::LightSource::Range(float range)
{
	mRange = range;
}

void leo::LightSource::Diffuse(const float3 & diffuse)
{
	mDiffuse = diffuse;
}

leo::PointLightSource::PointLightSource()
	:LightSource(point_light)
{
}

const float3& leo::PointLightSource::FallOff() const {
	return mFallOff;
}

void leo::PointLightSource::FallOff(const float3& falloff){
	 mFallOff = falloff;
}


leo::SpotLightSource::SpotLightSource()
	:LightSource(spot_light)
{
}

const float3& leo::SpotLightSource::FallOff() const {
	return mFallOff;
}

void leo::SpotLightSource::FallOff(const float3& falloff) {
	mFallOff = falloff;
}

leo::DirectionalLightSource::DirectionalLightSource()
	:LightSource(directional_light)
{
}

leo::AmbientLightSource::AmbientLightSource()
	: LightSource(ambient_light)
{
}

const leo::float3 leo::AmbientLightSource::mDirectional = { 0,-1.f,0 };



