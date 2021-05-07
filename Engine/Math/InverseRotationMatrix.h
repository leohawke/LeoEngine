#pragma once

#include "Rotator.h"

namespace LeoEngine
{
	class InverseRotationMatrix :public lm::float4x4
	{
	public:
		explicit InverseRotationMatrix(Rotator rot)
		{
			float Pitch = rot.Pitch * leo::math::PI / 180.0f;
			float Yaw = rot.Yaw * leo::math::PI / 180.0f;
			float Roll = rot.Roll * leo::math::PI / 180.0f;

			float SP, SY, SR;
			float CP, CY, CR;


			SP = std::sin(Pitch);
			CP = std::cos(Pitch);
			SY = std::sin(Yaw);
			CY = std::cos(Yaw);
			SR = std::sin(Roll);
			CR = std::cos(Roll);


			operator[](0) = lm::float4(CY * CR, SP * SY * CR + CP * (-SR), CP * (-SY) * CR + SP * (-SR), 0);

			/* | SP*SY  CP  -SP*CY |  */
			operator()(1, 0) = CY * SR;
			operator()(1, 1) = SP * SY * SR + CP * CR;
			operator()(1, 2) = CP * (-SY) * SR + SP * CR;
			operator()(1, 3) = 0.f;

			operator()(2, 0) = SY;
			operator()(2, 1) = -SP * CY;
			operator()(2, 2) = CP * CY;
			operator()(2, 3) = 0.f;

			operator()(3, 0) = 0;
			operator()(3, 1) = 0;
			operator()(3, 2) = 0;
			operator()(3, 3) = 1;
		}
	};
}