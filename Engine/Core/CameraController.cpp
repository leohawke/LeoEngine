#include <LBase/lmacro.h>
#include "CameraController.h"

using namespace LeoEngine::Core;

ImplDeDtor(CameraManipulator)

void CameraManipulator::Attach(Camera& camera)
{
	pCamera.reset(&camera);
}

void CameraManipulator::Detach() {
	pCamera.reset();
}

void TrackballCameraManipulator::Attach(Camera & camera)
{
	CameraManipulator::Attach(camera);

	reverse_target = false;
	target = pCamera->GetEyePos() + pCamera->GetForwardVector()*look_at_dist;
	right = pCamera->GetRightVector();
}

void TrackballCameraManipulator::Move(lm::float2 delta)
{
	auto offset = right * (delta.x*moveScaler * 2);
	auto pos = pCamera->GetEyePos() + offset;
	target += offset;
	
	offset = pCamera->GetUpVector()*(delta.y*moveScaler * 2);
	pos += offset;
	target += offset;

	look_at_dist = lm::length(target - pos);
	pCamera->SetViewMatrix(X::look_at_lh(pos, target, pCamera->GetUpVector()));
}

void TrackballCameraManipulator::Rotate(lm::float2 offset)
{
	auto q = lm::rotation_axis(right, offset.y*rotationScaler);
	auto mat = lm::make_matrix(target, q, {});
	auto pos = lm::transformpoint(pCamera->GetEyePos(), mat);

	q = lm::rotation_axis({ 0,lm::sgn(pCamera->GetUpVector().y),0.0f }, offset.x*rotationScaler);
	mat = lm::make_matrix(target, q, {});
	pos = lm::transformpoint(pos, mat);

	right = lm::transformquat(right, q);

	lm::float3 dir{};

	if (reverse_target)
		dir = pos - target;
	else

		dir = target - pos;

	auto dist = lm::length(dir);
	dir /= dist;
	auto up = lm::cross(dir, right);

	look_at_dist = dist;
	pCamera->SetViewMatrix(X::look_at_lh(pos, pos + dir * look_at_dist, up));
}

void TrackballCameraManipulator::Zoom(lm::float2 delta)
{
	auto offset = pCamera->GetForwardVector()*((delta.x + delta.y)*moveScaler * 2);
	auto pos = pCamera->GetEyePos() + offset;

	if (lm::dot(target - pos, pCamera->GetForwardVector()) <= 0)
		reverse_target = true;
	else
		reverse_target = false;

	pCamera->SetViewMatrix(X::look_at_lh(pos, pos + pCamera->GetForwardVector() * look_at_dist, pCamera->GetUpVector()));
}
