////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/leo2dmath.hpp
//  Version:     v1.00
//  Created:     5/9/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 可选的2D逻辑(可以用3D替代,z =1.f)
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////

#ifndef IndePlatform_Leo2DMath_hpp
#define IndePlatform_Leo2DMath_hpp

#include "leomathtype.hpp"

namespace leo {
	//todo:另外一个文件

	inline float saturate(float x)
	{
		return max(0.f, min(1.f, x));
	}


	inline float smoothstep(float a, float b, float x)
	{
		float t = saturate((x - a) / (b - a));
		return t*t*(3.0f - (2.0f*t));
	}


	namespace ops{
		enum class axis_system {
			cartesian_system,
			windows_system,
			dx_texture_system,
			gl_texture_system,
			normalize_device_system
		};

		/*!
		\def Rect
		\brief	矩形区域。
		\construct  float4(float2(左上角坐标),float2(右下角坐标))
		\since build 1.00
		*/
		struct Rect {
			//top-left(x,y)
			//bottom-right(z,w)
			union{
				float4 tlbr;
				struct {
					float x, y, z, w;
				};
			};

			Rect() = default;

			Rect(const float4& TopLeftBottomRight)
				:tlbr(TopLeftBottomRight) {
			}

			float2 GetLeftTopCornet() const noexcept{
				return float2(tlbr.x, tlbr.y);
			}

			float2 GetRightBottomCornet() const noexcept{
				return float2(tlbr.z, tlbr.w);
			}

			float2& GetLeftTopCornet() noexcept {
				return * (float2*)(tlbr.begin());
			}

			float2& GetRightBottomCornet() noexcept {
				return *(float2*)(tlbr.begin()+2);
			}
		};

		template<axis_system src_system, axis_system dst_system>
		Rect Convert(const Rect& rect) noexcept;

		template<axis_system system>
		const Rect& IRect() noexcept;

		template<>
		inline Rect Convert<axis_system::dx_texture_system,axis_system::normalize_device_system>(const Rect& rect) noexcept
		{
			Rect o;
			o.x = rect.x * 2 - 1;
			o.z = rect.z * 2 - 1;

			o.y = 1 - rect.y * 2;
			o.w = 1 - rect.w * 2;

			return o;
		}

		template<>
		inline const Rect& IRect<axis_system::dx_texture_system>() noexcept
		{
			static Rect irect(float4(0.f, 0.f, 1.f, 1.f));
			return irect;
		}
	}
}

#endif
