#pragma once

#include "Rotator.h"

namespace LeoEngine
{
	class RotationMatrix :public lm::float4x4
	{
	public:
		explicit RotationMatrix(Rotator rot)
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


			/*
			* | 1   0   0  |    | CY  0  SY |     | CY  0  SY |
			* | 0   CP -SP |  * |  0  1   0 |  =
			* | 0   SP  CP |	| -SY 0  CY |
			*/
			operator[](0) = lm::float4(CY * CR, CY * SR, SY, 0);

			/* | SP*SY  CP  -SP*CY |  */
			operator()(1, 0) = SP * SY * CR + CP * (-SR);
			operator()(1, 1) = SP * SY * SR + CP * CR;
			operator()(1, 2) = -SP*CY;
			operator()(1, 3) = 0.f;

			/*
			*                       | CR   SR   0 |
			* [CP*(-SY),SP,CP*CY] * |-SR   CR   0 |
			*                       |  0    0   1 |
			*/
			operator()(2, 0) = CP * (-SY) * CR + SP * (-SR);
			operator()(2, 1) = CP * (-SY) * SR + SP * CR;
			operator()(2, 2) = CP * CY;
			operator()(2, 3) = 0.f;

			operator()(3, 0) = 0;
			operator()(3, 1) = 0;
			operator()(3, 2) = 0;
			operator()(3, 3) = 1;
		}

		static lm::float4x4 MakeFromZ(lm::float3 zaxis);
	};
}