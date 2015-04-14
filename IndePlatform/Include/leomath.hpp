////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/leomath.h
//  Version:     v1.03
//  Created:     9/28/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 要求目标处理器<X86支持SSE2指令集><X64支持MMX指令集>
// -------------------------------------------------------------------------
//  History:
//			 10/21/2014 改动了头文件,新增了函数
//			 12/1/2014  新增四元数函数
//		     04/13/2014 增加对应HLSL部分
////////////////////////////////////////////////////////////////////////////

#ifndef IndePlatform_LeoMath_hpp
#define IndePlatform_LeoMath_hpp

#include "ldef.h"
#include "platform_macro.h"
#include "leoint.hpp"
#include <immintrin.h>
#include "leo_math_convert_impl.h"
#include <array>
#include <cmath>

namespace leo {
	struct float2;
	struct float3;
	struct float4;

	struct half;
	struct half2;
	struct half3;
	struct half4;

	template<typename scalar,size_t multi>
	struct data_storage;

	template<typename scalar>
	struct data_storage<scalar, 2> {
		using scalar_type = scalar;

		union {
			struct {
				scaler x, y;
			};
			struct {
				scaler u, v;
			};
			scaler data[2];
		};

		constexpr size_t size() const{
			return 2;
		}

		scalar_type* begin() const{
			return data;
		}

		scalar_type* end() const{
			return data+size();
		}
	};

	template<typename scalar>
	struct data_storage<scalar, 3> {
		using scalar_type = scalar;

		union {
			struct {
				scaler x, y,z;
			};
			struct {
				scaler u, v,w;
			};
			scaler data[3];
		};

		constexpr size_t size() const {
			return 3;
		}

		scalar_type* begin() const{
			return data;
		}

		scalar_type* end() const {
			return data + size();
		}
	};

	template<typename scalar>
	struct data_storage<scalar, 4> {
		using scalar_type = scalar;

		union {
			struct {
				scaler x, y, z,w;
			};
			struct {
				scaler r, g, b,a;
			};
			scaler data[4];
		};

		constexpr size_t size() const {
			return 4;
		}

		scalar_type* begin() const {
			return data;
		}

		scalar_type* end() const {
			return data + size();
		}
	};


	//The float2 data type
	struct lalignas(16) float2 :data_storage<float,2>
	{

		float2() noexcept = default;

		float2(float X, float Y) noexcept
			:u(X), v(Y)
		{}

		explicit float2(const float* src) noexcept {
			std::memcpy(data, src, sizeof(float) * 2);
		}

		template<typename T>
		explicit float2(const T& src) noexcept
		{
			static_assert(sizeof(T) >= sizeof(float) * 2, "Need More Data");
			std::memcpy(this, &src, sizeof(float) * 2);
		}

		template<typename T>
		float2& operator=(const T& src) noexcept
		{
			static_assert(sizeof(T) >= sizeof(float) * 2, "Need More Data");
			std::memcpy(this, &src, sizeof(float) * 2);
			return *this;
		}

		template<typename T>
		float* operator&() noexcept
		{
			return data;
		}
	};
	
	//The float3 data type
	struct lalignas(16) float3 :data_storage<float, 3>
	{
		union {
			struct {
				float x, y, z;
			};
			struct {
				float u, v, w;
			};
			struct {
				float r, g, b;
			};
			float data[3];
		};

		float3() noexcept = default;

		float3(float X, float Y, float Z) noexcept
			:u(X), v(Y), w(Z)
		{}

		float3(const float2& XY, float Z) noexcept
			: x(XY.x), y(XY.y), z(Z)
		{
		}

		float3(float X, const float2& YZ) noexcept
			: x(X), y(YZ.x), z(YZ.y)
		{
		}

		explicit float3(const float* src) noexcept {
			std::memcpy(&x, src, sizeof(float) * 3);
		}

		template<typename T>
		explicit float3(const T& src) noexcept
		{
			static_assert(sizeof(T) >= sizeof(float) * 3, "Need More Data");
			std::memcpy(this, &src, sizeof(float) * 3);
		}

		template<typename T>
		float3& operator=(const T& src) noexcept
		{
			static_assert(sizeof(T) >= sizeof(float) * 3, "Need More Data");
			std::memcpy(this, &src, sizeof(float) * 3);
			return *this;
		}

		float* operator &() noexcept
		{
			return data;
		}
		
	};

