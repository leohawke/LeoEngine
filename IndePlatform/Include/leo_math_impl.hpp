////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/leo_math_impl.hpp
//  Version:     v1.00
//  Created:     11/30/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 几何体
// -------------------------------------------------------------------------
//  History:
//		
//
////////////////////////////////////////////////////////////////////////////

#ifndef IndePlatform_LeoMathutility_hpp
#error "不要包含实现头文件"
#endif

#ifndef IndePlatform_LeoMath_impl_hpp
#define IndePlatform_LeoMath_impl_hpp

#include "leomathutility.hpp"



namespace leo {

#ifdef LB_IMPL_MSCPP
	inline __m128  operator * (__m128 lhs, __m128 rhs) {
		return leo::Multiply(lhs, rhs);
	}

	inline __m128  operator - (__m128 lhs, __m128 rhs) {
		return leo::Subtract(lhs, rhs);
	}

	inline __m128  operator + (__m128 lhs, __m128 rhs) {
		return leo::Add(lhs, rhs);
	}

	inline __m128  operator - (__m128 lhs) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vnegq_f32(V);
#elif defined(LM_SSE_INTRINSICS)
		__m128 Z;

		Z = _mm_setzero_ps();

		return _mm_sub_ps(Z, lhs);
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS
	}
#endif
	

	inline float GetX(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vgetq_lane_f32(v, 0);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cvtss_f32(v);
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS_
	}

	inline float GetY(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vgetq_lane_f32(v, 0);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cvtss_f32(LM_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1)));
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS_
	}

	inline float GetZ(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vgetq_lane_f32(v, 0);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cvtss_f32(LM_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2)));
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS_
	}

	inline float GetW(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vgetq_lane_f32(v, 0);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cvtss_f32(LM_PERMUTE_PS(v, _MM_SHUFFLE(3, 3, 3, 3)));
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS_
	}

	namespace details {
		inline bool QuaternionIsUnit(__m128 Q)
		{
			static const vectorf32 UintQuaternionEpsilon = { 1.0e-4f, 1.0e-4f, 1.0e-4f, 1.0e-4f };

			__m128 Difference = Length<4>(Q) - SplatOne();
			return Less(Abs(Difference), UintQuaternionEpsilon);
		}

		inline __m128 LM_VECTOR_CALL SplatFalseInt() {
#if defined(LM_ARM_NEON_INTRINSICS)
			return vdupq_n_u32(0);
#elif defined(LM_SSE_INTRINSICS)
			return _mm_setzero_ps();
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS_
		}

		template<uint8 D = 3>
		inline bool AnyTrue(__m128 V) {
			__m128 C = Swizzle(V, LM_SWIZZLE_X, LM_SWIZZLE_Y, LM_SWIZZLE_Z, LM_SWIZZLE_X);
			return ComparisonAnyTrue(EqualIntR<>(C, SplatTrueInt()));
		}

		const static vectorf32 g_UnitPlaneEpsilon = { 1.0e-4f, 1.0e-4f, 1.0e-4f, 1.0e-4f };

		inline bool PlaneIsUnit(__m128 Plane)
		{
			__m128 Difference = Length<>(Plane)-SplatOne();
			return Less(Abs(Difference), g_UnitPlaneEpsilon);
		}
	}

	__m128 LM_VECTOR_CALL PlaneTransform(__m128 P, __m128 Q, __m128 O) {
		__m128 vNormal = Rotate<>(P, Q);
		__m128 vD = SplatW(P) - Dot<>(vNormal, O);

		return Insert<0, 0, 0, 0, 1>(vNormal, vD);
	}



	template<typename T, uint8 slot>
	struct sse_param_type {
		using type = const T&;
	};

	template<typename T>
	struct sse_param_type<T, 1> {
		using type = T;
	};

	template<typename T>
	struct sse_param_type<T, 2> {
		using type = T;
	};

	template<typename T>
	struct sse_param_type<T, 3> {
		using type = T;
	};

	template<typename T>
	//pass in - register for ARM, Xbox 360, and x64
	struct sse_param_type<T, 4> {
#if defined(ARCH_AMD64) || defined(ARCH_XENON) ||defined(ARCH_ARM)
		using type = T;
#else
		using type = const T&;
#endif
	};

	template<typename T>
	struct sse_param_type<T, 5> {
#if defined(ARCH_XENON)
		using type = T;
#else
		using type = const T&;
#endif
	};

	template<typename T>
	struct sse_param_type<T, 6> {
#if defined(ARCH_XENON)
		using type = T;
#else
		using type = const T&;
#endif
	};

	template<typename T, uint8 slot>
	using sse_param_type_t =typename sse_param_type<T, slot>::type;

	namespace TriangleTests {

		void inline LM_VECTOR_CALL FastIntersectTrianglePlane(__m128 V0, __m128 V1, __m128 V2, sse_param_type_t<__m128, 4> Plane, __m128 & Outside, __m128& Inside) {
			// Plane0
			__m128 Dist0 = Dot<4>(V0, Plane);
			__m128 Dist1 = Dot<4>(V1, Plane);
			__m128 Dist2 = Dot<4>(V2, Plane);

			__m128 MinDist = min(Dist0, Dist1);
			MinDist = min(MinDist, Dist2);

			__m128 MaxDist = min(Dist0, Dist1);
			MaxDist = max(MaxDist, Dist2);

			__m128 Zero = SplatZero();

			// Outside the plane?
			Outside = GreaterExt(MinDist, Zero);

			// Fully inside the plane?
			Inside = LessExt(MaxDist, Zero);
		}

		inline uint8_t LM_VECTOR_CALL ContainedBy(__m128 V0, __m128 V1, __m128 V2,
			sse_param_type_t<__m128, 4> Plane0, sse_param_type_t<__m128, 5> Plane1, sse_param_type_t<__m128, 6> Plane2,
			const __m128& Plane3, const __m128& Plane4, const __m128& Plane5)
		{

			__m128 One = SplatOne();

			// Set w of the points to one so we can dot4 with a plane.
			__m128 TV0 = Insert<0, 0, 0, 0, 1>(V0, One);
			__m128 TV1 = Insert<0, 0, 0, 0, 1>(V1, One);
			__m128 TV2 = Insert<0, 0, 0, 0, 1>(V2, One);

			__m128 Outside, Inside;

			// Test against each plane.
			FastIntersectTrianglePlane(TV0, TV1, TV2, Plane0, Outside, Inside);

			__m128 AnyOutside = Outside;
			__m128 AllInside = Inside;

			FastIntersectTrianglePlane(TV0, TV1, TV2, Plane1, Outside, Inside);
			AnyOutside = OrInt(AnyOutside, Outside);
			AllInside = AndInt(AllInside, Inside);

			FastIntersectTrianglePlane(TV0, TV1, TV2, Plane2, Outside, Inside);
			AnyOutside = OrInt(AnyOutside, Outside);
			AllInside = AndInt(AllInside, Inside);

			FastIntersectTrianglePlane(TV0, TV1, TV2, Plane3, Outside, Inside);
			AnyOutside = OrInt(AnyOutside, Outside);
			AllInside = AndInt(AllInside, Inside);

			FastIntersectTrianglePlane(TV0, TV1, TV2, Plane4, Outside, Inside);
			AnyOutside = OrInt(AnyOutside, Outside);
			AllInside = AndInt(AllInside, Inside);

			FastIntersectTrianglePlane(TV0, TV1, TV2, Plane5, Outside, Inside);
			AnyOutside = OrInt(AnyOutside, Outside);
			AllInside = AndInt(AllInside, Inside);

			// If the triangle is outside any plane it is outside.
			if (EqualInt(AnyOutside, details::SplatTrueInt()))
				return 0;

			// If the triangle is inside all planes it is inside.
			if (EqualInt(AllInside, details::SplatTrueInt()))
				return 2;

			// The triangle is not inside all planes or outside a plane, it may intersect.
			return 1;
		}
	}

	void inline LM_VECTOR_CALL FastIntersectSpherePlane(__m128 Center,  __m128 Radius,  __m128 Plane,
		__m128& Outside,  __m128& Inside) {
		__m128 Dist = Dot<4>(Center, Plane);

		// Outside the plane?
		Outside = GreaterExt(Dist, Radius);

		// Fully inside the plane?
		Inside = LessExt(Dist, -Radius);
	}

	void inline LM_VECTOR_CALL FastIntersectAxisAlignedBoxPlane(__m128 Center,__m128 Extents, __m128 Plane,
			__m128& Outside, __m128& Inside) {
		// Compute the distance to the center of the box.
		__m128 Dist = Dot<4>(Center, Plane);

		// Project the axes of the box onto the normal of the plane.  Half the
		// length of the projection (sometime called the "radius") is equal to
		// h(u) * abs(n dot b(u))) + h(v) * abs(n dot b(v)) + h(w) * abs(n dot b(w))
		// where h(i) are extents of the box, n is the plane normal, and b(i) are the 
		// axes of the box. In this case b(i) = [(1,0,0), (0,1,0), (0,0,1)].
		__m128 Radius = Dot<>(Extents, Abs(Plane));

		// Outside the plane?
		Outside = GreaterExt(Dist, Radius);

		// Fully inside the plane?
		Inside = LessExt(Dist, -Radius);
	}

	void inline LM_VECTOR_CALL FastIntersectOrientedBoxPlane(__m128 Center, __m128 Extents, __m128 Axis0,
		sse_param_type_t<__m128, 4> Axis1,
		sse_param_type_t<__m128, 5> Axis2,
		sse_param_type_t<__m128, 6> Plane,
		__m128& Outside, __m128& Inside) {
		// Compute the distance to the center of the box.
		__m128 Dist =Dot<4>(Center, Plane);

		// Project the axes of the box onto the normal of the plane.  Half the
		// length of the projection (sometime called the "radius") is equal to
		// h(u) * abs(n dot b(u))) + h(v) * abs(n dot b(v)) + h(w) * abs(n dot b(w))
		// where h(i) are extents of the box, n is the plane normal, and b(i) are the 
		// axes of the box.
		__m128 Radius = Dot<3>(Plane, Axis0);
		Radius = Insert<0, 0, 1, 0, 0>(Radius, Dot<>(Plane, Axis1));
		Radius = Insert<0, 0, 0, 1, 0>(Radius, Dot<>(Plane, Axis2));
		Radius = Dot<>(Extents, Abs(Radius));

		// Outside the plane?
		Outside = GreaterExt(Dist, Radius);

		// Fully inside the plane?
		Inside = LessExt(Dist, -Radius);
	}

	inline __m128 LM_VECTOR_CALL PointOnLineSegmentNearestPoint(__m128 S1, __m128 S2, __m128 P) {
		__m128 Dir = S2 - S1;
		__m128 Projection = (Dot<>(P, Dir) - Dot<>(S1, Dir));
		__m128 LengthSq = Dot<>(Dir, Dir);

		__m128 t = Projection * Reciprocal(LengthSq);
		__m128 Point = S1 + t * Dir;

		// t < 0
		__m128 SelectS1 = LessExt(Projection, SplatZero());
		Point = Select(Point, S1, SelectS1);

		// t > 1
		__m128 SelectS2 =GreaterExt(Projection, LengthSq);
		Point = Select(Point, S2, SelectS2);

		return Point;
	}

	inline void LM_VECTOR_CALL FastIntersectFrustumPlane(__m128 Point0, __m128 Point1, __m128 Point2,
		sse_param_type_t<__m128, 4>  Point3,
		sse_param_type_t<__m128, 5> Point4,
		sse_param_type_t<__m128, 6> Point5, 
		const __m128& Point6, 
		const __m128& Point7,
		const __m128& Plane, 
		__m128& Outside, __m128& Inside)
	{
		// Find the min/max projection of the frustum onto the plane normal.
		__m128 Min, Max, Dist;

		Min = Max = Dot<>(Plane, Point0);

		Dist = Dot<>(Plane, Point1);
		Min = min(Min, Dist);
		Max = max(Max, Dist);

		Dist = Dot<>(Plane, Point2);
		Min = min(Min, Dist);
		Max = max(Max, Dist);

		Dist = Dot<>(Plane, Point3);
		Min = min(Min, Dist);
		Max = max(Max, Dist);

		Dist = Dot<>(Plane, Point4);
		Min = min(Min, Dist);
		Max = max(Max, Dist);

		Dist = Dot<>(Plane, Point5);
		Min = min(Min, Dist);
		Max = max(Max, Dist);

		Dist = Dot<>(Plane, Point6);
		Min = min(Min, Dist);
		Max = max(Max, Dist);

		Dist = Dot<>(Plane, Point7);
		Min = min(Min, Dist);
		Max = max(Max, Dist);

		__m128 PlaneDist = -SplatW(Plane);

		// Outside the plane?
		Outside = GreaterExt(Min, PlaneDist);

		// Fully inside the plane?
		Inside = LessExt(Max, PlaneDist);
	}
}

#endif
