#include "LeoMath.h"
#include "leo_math_impl.hpp"

#include "Geometry.hpp"
#include "Geometry_impl.hpp"



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

		return TriangleTests::ContainedBy(load(Tri.p[0]), load(Tri.p[0]), load(Tri.p[0]), NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
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

	}
	bool Frustum::Intersects(const Box& box) const {

	}
	bool Frustum::Intersects(const OrientedBox& box) const {

	}

	//bool Frustum::Intersects(const Frustum& fr) const {}

	bool LM_VECTOR_CALL	Frustum::Intersects(const Triangle& Tri) const {

	}
	PLANE_INTERSECTION_TYPE    LM_VECTOR_CALL    Frustum::Intersects(vector Plane) const {

	}
	std::pair<bool, float>    LM_VECTOR_CALL     Frustum::Intersects(const Ray& ray) const {

	}

	//CONTAINMENT_TYPE     LM_VECTOR_CALL     Frustum::ContainedBy(vector Plane0, vector Plane1, vector Plane2,
	//	const vector& Plane3, const vector& Plane4, const vector& Plane5) const {
	//
	//}
}