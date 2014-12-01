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

#ifndef IndePlatform_LeoMath_h
#error "不要包含实现头文件"
#endif

#ifndef IndePlatform_LeoMath_impl_hpp
#define IndePlatform_LeoMath_impl_hpp

#include "leomath.h"

namespace leo {
	

	inline __m128 LM_VECTOR_CALL operator*(__m128 lhs, __m128 rhs)  {
		return Multiply(lhs, rhs);
	}

	inline __m128 LM_VECTOR_CALL operator-(__m128 lhs, __m128 rhs) {
		return Subtract(lhs, rhs);
	}

	inline __m128 LM_VECTOR_CALL operator+(__m128 lhs, __m128 rhs) {
		return Add(lhs, rhs);
	}

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

		inline __m128 SplatR3() {
			const static float4	r3(0.0f, 0.0f, 0.0f, 1.0f);
			const static auto _mr1 = load(r3);
			return _mr1;
		}

	}

	__m128 LM_VECTOR_CALL PlaneTransform(__m128 P, __m128 Q, __m128 O) {
		__m128 vNormal = Rotate<>(P, Q);
		__m128 vD =SplatW(P) - Dot<>(vNormal, O);

		return Insert<0, 0, 0, 0, 1>(vNormal, vD);
	}

	enum class CONTAINMENT_TYPE : std::uint8_t {
		DISJOINT = 0,
		INTERSECTS = 1,
		CONTAINS = 2,
	};

	template<typename T,uint8 slot>
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
	using sse_param_type_t = sse_param_type<T, slot>::type;

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

		inline CONTAINMENT_TYPE LM_VECTOR_CALL ContainedBy(__m128 V0, __m128 V1, __m128 V2,
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
			if (EqualInt(AnyOutside,details::SplatTrueInt()))
				return CONTAINMENT_TYPE::DISJOINT;

			// If the triangle is inside all planes it is inside.
			if (EqualInt(AllInside, details::SplatTrueInt()))
				return CONTAINMENT_TYPE::DISJOINT;

			// The triangle is not inside all planes or outside a plane, it may intersect.
			return CONTAINMENT_TYPE::DISJOINT;
		}
	}
}

#endif
