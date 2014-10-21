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
#include "..\Core\Geometry.hpp"

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

		std::pair<bool,float> Intersect(const Box& box) const{
			float dist = 0.f;
			auto result = box.Intersects(load(mOrigin), load(mDir),dist);
			return std::make_pair(result, dist);
		}

		std::pair<bool, float> Intersect(const Sphere& sphere) const{
			float dist = 0.f;
			auto result = sphere.Intersects(load(mOrigin), load(mDir), dist);
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

		std::pair<bool, float> Intersect(__m128 p0, __m128 p1, __m128 p2) const{
			auto Direction = load(mDir);

			assert(details::IsUnit<>(Direction));

			auto Zero = XMVectorZero();

			auto e1 = Subtract(p1, p0);// V1 - V0;
			auto e2 = Subtract(p2, p0);// V2 - V0;

			// p = Direction ^ e2;
			auto p = Cross(Direction, e2);

			// det = e1 * p;
			auto det = Dot(e1, p);

			XMVECTOR u, v, t;

			if (XMVector3GreaterOrEqual(det, g_RayEpsilon))
			{
				// Determinate is positive (front side of the triangle).
				XMVECTOR s = Origin - V0;

				// u = s * p;
				u = XMVector3Dot(s, p);

				XMVECTOR NoIntersection = XMVectorLess(u, Zero);
				NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(u, det));

				// q = s ^ e1;
				XMVECTOR q = XMVector3Cross(s, e1);

				// v = Direction * q;
				v = XMVector3Dot(Direction, q);

				NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(v, Zero));
				NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(u + v, det));

				// t = e2 * q;
				t = XMVector3Dot(e2, q);

				NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(t, Zero));

				if (XMVector4EqualInt(NoIntersection, XMVectorTrueInt()))
				{
					Dist = 0.f;
					return false;
				}
			}
			else if (XMVector3LessOrEqual(det, g_RayNegEpsilon))
			{
				// Determinate is negative (back side of the triangle).
				XMVECTOR s = Origin - V0;

				// u = s * p;
				u = XMVector3Dot(s, p);

				XMVECTOR NoIntersection = XMVectorGreater(u, Zero);
				NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(u, det));

				// q = s ^ e1;
				XMVECTOR q = XMVector3Cross(s, e1);

				// v = Direction * q;
				v = XMVector3Dot(Direction, q);

				NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(v, Zero));
				NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(u + v, det));

				// t = e2 * q;
				t = XMVector3Dot(e2, q);

				NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(t, Zero));

				if (XMVector4EqualInt(NoIntersection, XMVectorTrueInt()))
				{
					Dist = 0.f;
					return false;
				}
			}
			else
			{
				// Parallel ray.
				Dist = 0.f;
				return false;
			}

			t = XMVectorDivide(t, det);

			// (u / det) and (v / dev) are the barycentric cooridinates of the intersection.

			// Store the x-component to *pDist
			XMStoreFloat(&Dist, t);

			return true;
		}
	};
}

#endif