	//The float4 data type
	struct lalignas(16) float4 :data_storage<float, 4>
	{
		union {
			struct {
				float x, y, z, w;
			};
			struct {
				float r, g, b,a;
			};
			float data[4];
		};

		float4() noexcept = default;

		float4(float X, float Y, float Z, float W) noexcept
			:x(X), y(Y), z(Z), w(W)
		{}

		float4(const float2& XY, const float2& ZW) noexcept
			: x(XY.x), y(XY.y), z(ZW.x), w(ZW.y)
		{
		}

		float4(const float2& XY, float Z, float W) noexcept
			: x(XY.x), y(XY.y), z(Z), w(W)
		{
		}

		float4(float X, const float2& YZ, float W) noexcept
			: x(X), y(YZ.x), z(YZ.y), w(W)
		{
		}

		float4(float X, float Y, const float2& ZW) noexcept
			: x(X), y(Y), z(ZW.x), w(ZW.y)
		{
		}

		float4(const float3& XYZ, float W) noexcept
			: x(XYZ.x), y(XYZ.y), z(XYZ.z), w(W)
		{
		}

		float4(float X, const float3& YZW) noexcept
			: x(X), y(YZW.x), z(YZW.y), w(YZW.z)
		{
		}


		explicit float4(const float* src) noexcept {
			std::memcpy(&x, src, sizeof(float) * 4);
		}

		template<typename T>
		explicit float4(const T& src) noexcept
		{
			static_assert(sizeof(T) >= sizeof(float4), "Need More Data");
			std::memcpy(this, &src, sizeof(float4));
		}

		template<typename T>
		float4& operator=(const T& src) noexcept
		{
			static_assert(sizeof(T) >= sizeof(float4), "Need More Data");
			std::memcpy(this, &src, sizeof(float4));
			return *this;
		}

		float* operator &() noexcept
		{
			return data;
		}
		
	};

	//The float3x3 data type
	struct lalignas(16) float3x3 {
		std::array<float3,3> r;

		float& operator()(uint8 row, uint8 col) noexcept{
			return r[row].data[col];
		}

		float operator()(uint8 row, uint8 col) const noexcept {
			return r[row].data[col];
		}

		float3& operator[](uint8 row) noexcept {
			return r[row];
		}

		const float3& operator[](uint8 row) const noexcept {
			return r[row];
		}

		float3x3() noexcept = default;

		float3x3(const float3& r0, const float3& r1, const float3& r2) noexcept
			:r({ r0,r1,r2 })
		{
		}

		explicit float3x3(const float* t) noexcept {
			std::memcpy(r.data(), t, sizeof(float)*3*3);
		}

	};

	//The float4x4 
	struct lalignas(16) float4x4 {
		std::array<float4, 4> r;

		float& operator()(uint8 row, uint8 col) noexcept{
			return r[row].data[col];
		}

		float operator()(uint8 row, uint8 col) const noexcept {
			return r[row].data[col];
		}


		float4& operator[](uint8 row) noexcept {
			return r[row];
		}

		const float4& operator[](uint8 row) const noexcept {
			return r[row];
		}

		explicit float4x4(const float* t) noexcept {
			std::memcpy(r.data(), t, sizeof(float) * 4 * 4);
		}

		float4x4(const float3x3& m) noexcept {
			r[0] = float4(m[0], 0.f);
			r[1] = float4(m[1], 0.f);
			r[2] = float4(m[2], 0.f);
			r[3] = float4(0.f, 0.f, 0.f, 1.f);
		}

		float4x4() noexcept = default;

		float4x4(const float4& r0, const float4& r1, const float4& r2, const float4& r3) noexcept
			:r({ r0,r1,r2,r3 })
		{
		}
	};


	//The HALF data type is equivalent to the IEEE 754 binary16 format
	//consisting of a sign bit, a 5-bit biased exponent, and a 10-bit mantissa.
	//[15] SEEEEEMMMMMMMMMM [0]
	//(-1) ^ S * 0.0, if E == 0 and M == 0,
	//(-1) ^ S * 2 ^ -14 * (M / 2 ^ 10), if E == 0 and M != 0,
	//(-1) ^ S * 2 ^ (E - 15) * (1 + M / 2 ^ 10), if 0 < E < 31,
	//(-1) ^ S * INF, if E == 31 and M == 0, or
	//NaN, if E == 31 and M != 0,
	struct lalignas(2) half
	{
		uint16 data;
		explicit half(float f) noexcept
			:data(details::float_to_half(f))
		{

		}

