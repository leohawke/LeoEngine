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

#include "LeoMathutility.hpp"
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
			save(dir,leo::Multiply(load(mDir), matrix));
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

	
}

#endif