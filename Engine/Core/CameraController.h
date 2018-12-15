/*! \file Core\CameraController.h
\ingroup Engine
\brief 提供相机相关的基本操作器。
*/

#ifndef LE_Core_CameraController_H
#define LE_Core_CameraController_H 1

#include <LBase/sutility.h>
#include <LBase/memory.hpp>

#include "Camera.h"


namespace LeoEngine::Core {
	namespace lm = leo::math;

	class CameraManipulator : leo::noncopyable {
	public:
		CameraManipulator(float rotationSpeed = 0.05f,float moveSpeed=1):
			rotationScaler(rotationSpeed), moveScaler(moveSpeed)
		{}

		void SetSpeed(float rotationSpeed, float moveSpeed) {
			rotationScaler = rotationSpeed;
			moveScaler = moveSpeed;
		}

		virtual ~CameraManipulator();

		virtual void Attach(Camera& camera);
		virtual void Detach();
	protected:
		leo::observer_ptr<Camera> pCamera;

		float rotationScaler, moveScaler;
	};

	class TrackballCameraManipulator :public CameraManipulator {
	public:
		explicit TrackballCameraManipulator(float dist):
			look_at_dist(dist)
		{}

		void Attach(Camera& camera) override;

		void Move(lm::float2 offset);
		void Rotate(lm::float2 offset);
		void Zoom(lm::float2 offset);
	private:
		bool reverse_target;
		lm::float3 target;
		lm::float3 right;

		float look_at_dist;
	};
}

#endif