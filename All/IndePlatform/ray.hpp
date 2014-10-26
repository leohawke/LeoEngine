////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/ray.hpp
//  Version:     v1.00
//  Created:     10/20/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: …‰œﬂœ‡πÿ
// -------------------------------------------------------------------------
//  History:
//		
//
////////////////////////////////////////////////////////////////////////////
#ifndef IndePlatform_leo_math_ray_h
#define IndePlatform_leo_math_ray_h

#include "LeoMath.h"
#include "Geometry.hpp"

namespace leo{
	struct lalignas(16) ViewPort{
		float mTLX;
		float mTLY;
		float mWindth;
		float mHeight;
		float mMinDepth;
		float mMaxDepth;
	};

	struct lalignas(16) Ray{
		float3 mOrigin;
		float4 mDir;

		Ray() = default;

		Ray(const float3& origin, const float4& dir)
			:mOrigin(origin), mDir(dir){
		}

		static Ray Pick(const ViewPort& vp, const float4x4& proj,const float2& pos,bool descarte = false){
			
			float x_ndc = 2 * (pos.x - vp.mTLX) / vp.mWindth - 1;

			float y_ndc = 0.f;
			if (descarte)
				y_ndc = 2 * (pos.y - vp.mTLY) / vp.mHeight - 1;
			else
				y_ndc = -2 * (pos.y - vp.mTLY) / vp.mHeight + 1;

			float x_view = x_ndc / proj.r[0].x;
			float y_view = y_ndc / proj.r[1].y;
			
			return Ray(float3(0.f, 0.f, 0.f), float4(x_view, y_view, 1.0f, 0.f));
		}

		Ray Transform(const float4x4& matrix) const{
			return Transform(load(matrix));
		}

		Ray Transform(const std::array<__m128, 4>& matrix) const{
			float3 origin;
			save(origin, TransformCoord<>(load(mOrigin), matrix));
			float4 dir;
			save(dir,leo::Transform(load(mDir), matrix));
			return Ray(origin,dir);
		}

		Ray& Normalize(){
			save(mDir,leo::Normalize<>(load(mDir)));
			return *this;
		}

		Ray Normalize() const{
			float4 dir;
			save(dir, leo::Normalize<>(load(mDir)));
			return Ray(mOrigin,dir);
		}

		std::pair<bool, float> Intersect(const Sphere& sphere) const{
			return sphere.Intersects(*this);
		}

		std::pair<bool, float> Intersect(const Triangle& tri) const{
			return tri.Intersects(*this);
		}

#if 0
		std::pair<bool,float> Intersect(const Box& box) const{
			float dist = 0.f;
			auto result = box.Intersects(load(mOrigin), load(mDir),dist);
			return std::make_pair(result, dist);
		}

		

		std::pair<bool, float> Intersect(const OrientedBox& obox) const{
			float dist = 0.f;
			auto result = obox.Intersects(load(mOrigin), load(mDir), dist);
			return std::make_pair(result, dist);
		}

		std::pair<bool, float> Intersect(const Frustum& fru) const{
			float dist = 0.f;
			auto result = fru.Intersects(load(mOrigin), load(mDir), dist);
			return std::make_pair(result, dist);
		}

		//std::pair<bool, float> Intersect(const Triangle& tri) const;

#endif
	};

	std::pair<bool, float> Sphere::Intersects(const Ray& ray) const{
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
		if (NotEqualInt<>(NoIntersection, details::SplatTrueInt())){
			float dist = 0.f;
			save(dist, t);
			return{ true, dist };
		}

		return{ false, 0.f };
	}

	std::pair<bool, float> Triangle::Intersects(const Ray& ray) const{
		auto Direction = load(ray.mDir);

		assert(details::IsUnit<>(Direction));

		auto Zero = SplatZero();

		auto p0 = load(p[0]);
		auto p1 = load(p[1]);
		auto p2 = load(p[2]);
		auto e1 = Subtract(p1, p0);// V1 - p0;
		auto e2 = Subtract(p2, p0);// V2 - p0;

		// p = Direction ^ e2;
		auto p = Cross<>(Direction, e2);

		// det = e1 * p;
		auto det = Dot<>(e1, p);

		__m128 u, v, t;

		auto Origin = load(ray.mOrigin);
		if (GreaterOrEqual<>(det, details::SplatRayEpsilon()))
		{
			// Determinate is positive (front side of the triangle).
			auto s = Subtract(Origin, p0);

			// u = s * p;
			u = Dot<>(s, p);

			auto NoIntersection = LessExt(u, Zero);
			NoIntersection = OrInt(NoIntersection, GreaterExt(u, det));

			// q = s ^ e1;
			auto q = Cross<>(s, e1);

			// v = Direction * q;
			v = Dot<>(Direction, q);

			NoIntersection = OrInt(NoIntersection, LessExt(v, Zero));
			NoIntersection = OrInt(NoIntersection, GreaterExt(u + v, det));

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
			u = Dot<>(s, p);

			auto NoIntersection = GreaterExt(u, Zero);
			NoIntersection = OrInt(NoIntersection, LessExt(u, det));

			// q = s ^ e1;
			auto q = Cross<>(s, e1);

			// v = Direction * q;
			v = Dot<>(Direction, q);

			NoIntersection = OrInt(NoIntersection, GreaterExt(v, Zero));
			NoIntersection = OrInt(NoIntersection, LessExt(u + v, det));

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
}

#endif