		explicit half(uint16 bin) noexcept
			: data(bin)
		{
		}

		half& operator=(float f) noexcept
		{
			*this = half(f);
		}

		half& operator=(uint16 i) noexcept
		{
			*this = half(i);
		}

		explicit operator float() const noexcept
		{
			return details::half_to_float(data);
		}
	};

	struct lalignas(4) half2 :data_storage<half, 2>
	{
		half2(float X, float Y) noexcept
			:u(X), v(Y)
		{}

		half2(half X, half Y) noexcept
			: u(X), v(Y)
		{}

		half2(const float2& XY) noexcept
		:u(XY.x),v(XY.y){
		}

		half2() noexcept = default;

		half2& operator=(const float2& XY) noexcept
		{
			*this = half2(XY);
		}

		explicit operator float2() const noexcept
		{
			return float2(x.operator float(),y.operator float());
		}
	};

	struct lalignas(8) half3 :data_storage<half, 3>
	{
		union {
			struct {
				half x, y, z;
			};
			struct {
				half u, v, w;
			};
			half data[3];
		};

		half3(float X, float Y, float Z) noexcept
			:u(x), v(Y), w(Z)
		{}

		half3(const float3& XYZ) noexcept
			: u(XYZ.x), v(XYZ.y), w(XYZ.z)
		{}

		half3(const float2& XY, float Z) noexcept
			: u(XY.x), v(XY.y), w(Z)
		{}

		half3(float X,const float2& YZ) noexcept
			: u(X), v(YZ.x), w(YZ.y)
		{}

		half3& operator=(const float3& XYZ) noexcept
		{
			*this = half3(XYZ);
		}

		explicit operator float3() const noexcept
		{
			return float3(x.operator float(), y.operator float(),z.operator float());
		}
	};

	struct lalignas(8) half4 :data_storage<half, 4>
	{
		union {
			struct {
				half x, y, z, w;
			};
			struct {
				half r, g, b, a;
			};
			half data[4];
		};

		half4(float X, float Y, float Z, float W) noexcept
			:x(X), y(Y), z(Z), w(W)
		{}

		half4(const float4& XYZW) noexcept
			: x(XYZW.x), y(XYZW.y), z(XYZW.z), w(XYZW.w)
		{}

		half4(const float2& XY, const float2& ZW) noexcept
			: x(XY.x), y(XY.y), z(ZW.x), w(ZW.y)
		{
		}

		half4(const float2& XY, float Z, float W) noexcept
			: x(XY.x), y(XY.y), z(Z), w(W)
		{
		}

		half4(float X, const float2& YZ, float W) noexcept
			: x(X), y(YZ.x), z(YZ.y), w(W)
		{
		}

		half4(float X, float Y, const float2& ZW) noexcept
			: x(X), y(Y), z(ZW.x), w(ZW.y)
		{
		}

		half4(const float3& XYZ, float W) noexcept
			: x(XYZ.x), y(XYZ.y), z(XYZ.z), w(W)
		{
		}

		half4(float X, const float3& YZW) noexcept
			: x(X), y(YZW.x), z(YZW.y), w(YZW.z)
		{
		}


		half4(half X, half Y, half Z, half W) noexcept
			: x(X), y(Y), z(Z), w(W)
		{}

		half4(const half2& XY, const half2& ZW) noexcept
			: x(XY.x), y(XY.y), z(ZW.x), w(ZW.y)
		{
		}

		half4(const half2& XY, half Z, half W) noexcept
			: x(XY.x), y(XY.y), z(Z), w(W)
		{
		}

		half4(half X, const half2& YZ, half W) noexcept
			: x(X), y(YZ.x), z(YZ.y), w(W)
		{
		}

		half4(half X, half Y, const half2& ZW) noexcept
			: x(X), y(Y), z(ZW.x), w(ZW.y)
		{
		}

		half4(const half3& XYZ, half W) noexcept
			: x(XYZ.x), y(XYZ.y), z(XYZ.z), w(W)
		{
		}

		half4(half X, const half3& YZW) noexcept
			: x(X), y(YZW.x), z(YZW.y), w(YZW.z)
		{
		}


		half4& operator=(const float4& XYZW) noexcept
		{
			*this = half4(XYZW);
		}

		explicit operator float4() const noexcept
		{
			return float4(x.operator float(), y.operator float(), z.operator float(),w.operator float());
		}
	};
}


#endif
