////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/Geometry.hpp
//  Version:     v1.00
//  Created:     10/21/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: ¼¸ºÎÌå
// -------------------------------------------------------------------------
//  History:
//		
//
////////////////////////////////////////////////////////////////////////////
#ifndef IndePlatform_Geometry_hpp
#define IndePlatform_Geometry_hpp

#include "leoint.hpp"
#include "LeoMath.h"

namespace leo{

	enum class FRUSTUM_PLANE_TYPE : std::uint8_t
	{
		FRUSTUM_LEFT, FRUSTUM_RIGHT,
		FRUSTUM_TOP, FRUSTUM_BOTTOM,
		FRUSTUM_NEAR, FRUSTUM_FAR,
		FRUSTUM_PLANES = 6
	};

	enum class DIRECTION_2D_TYPE : std::uint8_t
	{
		DIRECTION_LEFT = 0, DIRECTION_RIGHT = 1,
		DIRECTION_TOP = 2, DIRECTION_BOTTOM = 3,
		DIRECTIONS_2DS = 4
	};

	enum class DIRECTION_3D_TYPE : std::uint8_t
	{
		DIRECTION_LEFT, DIRECTION_RIGHT,
		DIRECTION_TOP, DIRECTION_BOTTOM,
		DIRECTION_FRONT, DIRECTION_DORSUM,
		DIRECTIONS_3DS = 6
	};

	enum class PROJECTION_TYPE : std::uint8_t
	{
		ORTHOGRAPHIC,
		PERSPECTIVE
	};

	enum class CONTAINMENT_TYPE : std::uint8_t{
		DISJOINT = 0,
		INTERSECTS = 1,
		CONTAINS = 2,
	};


	struct Triangle;
	struct Box;
	struct Sphere;
	struct OrientedBox;
	struct Frustum;

	struct Triangle{
		float3 p[3];
	public:
		Triangle() = default;
		Triangle(const float3& p0, const float3& p1, const float3& p2){
			p[0] = p0;
			p[1] = p1;
			p[2] = p2;
		}

		void operator=(const Triangle& tri){
			std::memcpy(this, &tri, sizeof(Triangle));
		}
	};
}


#endif