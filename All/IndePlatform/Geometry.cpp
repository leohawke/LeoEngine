#include "LeoMath.h"
#include "leo_math_impl.hpp"

#include "Geometry.hpp"
#include "Geometry_impl.hpp"

#include <float.h>

namespace leo {

	CONTAINMENT_TYPE  Triangle::ContainedBy(const Frustum& Fru) const {
		return Fru.Contains(*this);
	}


	std::pair<bool, float> Sphere::Intersects(const Ray& ray) const {
		const auto Direction = load(ray.mDir);

		assert(details::IsUnit<>(Direction));

		auto mCenter = load(GetCenter());
		auto mRadius = Splat(GetRadius());

		auto Origin = load(ray.mOrigin);

		// l is the vector from the ray origin to the center of the sphere.
		auto l = Subtract(mCenter, Origin);
		auto l2 = Dot<>(l, l);

		// s is the projection of the l onto the ray direction.
		auto s = Dot<>(l, Direction);

		auto r2 = Multiply(mRadius, mRadius);

		// m2 is squared distance from the center of the sphere to the projection.
		auto m2 = Subtract(l2, Multiply(s, s));

		// If the ray origin is outside the sphere and the center of the sphere is 
		// behind the ray origin there is no intersection.
		auto NoIntersection = AndInt(LessExt(s, SplatZero()), GreaterExt(l2, r2));

		// If the squared distance from the center of the sphere to the projection
		// is greater than the radius squared the ray will miss the sphere.
		NoIntersection = OrInt(NoIntersection, GreaterExt(m2, r2));

		// The ray hits the sphere, compute the nearest intersection point.
		auto q = Sqrt(Subtract(r2, m2));
		auto t1 = Subtract(s, q);
		auto t2 = Add(s, q);

		auto OriginInside = LessOrEqualExt(l2, r2);
		auto t = Select(t1, t2, OriginInside);
		if (NotEqualInt<>(NoIntersection, details::SplatTrueInt())) {
			float dist = 0.f;
			save(dist, t);
			return{ true, dist };
		}

		return{ false, 0.f };
	}

	CONTAINMENT_TYPE  Sphere::ContainedBy(const Frustum& Fru) const {
		// Load origin and orientation.
		vector vOrigin = load(Fru.mOrigin);
		vector vOrientation = load(Fru.mOrientation);

		// Create 6 planes (do it inline to encourage use of registers)
		__m128 Plane0 = set(0.0f, 0.0f, -1.0f, Fru.mNear);
		Plane0 = PlaneTransform(Plane0, vOrientation, vOrigin);
		Plane0 = PlaneNormalize(Plane0);

		__m128 Plane1 = set(0.0f, 0.0f, 1.0f, -Fru.mFar);
		Plane1 = PlaneTransform(Plane1, vOrientation, vOrigin);
		Plane1 = PlaneNormalize(Plane1);

		__m128 Plane2 = set(1.0f, 0.0f, -Fru.mRightSlope, 0.0f);
		Plane2 = PlaneTransform(Plane2, vOrientation, vOrigin);
		Plane2 = PlaneNormalize(Plane2);

		__m128 Plane3 = set(-1.0f, 0.0f, Fru.mLeftSlope, 0.0f);
		Plane3 = PlaneTransform(Plane3, vOrientation, vOrigin);
		Plane3 = PlaneNormalize(Plane3);

		__m128 Plane4 = set(0.0f, 1.0f, -Fru.mTopSlope, 0.0f);
		Plane4 = PlaneTransform(Plane4, vOrientation, vOrigin);
		Plane4 = PlaneNormalize(Plane4);

		__m128 Plane5 = set(0.0f, -1.0f, Fru.mBottomSlope, 0.0f);
		Plane5 = PlaneTransform(Plane5, vOrientation, vOrigin);
		Plane5 = PlaneNormalize(Plane5);


		// Load the sphere.
		__m128 vCenter = load(GetCenter());
		__m128 vRadius = Splat(GetRadius());

		// Set w of the center to one so we can dot4 with a plane.
		vCenter = Insert<0, 0, 0, 0, 1>(vCenter,SplatOne());

		__m128 Outside, Inside;

		// Test against each plane.
		FastIntersectSpherePlane(vCenter, vRadius, Plane0, Outside, Inside);

		__m128 AnyOutside = Outside;
		__m128 AllInside = Inside;

		FastIntersectSpherePlane(vCenter, vRadius, Plane1, Outside, Inside);
		AnyOutside = OrInt(AnyOutside, Outside);
		AllInside = AndInt(AllInside, Inside);

		FastIntersectSpherePlane(vCenter, vRadius, Plane2, Outside, Inside);
		AnyOutside = OrInt(AnyOutside, Outside);
		AllInside = AndInt(AllInside, Inside);

		FastIntersectSpherePlane(vCenter, vRadius, Plane3, Outside, Inside);
		AnyOutside = OrInt(AnyOutside, Outside);
		AllInside = AndInt(AllInside, Inside);

		FastIntersectSpherePlane(vCenter, vRadius, Plane4, Outside, Inside);
		AnyOutside = OrInt(AnyOutside, Outside);
		AllInside = AndInt(AllInside, Inside);

		FastIntersectSpherePlane(vCenter, vRadius, Plane5, Outside, Inside);
		AnyOutside = OrInt(AnyOutside, Outside);
		AllInside = AndInt(AllInside, Inside);

		// If the sphere is outside any plane it is outside.
		if (EqualInt(AnyOutside, details::SplatTrueInt()))
			return CONTAINMENT_TYPE::DISJOINT;

		// If the sphere is inside all planes it is inside.
		if (EqualInt(AllInside, details::SplatTrueInt()))
			return CONTAINMENT_TYPE::CONTAINS;

		// The sphere is not inside all planes or outside a plane, it may intersect.
		return CONTAINMENT_TYPE::INTERSECTS;
	}

