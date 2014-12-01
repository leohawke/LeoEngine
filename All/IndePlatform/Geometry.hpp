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

namespace leo {

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

	enum class CONTAINMENT_TYPE : std::uint8_t {
		DISJOINT = 0,
		INTERSECTS = 1,
		CONTAINS = 2,
	};

	enum PLANE_INTERSECTION_TYPE : std::uint8_t {
		FRONT = 0,
		INTERSECTING = 1,
		BACK = 2,
	};



	struct Triangle;
	struct Box;
	struct Sphere;
	struct OrientedBox;
	struct Frustum;

	struct Ray;

	struct lalignas(16) LB_API Triangle {
		using vector = __m128;

		float3 p[3];
		Triangle() = default;
		Triangle(const float3& p0, const float3& p1, const float3& p2) {
			p[0] = p0;
			p[1] = p1;
			p[2] = p2;
		}



		CONTAINMENT_TYPE    LM_VECTOR_CALL     Contains(vector Point) const;
		CONTAINMENT_TYPE    LM_VECTOR_CALL     Contains(const Triangle& Tri) const;


		bool Intersects(const Sphere& sh) const;
		bool Intersects(const Box& box) const;
		bool Intersects(const OrientedBox& box) const;
		bool Intersects(const Frustum& fr) const;
		bool LM_VECTOR_CALL	Intersects(const Triangle& Tri) const;
		PLANE_INTERSECTION_TYPE    LM_VECTOR_CALL    Intersects(vector Plane) const;
		std::pair<bool, float> Intersects(const Ray& sphere) const;

		CONTAINMENT_TYPE  ContainedBy(const Frustum&) const;
	};

	struct lalignas(16) LB_API Sphere {
		using vector = __m128;

		float4 mCenterRadius;
		inline float3 GetCenter() const {
			return float3(mCenterRadius);
		}
		inline float GetRadius() const {
			return mCenterRadius.w;
		}

		Sphere(const float3& center, float radius)
			:mCenterRadius(center, radius)
		{}


		CONTAINMENT_TYPE    LM_VECTOR_CALL     Contains(vector Point) const;
		CONTAINMENT_TYPE    LM_VECTOR_CALL     Contains(const Triangle& Tri) const;


		bool Intersects(const Sphere& sh) const;
		bool Intersects(const Box& box) const;
		bool Intersects(const OrientedBox& box) const;
		bool Intersects(const Frustum& fr) const;
		bool LM_VECTOR_CALL	Intersects(const Triangle& Tri) const;
		PLANE_INTERSECTION_TYPE    LM_VECTOR_CALL    Intersects(vector Plane) const;
		std::pair<bool, float> Intersects(const Ray& sphere) const;

		CONTAINMENT_TYPE  ContainedBy(const Frustum&) const;
	};

	struct lalignas(16) LB_API Box {
		using vector = __m128;

		//static const size_t CORNER_COUNT = 8;

		float3 mCenter;            // Center of the box.
		float3 mExtents;           // Distance from the center to each side.

		CONTAINMENT_TYPE    LM_VECTOR_CALL     Contains(vector Point) const;
		CONTAINMENT_TYPE    LM_VECTOR_CALL     Contains(const Triangle& Tri) const;


		bool Intersects(const Sphere& sh) const;
		bool Intersects(const Box& box) const;
		bool Intersects(const OrientedBox& box) const;
		bool Intersects(const Frustum& fr) const;
		bool LM_VECTOR_CALL	Intersects(const Triangle& Tri) const;
		PLANE_INTERSECTION_TYPE    LM_VECTOR_CALL    Intersects(vector Plane) const;
		std::pair<bool, float> Intersects(const Ray& sphere) const;

		CONTAINMENT_TYPE  ContainedBy(const Frustum&) const;
	};

	struct lalignas(16) LB_API OrientedBox : public Box {
		float4 mOrientation;

		using vector = __m128;
		using matrix = std::array<vector, 4>;

