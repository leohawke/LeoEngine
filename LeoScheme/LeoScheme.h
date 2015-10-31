////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine/Script Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   LeoScheme/LeoScheme.h
//  Version:     v1.00
//  Created:     10/17/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: LeoScheme»ù´¡Í·
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////
#ifndef Script_leoScheme_h
#define Script_leoScheme_h

#include <cstdint>
#include <script_type.hpp>
#include <string>

namespace leo {
	namespace script {
		using int8 = std::int8_t;
		using unit8 = std::uint8_t;
		using int16 = std::int16_t;
		using unit16 = std::uint16_t;
		using int32 = std::int32_t;
		using unit32 = std::uint32_t;
		using int64 = std::int64_t;
		using unit64 = std::uint64_t;
		using byte = int8;

		struct small;
		using real8 = small;
		using real16 = leo::half;
		static_assert(sizeof(float) == 4, "Non't Support");
		using real32 = float;
		static_assert(sizeof(double) == 8, "Non't Support");
		using real64 = double;

		using vec2 = leo::float2;
		using vec3 = leo::float3;
		using vec4 = leo::float4;
		using vec3x3 = leo::float3x3;
		using vec4x4 = leo::float4x4;

		using string = std::string;

		//using scheme_int;
		//using scheme_real;
		//using scheme_obj;

		extern std::shared_ptr<scheme_void_t> scheme_void;

		struct scheme_value {
		public:
			scheme_value(const std::shared_ptr<scheme_void_t>&){
			}

			scheme_value(const std::shared_ptr<scheme_obj>& obj_ptr) {

			}
		};
	}
}


#endif
