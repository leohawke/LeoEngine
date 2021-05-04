#pragma once

#include "Rotator.h"

namespace LeoEngine
{
	class RotationMatrix :public leo::math::float4x4
	{
	public:
		explicit RotationMatrix(Rotator rot)
		{
			float x = rot.Pitch * leo::math::PI / 180.0f;
			float y = rot.Yaw * leo::math::PI / 180.0f;
			float z = rot.Roll * leo::math::PI / 180.0f;

			float cx = std::cos(x),sx = std::sin(x);
			float cz = std::cos(z), sz = std::sin(z);
			float cy = std::cos(y), sy = std::sin(y);

			operator()(0, 0) = cy * cz;
			operator()(0, 1) = cy * sz;
			operator()(0, 2) = sy;
			operator()(0, 3) = 0;

			operator()(1, 0) = sx * sy*cz - cx*sz;
			operator()(1, 1) = sx * sy*sz + cx*cz;
			operator()(1, 2) = sx*cy;
			operator()(1, 3) = 0;

			operator()(2, 0) = -(cx*sy*cz+sx*sz);
			operator()(2, 1) = cz * sx- cx * sy * sz;
			operator()(2, 2) = cx*cy;
			operator()(2, 3) = 0;

			operator()(3, 0) = 0;
			operator()(3, 1) = 0;
			operator()(3, 2) = 0;
			operator()(3, 3) = 1;
		}

		static lm::float4x4 MakeFromZ(lm::float3 zaxis);
	};
}