	CONTAINMENT_TYPE  Box::ContainedBy(const Frustum& Fru) const {
		// Load origin and orientation.
		vector vOrigin = load(Fru.mOrigin);
		vector vOrientation = load(Fru.mOrientation);

		// Create 6 planes (do it inline to encourage use of registers)
		__m128 Plane0 = set(0.0f, 0.0f, -1.0f, Fru.mNear);
		Plane0 = PlaneTransform(Plane0, vOrientation, vOrigin);
		Plane0 = PlaneNormalize(Plane0);

		__m128 Plane1 = set(0.0f, 0.0f, 1.0f, -Fru.mFar);
		Plane1 = PlaneTransform(Plane1, vOrientation, vOrigin);
		Plane1 = PlaneNormalize(Plane1);

		__m128 Plane2 = set(1.0f, 0.0f, -Fru.mRightSlope, 0.0f);
		Plane2 = PlaneTransform(Plane2, vOrientation, vOrigin);
		Plane2 = PlaneNormalize(Plane2);

		__m128 Plane3 = set(-1.0f, 0.0f, Fru.mLeftSlope, 0.0f);
		Plane3 = PlaneTransform(Plane3, vOrientation, vOrigin);
		Plane3 = PlaneNormalize(Plane3);

		__m128 Plane4 = set(0.0f, 1.0f, -Fru.mTopSlope, 0.0f);
		Plane4 = PlaneTransform(Plane4, vOrientation, vOrigin);
		Plane4 = PlaneNormalize(Plane4);

		__m128 Plane5 = set(0.0f, -1.0f, Fru.mBottomSlope, 0.0f);
		Plane5 = PlaneTransform(Plane5, vOrientation, vOrigin);
		Plane5 = PlaneNormalize(Plane5);

		// Load the box.
		__m128 vCenter = load(mCenter);
		__m128 vExtents = load(mExtents);

		// Set w of the center to one so we can dot4 with a plane.
		vCenter = Insert<0, 0, 0, 0, 1>(vCenter, SplatOne());

		__m128 Outside, Inside;

		// Test against each plane.
		FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, Plane0, Outside, Inside);

		__m128 AnyOutside = Outside;
		__m128 AllInside = Inside;

		FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, Plane1, Outside, Inside);
		AnyOutside = OrInt(AnyOutside, Outside);
		AllInside = AndInt(AllInside, Inside);

		FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, Plane2, Outside, Inside);
		AnyOutside = OrInt(AnyOutside, Outside);
		AllInside = AndInt(AllInside, Inside);

		FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, Plane3, Outside, Inside);
		AnyOutside = OrInt(AnyOutside, Outside);
		AllInside = AndInt(AllInside, Inside);

		FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, Plane4, Outside, Inside);
		AnyOutside = OrInt(AnyOutside, Outside);
		AllInside = AndInt(AllInside, Inside);

		FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, Plane5, Outside, Inside);
		AnyOutside = OrInt(AnyOutside, Outside);
		AllInside = AndInt(AllInside, Inside);

		// If the box is outside any plane it is outside.
		if (EqualInt(AnyOutside, details::SplatTrueInt()))
			return CONTAINMENT_TYPE::DISJOINT;

		// If the box is inside all planes it is inside.
		if (EqualInt(AllInside, details::SplatTrueInt()))
			return CONTAINMENT_TYPE::CONTAINS;

		// The box is not inside all planes or outside a plane, it may intersect.
		return CONTAINMENT_TYPE::INTERSECTS;
	}

	CONTAINMENT_TYPE  OrientedBox::ContainedBy(const Frustum& Fru) const {
		// Load origin and orientation.
		vector vOrigin = load(Fru.mOrigin);
		vector vOrientation = load(Fru.mOrientation);

		// Create 6 planes (do it inline to encourage use of registers)
		__m128 Plane0 = set(0.0f, 0.0f, -1.0f, Fru.mNear);
		Plane0 = PlaneTransform(Plane0, vOrientation, vOrigin);
		Plane0 = PlaneNormalize(Plane0);

		__m128 Plane1 = set(0.0f, 0.0f, 1.0f, -Fru.mFar);
		Plane1 = PlaneTransform(Plane1, vOrientation, vOrigin);
		Plane1 = PlaneNormalize(Plane1);

		__m128 Plane2 = set(1.0f, 0.0f, -Fru.mRightSlope, 0.0f);
		Plane2 = PlaneTransform(Plane2, vOrientation, vOrigin);
		Plane2 = PlaneNormalize(Plane2);

		__m128 Plane3 = set(-1.0f, 0.0f, Fru.mLeftSlope, 0.0f);
		Plane3 = PlaneTransform(Plane3, vOrientation, vOrigin);
		Plane3 = PlaneNormalize(Plane3);

		__m128 Plane4 = set(0.0f, 1.0f, -Fru.mTopSlope, 0.0f);
		Plane4 = PlaneTransform(Plane4, vOrientation, vOrigin);
		Plane4 = PlaneNormalize(Plane4);

		__m128 Plane5 = set(0.0f, -1.0f, Fru.mBottomSlope, 0.0f);
		Plane5 = PlaneTransform(Plane5, vOrientation, vOrigin);
		Plane5 = PlaneNormalize(Plane5);


		// Load the box.
		__m128 vCenter = load(mCenter);
		__m128 vExtents = load(mExtents);
		__m128 BoxOrientation = load(mOrientation);

		assert(details::QuaternionIsUnit(BoxOrientation));

		// Set w of the center to one so we can dot4 with a plane.
		vCenter =Insert<0, 0, 0, 0, 1>(vCenter,SplatOne());

		// Build the 3x3 rotation matrix that defines the box axes.
		matrix R = Matrix(BoxOrientation);

		__m128 Outside, Inside;

		// Test against each plane.
		FastIntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane0, Outside, Inside);

		__m128 AnyOutside = Outside;
		__m128 AllInside = Inside;

		FastIntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane1, Outside, Inside);
		AnyOutside =OrInt(AnyOutside, Outside);
		AllInside =AndInt(AllInside, Inside);

		FastIntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane2, Outside, Inside);
		AnyOutside =OrInt(AnyOutside, Outside);
		AllInside =AndInt(AllInside, Inside);

		FastIntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane3, Outside, Inside);
		AnyOutside =OrInt(AnyOutside, Outside);
		AllInside =AndInt(AllInside, Inside);

		FastIntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane4, Outside, Inside);
		AnyOutside =OrInt(AnyOutside, Outside);
		AllInside =AndInt(AllInside, Inside);

		FastIntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane5, Outside, Inside);
		AnyOutside =OrInt(AnyOutside, Outside);
		AllInside =AndInt(AllInside, Inside);

		// If the OrientedBox is outside any plane it is outside.
		if (EqualInt(AnyOutside, details::SplatTrueInt()))
			return CONTAINMENT_TYPE::DISJOINT;

		// If the OrientedBox is inside all planes it is inside.
		if (EqualInt(AllInside, details::SplatTrueInt()))
			return CONTAINMENT_TYPE::CONTAINS;

		// The OrientedBox is not inside all planes or outside a plane, it may intersect.
		return CONTAINMENT_TYPE::INTERSECTS;
	}

	std::pair<bool, float> Triangle::Intersects(const Ray& ray) const {
		auto Direction = load(ray.mDir);

		assert(details::IsUnit<>(Direction));

		auto Zero = SplatZero();

		auto p0 = load(p[0]);
		auto p1 = load(p[1]);
		auto p2 = load(p[2]);
		auto e1 = Subtract(p1, p0);// V1 - p0;
		auto e2 = Subtract(p2, p0);// V2 - p0;

								   // p = Direction ^ e2;
		auto vp = Cross<>(Direction, e2);

		// det = e1 * p;
		auto det = Dot<>(e1, vp);

		__m128 u, v, t;

		auto Origin = load(ray.mOrigin);
		if (GreaterOrEqual<>(det, details::SplatRayEpsilon()))
		{
			// Determinate is positive (front side of the triangle).
			auto s = Subtract(Origin, p0);

			// u = s * p;
			u = Dot<>(s, vp);

			auto NoIntersection = LessExt(u, Zero);
			NoIntersection = OrInt(NoIntersection, GreaterExt(u, det));

			// q = s ^ e1;
			auto q = Cross<>(s, e1);

			// v = Direction * q;
			v = Dot<>(Direction, q);

			NoIntersection = OrInt(NoIntersection, LessExt(v, Zero));
			NoIntersection = OrInt(NoIntersection, GreaterExt(Add(u, v), det));

			// t = e2 * q;
			t = Dot<>(e2, q);

			NoIntersection = OrInt(NoIntersection, LessExt(t, Zero));

			if (EqualInt<>(NoIntersection, details::SplatTrueInt()))
			{
				return{ false, 0.f };
			}
		}
		else if (LessOrEqual<>(det, details::SplatNegRayEpsilon()))
		{
			// Determinate is negative (back side of the triangle).
			auto s = Subtract(Origin, p0);

			// u = s * p;
			u = Dot<>(s, vp);

			auto NoIntersection = GreaterExt(u, Zero);
			NoIntersection = OrInt(NoIntersection, LessExt(u, det));

			// q = s ^ e1;
			auto q = Cross<>(s, e1);

			// v = Direction * q;
			v = Dot<>(Direction, q);

			NoIntersection = OrInt(NoIntersection, GreaterExt(v, Zero));
			NoIntersection = OrInt(NoIntersection, LessExt(Add(u, v), det));

			// t = e2 * q;
			t = Dot<>(e2, q);

			NoIntersection = OrInt(NoIntersection, GreaterExt(t, Zero));

			if (EqualInt<>(NoIntersection, details::SplatTrueInt()))
			{
				return{ false, 0.f };
			}
		}
		else
		{
			// Parallel ray.
			return{ false, 0.f };
		}

		t = Divide(t, det);

		// (u / det) and (v / dev) are the barycentric cooridinates of the intersection.

		// Store the x-component to *pDist
		float dist = 0.f;
		save(dist, t);
		return{ true, dist };
	}

	Frustum&  LM_VECTOR_CALL  Frustum::operator=(matrix Projection) {
		// Corners of the projection frustum in homogenous space.
		static vectorf32 HomogenousPoints[6] =
		{
			{ 1.0f,  0.0f, 1.0f, 1.0f },   // right (at far plane)
			{ -1.0f,  0.0f, 1.0f, 1.0f },   // left
			{ 0.0f,  1.0f, 1.0f, 1.0f },   // top
			{ 0.0f, -1.0f, 1.0f, 1.0f },   // bottom

			{ 0.0f, 0.0f, 0.0f, 1.0f },     // near
			{ 0.0f, 0.0f, 1.0f, 1.0f }      // far
		};

		vector Determinant;
		matrix matInverse = Inverse(Determinant, Projection);

		// Compute the frustum corners in world space.
		vector Points[6];

		for (size_t i = 0; i < 6; ++i)
		{
			// Transform point.
			Points[i] = leo::Transform(HomogenousPoints[i], matInverse);
		}

		mOrigin = float3(0.0f, 0.0f, 0.0f);
		mOrientation = float4(0.0f, 0.0f, 0.0f, 1.0f);

		// Compute the slopes.
		Points[0] = Points[0] * Reciprocal(SplatZ(Points[0]));
		Points[1] = Points[1] * Reciprocal(SplatZ(Points[1]));
		Points[2] = Points[2] * Reciprocal(SplatZ(Points[2]));
		Points[3] = Points[3] * Reciprocal(SplatZ(Points[3]));

		mRightSlope = GetX(Points[0]);
		mLeftSlope = GetX(Points[1]);
		mTopSlope = GetY(Points[2]);
		mBottomSlope = GetY(Points[3]);

		// Compute near and far.
		Points[4] = Points[4] * Reciprocal(SplatW(Points[4]));
		Points[5] = Points[5] * Reciprocal(SplatW(Points[5]));

		mNear = GetZ(Points[4]);
		mFar = GetZ(Points[5]);

		return *this;
	}

	Frustum&  LM_VECTOR_CALL Frustum::Transform(matrix M){
		// Load the frustum.
		vector vOrigin = load(mOrigin);
		vector vOrientation = load(mOrientation);

		assert(details::QuaternionIsUnit(vOrientation));

		// Composite the frustum rotation and the transform rotation
		matrix nM;
		nM[0] = Normalize(M[0]);
		nM[1] = Normalize(M[1]);
		nM[2] = Normalize(M[2]);
		nM[3] = details::SplatR3();
		vector Rotation = Quaternion(nM);
		vOrientation = QuaternionMultiply(vOrientation, Rotation);

		// Transform the center.
		vOrigin = TransformCoord<3>(vOrigin, M);

		// Store the frustum.
		save(mOrigin, vOrigin);
		save(mOrientation, vOrientation);

		// Scale the near and far distances (the slopes remain the same).
		vector dX = Dot(M[0], M[0]);
		vector dY = Dot(M[1], M[1]);
		vector dZ = Dot(M[2], M[2]);

		vector d = max(dX,max(dY, dZ));
		float Scale = sqrtf(GetX(d));

		mNear = mNear * Scale;
		mFar = mFar * Scale;

		return *this;
	}
	Frustum&  LM_VECTOR_CALL Frustum::Transform(float Scale, vector Rotation, vector Translation) {
		assert(details::QuaternionIsUnit(Rotation));

		// Load the frustum.
		vector vOrigin = load(mOrigin);
		vector vOrientation = load(mOrientation);

		assert(details::QuaternionIsUnit(vOrientation));

		// Composite the frustum rotation and the transform rotation.
		vOrientation = QuaternionMultiply(vOrientation, Rotation);

		// Transform the origin.
		vOrigin = Rotate<3>(vOrigin *Splat(Scale), Rotation) + Translation;

		// Store the frustum.
		save(mOrigin, vOrigin);
		save(mOrientation, vOrientation);

		mNear = mNear * Scale;
		mFar = mFar * Scale;

		return *this;
	}

	//void GetCorners(float3* Corners) const;
	// Gets the 8 corners of the frustum

	CONTAINMENT_TYPE    LM_VECTOR_CALL     Frustum::Contains(vector Point) const {
		// Build frustum planes.
		vector Planes[6];
		Planes[0] = set(0.0f, 0.0f, -1.0f, mNear);
		Planes[1] = set(0.0f, 0.0f, 1.0f, -mFar);
		Planes[2] = set(1.0f, 0.0f, -mRightSlope, 0.0f);
		Planes[3] = set(-1.0f, 0.0f, mLeftSlope, 0.0f);
		Planes[4] = set(0.0f, 1.0f, -mTopSlope, 0.0f);
		Planes[5] = set(0.0f, -1.0f, mBottomSlope, 0.0f);

		// Load origin and orientation.
		vector vOrigin =load(mOrigin);
		vector vOrientation = load(mOrientation);

		assert(details::QuaternionIsUnit(vOrientation));

		// Transform point into local space of frustum.
		vector TPoint = InverseRotate<>(Point - vOrigin, vOrientation);

		// Set w to one.
		TPoint = Insert<0, 0, 0, 0, 1>(TPoint,SplatOne());

		vector Zero = SplatZero();
		vector Outside = Zero;

		// Test point against each plane of the frustum.
		for (size_t i = 0; i < 6; ++i)
		{
			vector DotR = Dot<4>(TPoint, Planes[i]);
			Outside = OrInt(Outside, GreaterExt(DotR, Zero));
		}

		return NotEqualInt(Outside,details::SplatTrueInt()) ? CONTAINMENT_TYPE::CONTAINS : CONTAINMENT_TYPE::DISJOINT;
	}
	CONTAINMENT_TYPE    LM_VECTOR_CALL     Frustum::Contains(const Triangle& Tri) const {
		// Load origin and orientation.
		vector vOrigin = load(mOrigin);
		vector vOrientation = load(mOrientation);

		// Create 6 planes (do it inline to encourage use of registers)
		__m128 NearPlane = set(0.0f, 0.0f, -1.0f, mNear);
		NearPlane = PlaneTransform(NearPlane, vOrientation, vOrigin);
		NearPlane = PlaneNormalize(NearPlane);

		__m128 FarPlane = set(0.0f, 0.0f, 1.0f, -mFar);
		FarPlane = PlaneTransform(FarPlane, vOrientation, vOrigin);
		FarPlane = PlaneNormalize(FarPlane);

		__m128 RightPlane = set(1.0f, 0.0f, -mRightSlope, 0.0f);
		RightPlane = PlaneTransform(RightPlane, vOrientation, vOrigin);
		RightPlane = PlaneNormalize(RightPlane);

		__m128 LeftPlane = set(-1.0f, 0.0f, mLeftSlope, 0.0f);
		LeftPlane = PlaneTransform(LeftPlane, vOrientation, vOrigin);
		LeftPlane = PlaneNormalize(LeftPlane);

		__m128 TopPlane = set(0.0f, 1.0f, -mTopSlope, 0.0f);
		TopPlane = PlaneTransform(TopPlane, vOrientation, vOrigin);
		TopPlane = PlaneNormalize(TopPlane);

		__m128 BottomPlane = set(0.0f, -1.0f, mBottomSlope, 0.0f);
		BottomPlane = PlaneTransform(BottomPlane, vOrientation, vOrigin);
		BottomPlane = PlaneNormalize(BottomPlane);

		return CONTAINMENT_TYPE(TriangleTests::ContainedBy(load(Tri.p[0]), load(Tri.p[0]), load(Tri.p[0]), NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane));
	}
	CONTAINMENT_TYPE Frustum::Contains(const Sphere& sp) const {
		return sp.ContainedBy(*this);
	}
	CONTAINMENT_TYPE Frustum::Contains(const Box& box) const {
		return box.ContainedBy(*this);
	}
	CONTAINMENT_TYPE Frustum::Contains(const OrientedBox& box) const {
		return box.ContainedBy(*this);
	}
	//CONTAINMENT_TYPE Contains(const Frustum& fr) const;
	// Frustum-Frustum test

	bool Frustum::Intersects(const Sphere& sh) const {
		auto Zero = SplatZero();

		// Build frustum planes.
		vector Planes[6];
		Planes[0] = set(0.0f, 0.0f, -1.0f, mNear);
		Planes[1] = set(0.0f, 0.0f, 1.0f, -mFar);
		Planes[2] = set(1.0f, 0.0f, -mRightSlope, 0.0f);
		Planes[3] = set(-1.0f, 0.0f, mLeftSlope, 0.0f);
		Planes[4] = set(0.0f, 1.0f, -mTopSlope, 0.0f);
		Planes[5] = set(0.0f, -1.0f, mBottomSlope, 0.0f);

		// Load origin and orientation.
		vector vOrigin = load(mOrigin);
		vector vOrientation = load(mOrientation);

		assert(details::QuaternionIsUnit(vOrientation));

		// Load the sphere.
		__m128 vCenter = load(sh.GetCenter());
		__m128 vRadius = Splat(sh.GetRadius());

		// Transform the center of the sphere into the local space of frustum.
		vCenter = InverseRotate<>(vCenter - vOrigin, vOrientation);

		// Set w of the center to one so we can dot4 with the plane.
		vCenter = Insert<0, 0, 0, 0, 1>(vCenter, SplatOne());

		// Check against each plane of the frustum.
		__m128 Outside = details::SplatFalseInt();
		__m128 InsideAll =details::SplatTrueInt();
		__m128 CenterInsideAll = details::SplatTrueInt();

		__m128 Dist[6];

		for (size_t i = 0; i < 6; ++i)
		{
			Dist[i] = Dot<4>(vCenter, Planes[i]);

			// Outside the plane?
			Outside = OrInt(Outside, GreaterExt(Dist[i], vRadius));

			// Fully inside the plane?
			InsideAll = AndInt(InsideAll, LessOrEqualExt(Dist[i], -vRadius));

			// Check if the center is inside the plane.
			CenterInsideAll = AndInt(CenterInsideAll, LessOrEqualExt(Dist[i], Zero));
		}

		// If the sphere is outside any of the planes it is outside. 
		if (EqualInt(Outside, details::SplatTrueInt()))
			return false;

		// If the sphere is inside all planes it is fully inside.
		if (EqualInt(InsideAll, details::SplatTrueInt()))
			return true;

		// If the center of the sphere is inside all planes and the sphere intersects 
		// one or more planes then it must intersect.
		if (EqualInt(CenterInsideAll, details::SplatTrueInt()))
			return true;

		// The sphere may be outside the frustum or intersecting the frustum.
		// Find the nearest feature (face, edge, or corner) on the frustum 
		// to the sphere.

		// The faces adjacent to each face are:
		static const size_t adjacent_faces[6][4] =
		{
			{ 2, 3, 4, 5 },    // 0
			{ 2, 3, 4, 5 },    // 1
			{ 0, 1, 4, 5 },    // 2
			{ 0, 1, 4, 5 },    // 3
			{ 0, 1, 2, 3 },    // 4
			{ 0, 1, 2, 3 }
		};  // 5

		__m128 Intersects = details::SplatFalseInt();

		// Check to see if the nearest feature is one of the planes.
		for (size_t i = 0; i < 6; ++i)
		{
			// Find the nearest point on the plane to the center of the sphere.
			__m128 Point = vCenter - (Planes[i] * Dist[i]);

			// Set w of the point to one.
			Point = Insert<0, 0, 0, 0, 1>(Point, SplatOne());

			// If the point is inside the face (inside the adjacent planes) then
			// this plane is the nearest feature.
			__m128 InsideFace = details::SplatTrueInt();

			for (size_t j = 0; j < 4; j++)
			{
				size_t plane_index = adjacent_faces[i][j];

				InsideFace = AndInt(InsideFace,
					LessOrEqualExt(Dot<4>(Point, Planes[plane_index]), Zero));
			}

			// Since we have already checked distance from the plane we know that the
			// sphere must intersect if this plane is the nearest feature.
			Intersects = OrInt(Intersects,
				AndInt(GreaterExt(Dist[i], Zero), InsideFace));
		}

		if (EqualInt(Intersects, details::SplatTrueInt()))
			return true;

		// Build the corners of the frustum.
		__m128 vRightTop = set(mRightSlope, mTopSlope, 1.0f, 0.0f);
		__m128 vRightBottom = set(mRightSlope, mBottomSlope, 1.0f, 0.0f);
		__m128 vLeftTop = set(mLeftSlope, mTopSlope, 1.0f, 0.0f);
		__m128 vLeftBottom = set(mLeftSlope, mBottomSlope, 1.0f, 0.0f);
		__m128 vNear = Splat(mNear);
		__m128 vFar = Splat(mFar);

		__m128 Corners[CORNER_COUNT];
		Corners[0] = vRightTop * vNear;
		Corners[1] = vRightBottom * vNear;
		Corners[2] = vLeftTop * vNear;
		Corners[3] = vLeftBottom * vNear;
		Corners[4] = vRightTop * vFar;
		Corners[5] = vRightBottom * vFar;
		Corners[6] = vLeftTop * vFar;
		Corners[7] = vLeftBottom * vFar;

		// The Edges are:
		static const size_t edges[12][2] =
		{
			{ 0, 1 },{ 2, 3 },{ 0, 2 },{ 1, 3 },    // Near plane
			{ 4, 5 },{ 6, 7 },{ 4, 6 },{ 5, 7 },    // Far plane
			{ 0, 4 },{ 1, 5 },{ 2, 6 },{ 3, 7 },
		}; // Near to far

		__m128 RadiusSq = vRadius * vRadius;

		// Check to see if the nearest feature is one of the edges (or corners).
		for (size_t i = 0; i < 12; ++i)
		{
			size_t ei0 = edges[i][0];
			size_t ei1 = edges[i][1];

			// Find the nearest point on the edge to the center of the sphere.
			// The corners of the frustum are included as the endpoints of the edges.
			__m128 Point = PointOnLineSegmentNearestPoint(Corners[ei0], Corners[ei1], vCenter);

			__m128 Delta = vCenter - Point;

			__m128 DistSq = Dot(Delta, Delta);

			// If the distance to the center of the sphere to the point is less than 
			// the radius of the sphere then it must intersect.
			Intersects = OrInt(Intersects, LessOrEqualExt(DistSq, RadiusSq));
		}

		if (EqualInt(Intersects,details::SplatTrueInt()))
			return true;

		// The sphere must be outside the frustum.
		return false;
	}
	bool Frustum::Intersects(const Box& box) const {
		// Make the axis aligned box oriented and do an OBB vs frustum test.
		OrientedBox obox{ box };
		return Intersects(obox);
	}
	bool Frustum::Intersects(const OrientedBox& box) const {
		static const vectori32 SelectY =
		{
			LM_SELECT_0,LM_SELECT_1, LM_SELECT_0, LM_SELECT_0
		};
		static const vectori32 SelectZ =
		{
			LM_SELECT_0, LM_SELECT_0, LM_SELECT_1, LM_SELECT_0
		};

		auto Zero = SplatZero();

		// Build frustum planes.
		vector Planes[6];
		Planes[0] = set(0.0f, 0.0f, -1.0f, mNear);
		Planes[1] = set(0.0f, 0.0f, 1.0f, -mFar);
		Planes[2] = set(1.0f, 0.0f, -mRightSlope, 0.0f);
		Planes[3] = set(-1.0f, 0.0f, mLeftSlope, 0.0f);
		Planes[4] = set(0.0f, 1.0f, -mTopSlope, 0.0f);
		Planes[5] = set(0.0f, -1.0f, mBottomSlope, 0.0f);

		// Load origin and orientation.
		vector vOrigin = load(mOrigin);
		vector FrustumOrientation = load(mOrientation);

		assert(details::QuaternionIsUnit(FrustumOrientation));

		// Load the box.
		vector Center = load(box.mCenter);
		vector Extents = load(box.mExtents);
		vector BoxOrientation = load(box.mOrientation);

		assert(details::QuaternionIsUnit(BoxOrientation));

		// Transform the oriented box into the space of the frustum in order to 
		// minimize the number of transforms we have to do.
		Center =InverseRotate<>(Center - vOrigin, FrustumOrientation);
		BoxOrientation = QuaternionMultiply(BoxOrientation, QuaternionConjugate(FrustumOrientation));

		// Set w of the center to one so we can dot4 with the plane.
		Center = Insert<0, 0, 0, 0, 1>(Center, SplatOne());

		// Build the 3x3 rotation matrix that defines the box axes.
		matrix R = Matrix(BoxOrientation);

		// Check against each plane of the frustum.
		__m128 Outside = details::SplatFalseInt();
		__m128 InsideAll = details::SplatTrueInt();
		__m128 CenterInsideAll = details::SplatTrueInt();

		for (size_t i = 0; i < 6; ++i)
		{
			// Compute the distance to the center of the box.
			__m128 Dist = Dot<4>(Center, Planes[i]);

			// Project the axes of the box onto the normal of the plane.  Half the
			// length of the projection (sometime called the "radius") is equal to
			// h(u) * abs(n dot b(u))) + h(v) * abs(n dot b(v)) + h(w) * abs(n dot b(w))
			// where h(i) are extents of the box, n is the plane normal, and b(i) are the 
			// axes of the box.
			__m128 Radius = Dot<>(Planes[i], R[0]);
			Radius = Select(Radius, Dot<>(Planes[i], R[1]), SelectY);
			Radius = Select(Radius, Dot<>(Planes[i], R[2]), SelectZ);
			Radius = Dot<>(Extents, Abs(Radius));

			// Outside the plane?
			Outside = OrInt(Outside, GreaterExt(Dist, Radius));

			// Fully inside the plane?
			InsideAll = AndInt(InsideAll, LessOrEqualExt(Dist, -Radius));

			// Check if the center is inside the plane.
			CenterInsideAll = AndInt(CenterInsideAll, LessOrEqualExt(Dist, Zero));
		}

		// If the sphere is outside any of the planes it is outside. 
		if (EqualInt(Outside, details::SplatTrueInt()))
			return false;

		// If the sphere is inside all planes it is fully inside.
		if (EqualInt(InsideAll, details::SplatTrueInt()))
			return true;

		// If the center of the sphere is inside all planes and the sphere intersects 
		// one or more planes then it must intersect.
		if (EqualInt(CenterInsideAll, details::SplatTrueInt()))
			return true;

		// Build the corners of the frustum.
		__m128 vRightTop = set(mRightSlope, mTopSlope, 1.0f, 0.0f);
		__m128 vRightBottom = set(mRightSlope, mBottomSlope, 1.0f, 0.0f);
		__m128 vLeftTop = set(mLeftSlope, mTopSlope, 1.0f, 0.0f);
		__m128 vLeftBottom = set(mLeftSlope, mBottomSlope, 1.0f, 0.0f);
		__m128 vNear = Splat(mNear);
		__m128 vFar = Splat(mFar);

		__m128 Corners[CORNER_COUNT];
		Corners[0] = vRightTop * vNear;
		Corners[1] = vRightBottom * vNear;
		Corners[2] = vLeftTop * vNear;
		Corners[3] = vLeftBottom * vNear;
		Corners[4] = vRightTop * vFar;
		Corners[5] = vRightBottom * vFar;
		Corners[6] = vLeftTop * vFar;
		Corners[7] = vLeftBottom * vFar;

		// Test against box axes (3)
		{
			// Find the min/max values of the projection of the frustum onto each axis.
			__m128 FrustumMin, FrustumMax;

			FrustumMin = Dot<>(Corners[0], R[0]);
			FrustumMin = Select(FrustumMin, Dot<>(Corners[0], R[1]), SelectY);
			FrustumMin = Select(FrustumMin, Dot<>(Corners[0], R[2]), SelectZ);
			FrustumMax = FrustumMin;

			for (size_t i = 1; i < OrientedBox::CORNER_COUNT; ++i)
			{
				__m128 Temp = Dot<>(Corners[i], R[0]);
				Temp = Select(Temp, Dot<>(Corners[i], R[1]), SelectY);
				Temp = Select(Temp, Dot<>(Corners[i], R[2]), SelectZ);

				FrustumMin = min(FrustumMin, Temp);
				FrustumMax = max(FrustumMax, Temp);
			}

			// Project the center of the box onto the axes.
			__m128 BoxDist = Dot<>(Center, R[0]);
			BoxDist = Select(BoxDist, Dot<>(Center, R[1]), SelectY);
			BoxDist = Select(BoxDist, Dot<>(Center, R[2]), SelectZ);

			// The projection of the box onto the axis is just its Center and Extents.
			// if (min > box_max || max < box_min) reject;
			__m128 Result = OrInt(GreaterExt(FrustumMin, BoxDist + Extents),
				LessExt(FrustumMax, BoxDist - Extents));

			if (details::AnyTrue<>(Result))
				return false;
		}


		// Test against edge/edge axes (3*6).
		__m128 FrustumEdgeAxis[6];

		FrustumEdgeAxis[0] = vRightTop;
		FrustumEdgeAxis[1] = vRightBottom;
		FrustumEdgeAxis[2] = vLeftTop;
		FrustumEdgeAxis[3] = vLeftBottom;
		FrustumEdgeAxis[4] = vRightTop - vLeftTop;
		FrustumEdgeAxis[5] = vLeftBottom - vLeftTop;

		for (size_t i = 0; i < 3; ++i)
		{
			for (size_t j = 0; j < 6; j++)
			{
				// Compute the axis we are going to test.
				__m128 Axis = Cross<>(R[i], FrustumEdgeAxis[j]);

				// Find the min/max values of the projection of the frustum onto the axis.
				__m128 FrustumMin, FrustumMax;

				FrustumMin = FrustumMax = Dot<>(Axis, Corners[0]);

				for (size_t k = 1; k < CORNER_COUNT; k++)
				{
					__m128 Temp = Dot<>(Axis, Corners[k]);
					FrustumMin = min(FrustumMin, Temp);
					FrustumMax = max(FrustumMax, Temp);
				}

				// Project the center of the box onto the axis.
				__m128 Dist = Dot<>(Center, Axis);

				// Project the axes of the box onto the axis to find the "radius" of the box.
				__m128 Radius = Dot<>(Axis, R[0]);
				Radius = Select(Radius, Dot<>(Axis, R[1]), SelectY);
				Radius = Select(Radius, Dot<>(Axis, R[2]), SelectZ);
				Radius = Dot<>(Extents, Abs(Radius));

				// if (center > max + radius || center < min - radius) reject;
				Outside = OrInt(Outside, GreaterExt(Dist, FrustumMax + Radius));
				Outside = OrInt(Outside, LessExt(Dist, FrustumMin - Radius));
			}
		}

		if (EqualInt(Outside,details::SplatTrueInt()))
			return false;

		// If we did not find a separating plane then the box must intersect the frustum.
		return true;
	}

	//bool Frustum::Intersects(const Frustum& fr) const {}

	bool Frustum::Intersects(const Triangle& Tri) const {
		// Build frustum planes.
		vector Planes[6];
		Planes[0] = set(0.0f, 0.0f, -1.0f, mNear);
		Planes[1] = set(0.0f, 0.0f, 1.0f, -mFar);
		Planes[2] = set(1.0f, 0.0f, -mRightSlope, 0.0f);
		Planes[3] = set(-1.0f, 0.0f, mLeftSlope, 0.0f);
		Planes[4] = set(0.0f, 1.0f, -mTopSlope, 0.0f);
		Planes[5] = set(0.0f, -1.0f, mBottomSlope, 0.0f);

		// Load origin and orientation.
		vector vOrigin = load(mOrigin);
		vector vOrientation = load(mOrientation);

		assert(details::QuaternionIsUnit(vOrientation));

		auto V0 = load(Tri.p[0]);
		auto V1 = load(Tri.p[1]);
		auto V2 = load(Tri.p[2]);

		// Transform triangle into the local space of frustum.
		vector TV0 = InverseRotate<>(V0 - vOrigin, vOrientation);
		vector TV1 = InverseRotate<>(V1 - vOrigin, vOrientation);
		vector TV2 = InverseRotate<>(V2 - vOrigin, vOrientation);

		// Test each vertex of the triangle against the frustum planes.
		vector Outside = details::SplatFalseInt();
		vector InsideAll = details::SplatTrueInt();

		for (size_t i = 0; i < 6; ++i)
		{
			vector Dist0 = Dot<>(TV0, Planes[i]);
			vector Dist1 = Dot<>(TV1, Planes[i]);
			vector Dist2 = Dot<>(TV2, Planes[i]);

			vector MinDist = min(Dist0, Dist1);
			MinDist = min(MinDist, Dist2);
			vector MaxDist = max(Dist0, Dist1);
			MaxDist = max(MaxDist, Dist2);

			vector PlaneDist = SplatW(Planes[i]);

			// Outside the plane?
			Outside = OrInt(Outside, GreaterExt(MinDist, PlaneDist));

			// Fully inside the plane?
			InsideAll = AndInt(InsideAll, LessOrEqualExt(MaxDist, PlaneDist));
		}

		// If the triangle is outside any of the planes it is outside. 
		if (EqualInt(Outside, details::SplatTrueInt()))
			return false;

		// If the triangle is inside all planes it is fully inside.
		if (EqualInt(InsideAll, details::SplatTrueInt()))
			return true;

		// Build the corners of the frustum.
		__m128 vRightTop = set(mRightSlope, mTopSlope, 1.0f, 0.0f);
		__m128 vRightBottom = set(mRightSlope, mBottomSlope, 1.0f, 0.0f);
		__m128 vLeftTop = set(mLeftSlope, mTopSlope, 1.0f, 0.0f);
		__m128 vLeftBottom = set(mLeftSlope, mBottomSlope, 1.0f, 0.0f);
		__m128 vNear = Splat(mNear);
		__m128 vFar = Splat(mFar);

		vector Corners[CORNER_COUNT];
		Corners[0] = vRightTop * vNear;
		Corners[1] = vRightBottom * vNear;
		Corners[2] = vLeftTop * vNear;
		Corners[3] = vLeftBottom * vNear;
		Corners[4] = vRightTop * vFar;
		Corners[5] = vRightBottom * vFar;
		Corners[6] = vLeftTop * vFar;
		Corners[7] = vLeftBottom * vFar;

		// Test the plane of the triangle.
		vector Normal = Cross<>(V1 - V0, V2 - V0);
		vector Dist = Dot<>(Normal, V0);

		vector MinDist, MaxDist;
		MinDist = MaxDist = Dot<>(Corners[0], Normal);
		for (size_t i = 1; i < CORNER_COUNT; ++i)
		{
			vector Temp = Dot<>(Corners[i], Normal);
			MinDist = min(MinDist, Temp);
			MaxDist = max(MaxDist, Temp);
		}

		Outside = OrInt(GreaterExt(MinDist, Dist), LessExt(MaxDist, Dist));
		if (EqualInt(Outside, details::SplatTrueInt()))
			return false;

		// Check the edge/edge axes (3*6).
		vector TriangleEdgeAxis[3];
		TriangleEdgeAxis[0] = V1 - V0;
		TriangleEdgeAxis[1] = V2 - V1;
		TriangleEdgeAxis[2] = V0 - V2;

		vector FrustumEdgeAxis[6];
		FrustumEdgeAxis[0] = vRightTop;
		FrustumEdgeAxis[1] = vRightBottom;
		FrustumEdgeAxis[2] = vLeftTop;
		FrustumEdgeAxis[3] = vLeftBottom;
		FrustumEdgeAxis[4] = vRightTop - vLeftTop;
		FrustumEdgeAxis[5] = vLeftBottom - vLeftTop;

		for (size_t i = 0; i < 3; ++i)
		{
			for (size_t j = 0; j < 6; j++)
			{
				// Compute the axis we are going to test.
				vector Axis = Cross<>(TriangleEdgeAxis[i], FrustumEdgeAxis[j]);

				// Find the min/max of the projection of the triangle onto the axis.
				vector MinA, MaxA;

				vector Dist0 = Dot<>(V0, Axis);
				vector Dist1 = Dot<>(V1, Axis);
				vector Dist2 = Dot<>(V2, Axis);

				MinA = min(Dist0, Dist1);
				MinA = min(MinA, Dist2);
				MaxA = max(Dist0, Dist1);
				MaxA = max(MaxA, Dist2);

				// Find the min/max of the projection of the frustum onto the axis.
				vector MinB, MaxB;

				MinB = MaxB = Dot<>(Axis, Corners[0]);

				for (size_t k = 1; k < CORNER_COUNT; k++)
				{
					vector Temp = Dot<>(Axis, Corners[k]);
					MinB = min(MinB, Temp);
					MaxB = max(MaxB, Temp);
				}

				// if (MinA > MaxB || MinB > MaxA) reject;
				Outside = OrInt(Outside, GreaterExt(MinA, MaxB));
				Outside = OrInt(Outside, GreaterExt(MinB, MaxA));
			}
		}

		if (EqualInt(Outside, details::SplatTrueInt()))
			return false;

		// If we did not find a separating plane then the triangle must intersect the frustum.
		return true;
	}
	PLANE_INTERSECTION_TYPE    LM_VECTOR_CALL    Frustum::Intersects(vector Plane) const {
		assert(details::PlaneIsUnit(Plane));

		// Load origin and orientation of the frustum.
		vector vOrigin = load(mOrigin);
		vector vOrientation = load(mOrientation);

		assert(details::QuaternionIsUnit(vOrientation));

		// Set w of the origin to one so we can dot4 with a plane.
		vOrigin = Insert<0, 0, 0, 0, 1>(vOrigin, SplatOne());

		// Build the corners of the frustum.
		__m128 vRightTop = set(mRightSlope, mTopSlope, 1.0f, 0.0f);
		__m128 vRightBottom = set(mRightSlope, mBottomSlope, 1.0f, 0.0f);
		__m128 vLeftTop = set(mLeftSlope, mTopSlope, 1.0f, 0.0f);
		__m128 vLeftBottom = set(mLeftSlope, mBottomSlope, 1.0f, 0.0f);
		__m128 vNear = Splat(mNear);
		__m128 vFar = Splat(mFar);

		vRightTop = Rotate<>(vRightTop, vOrientation);
		vRightBottom = Rotate<>(vRightBottom, vOrientation);
		vLeftTop = Rotate<>(vLeftTop, vOrientation);
		vLeftBottom = Rotate<>(vLeftBottom, vOrientation);


		__m128 Corners[CORNER_COUNT];
		Corners[0] = vOrigin+ vRightTop * vNear;
		Corners[1] = vOrigin+vRightBottom * vNear;
		Corners[2] = vOrigin+vLeftTop * vNear;
		Corners[3] = vOrigin+vLeftBottom * vNear;
		Corners[4] = vOrigin+vRightTop * vFar;
		Corners[5] = vOrigin+vRightBottom * vFar;
		Corners[6] = vOrigin+vLeftTop * vFar;
		Corners[7] = vOrigin+vLeftBottom * vFar;

		
		__m128 Outside, Inside;
		FastIntersectFrustumPlane(Corners[0], Corners[1], Corners[2], Corners[3],
			Corners[4], Corners[5], Corners[6], Corners[7],
			Plane, Outside, Inside);

		// If the frustum is outside any plane it is outside.
		if (EqualInt(Outside, details::SplatTrueInt()))
			return PLANE_INTERSECTION_TYPE::FRONT;

		// If the frustum is inside all planes it is inside.
		if (EqualInt(Inside, details::SplatTrueInt()))
			return PLANE_INTERSECTION_TYPE::BACK;

		// The frustum is not inside all planes or outside a plane it intersects.
		return PLANE_INTERSECTION_TYPE::INTERSECTING;
	}
	std::pair<bool, float>   Frustum::Intersects(const Ray& ray) const {
		auto rayOrigin = load(ray.mOrigin);
		auto Direction = load(ray.mDir);
		// If ray starts inside the frustum, return a distance of 0 for the hit
		if (Contains(rayOrigin) == CONTAINMENT_TYPE::CONTAINS)
		{
			return std::make_pair(true, 0.f);
		}

		// Build frustum planes.
		vector Planes[6];
		Planes[0] = set(0.0f, 0.0f, -1.0f, mNear);
		Planes[1] = set(0.0f, 0.0f, 1.0f, -mFar);
		Planes[2] = set(1.0f, 0.0f, -mRightSlope, 0.0f);
		Planes[3] = set(-1.0f, 0.0f, mLeftSlope, 0.0f);
		Planes[4] = set(0.0f, 1.0f, -mTopSlope, 0.0f);
		Planes[5] = set(0.0f, -1.0f, mBottomSlope, 0.0f);

		// Load origin and orientation of the frustum.
		vector frOrigin = load(mOrigin);
		vector frOrientation = load(mOrientation);

		// This algorithm based on "Fast Ray-Convex Polyhedron Intersectin," in James Arvo, ed., Graphics Gems II pp. 247-250
		float tnear = -FLT_MAX;
		float tfar = FLT_MAX;

		for (size_t i = 0; i < 6; ++i)
		{
			vector Plane = PlaneTransform(Planes[i], frOrientation, frOrigin);
			Plane = PlaneNormalize(Plane);

			vector AxisDotOrigin = PlaneDotCoord(Plane, rayOrigin);
			vector AxisDotDirection = Dot<>(Plane, Direction);

			if (LessOrEqual<>(Abs(AxisDotDirection),details::SplatRayEpsilon()))
			{
				// Ray is parallel to plane - check if ray origin is inside plane's
				if (Greater<>(AxisDotOrigin, SplatZero()))
				{
					// Ray origin is outside half-space.
					return std::make_pair(false, 0.f);
				}
			}
			else
			{
				// Ray not parallel - get distance to plane.
				float vd = GetX(AxisDotDirection);
				float vn = GetX(AxisDotOrigin);
				float t = -vn / vd;
				if (vd < 0.0f)
				{
					// Front face - T is a near point.
					if (t > tfar)
					{
						return std::make_pair(false, 0.f);
					}
					if (t > tnear)
					{
						// Hit near face.
						tnear = t;
					}
				}
				else
				{
					// back face - T is far point.
					if (t < tnear)
					{
						return std::make_pair(false, 0.f);
					}
					if (t < tfar)
					{
						// Hit far face.
						tfar = t;
					}
				}
			}
		}

		// Survived all tests.
		// Note: if ray originates on polyhedron, may want to change 0.0f to some
		// epsilon to avoid intersecting the originating face.
		float distance = (tnear >= 0.0f) ? tnear : tfar;
		if (distance >= 0.0f)
		{
			return std::make_pair(true,distance);
		}

		return std::make_pair(false, 0.f);
	}

	//CONTAINMENT_TYPE     LM_VECTOR_CALL     Frustum::ContainedBy(vector Plane0, vector Plane1, vector Plane2,
	//	const vector& Plane3, const vector& Plane4, const vector& Plane5) const {
	//
	//}
}