		CONTAINMENT_TYPE    LM_VECTOR_CALL     Contains(vector Point) const;
		CONTAINMENT_TYPE    LM_VECTOR_CALL     Contains(const Triangle& Tri) const;


		bool Intersects(const Sphere& sh) const;
		bool Intersects(const Box& box) const;
		bool Intersects(const OrientedBox& box) const;
		bool Intersects(const Frustum& fr) const;
		bool LM_VECTOR_CALL	Intersects(const Triangle& Tri) const;
		PLANE_INTERSECTION_TYPE    LM_VECTOR_CALL    Intersects(vector Plane) const;
		std::pair<bool, float> Intersects(const Ray& sphere) const;

		CONTAINMENT_TYPE  ContainedBy(const Frustum&) const;
	};

	struct lalignas(16) LB_API Frustum {
		static const size_t CORNER_COUNT = 8;

		//WorldSpace
		float3 mOrigin;            // Origin of the frustum (and projection).
		//LocalSpace->WorldSpace
		float3 mOrientation;       // Quaternion representing rotation.

		float mRightSlope;           // Positive X slope (X/Z).
		float mLeftSlope;            // Negative X slope.
		float mTopSlope;             // Positive Y slope (Y/Z).
		float mBottomSlope;          // Negative Y slope.
		float mNear, mFar;            // Z of the near plane and far plane.

		// Creators
		Frustum() = default;
		Frustum(const float3& Origin, const float4& Orientation,
			float RightSlope, float LeftSlope, float TopSlope, float BottomSlope,
			float Near, float Far)
			:mOrigin(Origin), mOrientation(Orientation), mRightSlope(RightSlope), mLeftSlope(LeftSlope),
			mTopSlope(TopSlope), mBottomSlope(BottomSlope), mNear(Near), mFar(Far)
		{
		}
		Frustum(const Frustum& fr) = default;

		using vector = __m128;
		using matrix = std::array<vector, 4>;

		explicit Frustum(const matrix& Projection)
		{
			operator=(Projection);
		}

		Frustum&  LM_VECTOR_CALL  operator=(matrix Projection);

		Frustum&  LM_VECTOR_CALL Transform(matrix M);
		Frustum&  LM_VECTOR_CALL Transform(float Scale, vector Rotation, vector Translation);

		//void GetCorners(float3* Corners) const;
		// Gets the 8 corners of the frustum

		CONTAINMENT_TYPE    LM_VECTOR_CALL     Contains(vector Point) const;
		CONTAINMENT_TYPE    LM_VECTOR_CALL     Contains(const Triangle& Tri) const;
		CONTAINMENT_TYPE Contains(const Sphere& sp) const;
		CONTAINMENT_TYPE Contains(const Box& box) const;
		CONTAINMENT_TYPE Contains(const OrientedBox& box) const;
		//CONTAINMENT_TYPE Contains(const Frustum& fr) const;
		// Frustum-Frustum test

		bool Intersects(const Sphere& sh) const;
		bool Intersects(const Box& box) const;
		bool Intersects(const OrientedBox& box) const;
		//bool Intersects(const Frustum& fr) const;
		bool LM_VECTOR_CALL	Intersects(const Triangle& Tri) const;
		PLANE_INTERSECTION_TYPE    LM_VECTOR_CALL    Intersects(vector Plane) const;
		std::pair<bool,float>    LM_VECTOR_CALL     Intersects(const Ray& ray) const;

		CONTAINMENT_TYPE     LM_VECTOR_CALL     ContainedBy(vector Plane0, vector Plane1, vector Plane2,
			const vector& Plane3, const vector& Plane4, const vector& Plane5) const;
		// Test frustum against six planes (see Frustum::GetPlanes)

		//void GetPlanes(vector* NearPlane, vector* FarPlane, vector* RightPlane,
			//vector* LeftPlane, vector* TopPlane, vector* BottomPlane) const;
		// Create 6 Planes representation of Frustum

		
	};
}


#endif