////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2016.
// -------------------------------------------------------------------------
//  File name:   LBase/leomathtype.h
//  Version:     v1.04
//  Created:     9/28/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2016
//  Description: 要求目标处理器支持SSE2指令集(或等同功能指令)
// -------------------------------------------------------------------------
//  History:
//			 10/21/2014 改动了头文件,新增了函数
//			 12/1/2014  新增四元数函数
//		     04/13/2014 增加对应HLSL部分
////////////////////////////////////////////////////////////////////////////

#ifndef LBase_LMATHTYPE_HPP
#define LBase_LMATHTYPE_HPP 1

#include "ldef.h"
#include "lmathhalf.h"
#include "Debug.h"

#include <algorithm> //std::max,std::min
#include <cstring> //std::memcpy 
#include <array> //std::array
#include <limits>
//data type
namespace leo {
	namespace math {
		struct float2;
		struct float3;
		struct float4;

		struct half;
		struct half2;
		struct half3;
		struct half4;

		template<typename scalar, size_t multi>
		struct data_storage;

		//behavior change: constructor is no longer implicitly called
#ifdef LB_IMPL_MSCPP
#pragma warning(push)
#pragma warning(disable :4587)
#endif

		template<typename scalar>
		struct data_storage<scalar, 2> {
			using scalar_type = scalar;
			using vec_type = data_storage<scalar, 2>;

			union {
				struct {
					scalar x, y;
				};
				struct {
					scalar u, v;
				};
				scalar data[2];
			};

			data_storage(scalar X, scalar Y)
				:x(X), y(Y)
			{}

			data_storage() = default;

			constexpr size_t size() const {
				return 2;
			}

			const scalar_type& operator[](std::size_t index) const noexcept {
				lassume(index < size());
				return data[index];
			}

			scalar_type& operator[](std::size_t index) noexcept {
				lassume(index < size());
				return data[index];
			}

			const scalar_type* begin() const {
				return data + 0;
			}

			const scalar_type* end() const {
				return data + size();
			}

			scalar_type* begin() {
				return data + 0;
			}

			scalar_type* end() {
				return data + size();
			}
		};

		template<typename scalar>
		struct data_storage<scalar, 3> {
			using scalar_type = scalar;
			using vec_type = data_storage<scalar, 3>;

			union {
				struct {
					scalar x, y, z;
				};
				struct {
					scalar u, v, w;
				};
				struct {
					scalar r, g, b;
				};
				scalar data[3];
			};

			data_storage() = default;

			data_storage(scalar X, scalar Y, scalar Z)
				:x(X), y(Y), z(Z)
			{}

			constexpr size_t size() const {
				return 3;
			}

			const scalar_type& operator[](std::size_t index) const noexcept {
				lassume(index < size());
				return data[index];
			}

			scalar_type& operator[](std::size_t index) noexcept {
				lassume(index < size());
				return data[index];
			}

			const scalar_type* begin() const {
				return data + 0;
			}

			const scalar_type* end() const {
				return data + size();
			}

			scalar_type* begin() {
				return data + 0;
			}

			scalar_type* end() {
				return data + size();
			}
		};

		template<typename scalar>
		struct data_storage<scalar, 4> {
			using scalar_type = scalar;
			using vec_type = data_storage<scalar, 4>;

			union {
				struct {
					scalar x, y, z, w;
				};
				struct {
					scalar r, g, b, a;
				};
				scalar data[4];
			};

			data_storage() = default;

			data_storage(scalar X, scalar Y, scalar Z, scalar W)
				:x(X), y(Y), z(Z), w(W)
			{}

			constexpr size_t size() const {
				return 4;
			}

			const scalar_type& operator[](std::size_t index) const noexcept {
				lassume(index < size());
				return data[index];
			}

			scalar_type& operator[](std::size_t index) noexcept {
				lassume(index < size());
				return data[index];
			}

			const scalar_type* begin() const {
				return data + 0;
			}

			const scalar_type* end() const {
				return  data + size();
			}

			scalar_type* begin() {
				return data + 0;
			}

			scalar_type* end() {
				return  data + size();
			}
		};

#ifdef LB_IMPL_MSCPP
#pragma warning(pop)
#endif

		//The float2 data type
		struct lalignas(16) float2 :data_storage<float, 2>
		{

			float2() noexcept = default;

			float2(float X, float Y) noexcept
				:vec_type(X, Y)
			{}

			explicit float2(const float* src) noexcept
				: vec_type(src[0], src[1])
			{}

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

			float* operator&() noexcept
			{
				return data;
			}

			float2 operator-() const noexcept {
				return float2(-x, -y);
			}
		};

		//The float3 data type
		struct lalignas(16) float3 :data_storage<float, 3>
		{

			float3() noexcept = default;

			float3(float X, float Y, float Z) noexcept
				:vec_type(X, Y, Z)
			{}

			float3(const float2& XY, float Z) noexcept
				: vec_type(XY.x, XY.y, Z)
			{}

			float3(float X, const float2& YZ) noexcept
				: vec_type(X, YZ.x, YZ.y)
			{}

			explicit float3(const float* src) noexcept
				: vec_type(src[0], src[1], src[2])
			{}

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

			float3 operator-() const noexcept {
				return float3(-x, -y, -z);
			}

		};

		//The float4 data type
		struct lalignas(16) float4 :data_storage<float, 4>
		{

			float4() noexcept = default;

			float4(float X, float Y, float Z, float W) noexcept
				:vec_type(X, Y, Z, W)
			{}

			float4(const float2& XY, const float2& ZW) noexcept
				: vec_type(XY.x, XY.y, ZW.x, ZW.y)
			{
			}

			float4(const float2& XY, float Z, float W) noexcept
				: vec_type(XY.x, XY.y, Z, W)
			{
			}

			float4(float X, const float2& YZ, float W) noexcept
				: vec_type(X, YZ.x, YZ.y, W)
			{
			}

			float4(float X, float Y, const float2& ZW) noexcept
				: vec_type(X, Y, ZW.x, ZW.y)
			{
			}

			float4(const float3& XYZ, float W) noexcept
				: vec_type(XYZ.x, XYZ.y, XYZ.z, W)
			{
			}

			float4(float X, const float3& YZW) noexcept
				: vec_type(X, YZW.x, YZW.y, YZW.z)
			{
			}


			explicit float4(const float* src) noexcept
				: vec_type(src[0], src[1], src[2], src[3]) {
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

			float4 operator-() const noexcept {
				return float4(-x, -y, -z, -w);
			}

		};

		//The float3x3 data type
		struct lalignas(16) float3x3 {
			std::array<float3, 3> r;

			float& operator()(uint8 row, uint8 col) noexcept {
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
				std::memcpy(r.data(), t, sizeof(float) * 3 * 3);
			}

		};

		//The float4x4 
		struct lalignas(16) float4x4 {
			std::array<float4, 4> r;

			float& operator()(uint8 row, uint8 col) noexcept {
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

			half() noexcept
				: data(0)
			{}

			half& operator=(float f) noexcept
			{
				*this = half(f);
				return *this;
			}

			half& operator=(uint16 i) noexcept
			{
				*this = half(i);
				return *this;
			}

			explicit operator float() const noexcept
			{
				return details::half_to_float(data);
			}
		};

		struct lalignas(4) half2 :data_storage<half, 2>
		{
			half2(half X, half Y) noexcept
				:vec_type(X, Y)
			{}

			half2(float X, float Y) noexcept
				: half2(half(X), half(Y))
			{}

			half2(const float2& XY) noexcept
				: half2(XY.x, XY.y) {
			}

			half2() noexcept = default;

			half2& operator=(const float2& XY) noexcept
			{
				*this = half2(XY);
				return *this;
			}

			explicit operator float2() const noexcept
			{
				return float2(x.operator float(), y.operator float());
			}
		};

		struct lalignas(8) half3 :data_storage<half, 3>
		{
			half3(half X, half Y, half Z) noexcept
				:vec_type(X, Y, Z)
			{}

			half3(float X, float Y, float Z) noexcept
				: half3(half(X), half(Y), half(Z))
			{}

			half3(const float3& XYZ) noexcept
				: half3(XYZ.x, XYZ.y, XYZ.z)
			{}

			half3(const half2& XY, half Z) noexcept
				: vec_type(XY.x, XY.y, Z) {
			}

			half3(const float2& XY, float Z) noexcept
				: half3(half2(XY), half(Z))
			{}

			half3(half X, const half2& YZ) noexcept
				: vec_type(X, YZ.x, YZ.y)
			{}

			half3(float X, const float2& YZ) noexcept
				: half3(half(X), YZ)
			{}

			half3& operator=(const float3& XYZ) noexcept
			{
				*this = half3(XYZ);
				return *this;
			}

			explicit operator float3() const noexcept
			{
				return float3(x.operator float(), y.operator float(), z.operator float());
			}
		};

		struct lalignas(8) half4 :data_storage<half, 4>
		{
			half4(half X, half Y, half Z, half W) noexcept
				:vec_type(X, Y, Z, W)
			{}

			half4(const half2& XY, const half2& ZW) noexcept
				: vec_type(XY.x, XY.y, ZW.x, ZW.y)
			{}

			half4(const half2& XY, half Z, half W) noexcept
				: vec_type(XY.x, XY.y, Z, W)
			{}

			half4(half X, const half2& YZ, half W) noexcept
				: vec_type(X, YZ.x, YZ.y, W)
			{}

			half4(half X, half Y, const half2& ZW) noexcept
				: vec_type(X, Y, ZW.x, ZW.y)
			{}

			half4(const half3& XYZ, half W) noexcept
				: vec_type(XYZ.x, XYZ.y, XYZ.z, W)
			{}

			half4(half X, const half3& YZW) noexcept
				: vec_type(X, YZW.x, YZW.y, YZW.z)
			{}

			half4(float X, float Y, float Z, float W) noexcept
				: half4(half(X), half(Y), half(Z), half(W))
			{}

			half4(const float4& XYZW) noexcept
				: half4(half(XYZW.x), half(XYZW.y), half(XYZW.z), half(XYZW.w))
			{}

			half4(const float2& XY, const float2& ZW) noexcept
				: half4(half2(XY), half2(ZW))
			{}

			half4(const float2& XY, float Z, float W) noexcept
				: half4(half2(XY), half(Z), half(W))
			{}

			half4(float X, const float2& YZ, float W) noexcept
				: half4(half(X), half2(YZ), half(W))
			{}

			half4(float X, float Y, const float2& ZW) noexcept
				: half4(half(X), half(Y), half2(ZW))
			{}

			half4(const float3& XYZ, float W) noexcept
				: half4(half3(XYZ), half(W))
			{}

			half4(float X, const float3& YZW) noexcept
				: half4(half(X), half3(YZW))
			{}

			half4& operator=(const float4& XYZW) noexcept
			{
				*this = half4(XYZW);
				return *this;
			}

			explicit operator float4() const noexcept
			{
				return float4(x.operator float(), y.operator float(), z.operator float(), w.operator float());
			}
		};
	}
}

//data base function
namespace leo {
	namespace math {
		inline bool zero_float(float l) {
			return (l <= std::numeric_limits<float>::epsilon()) && (l >= -std::numeric_limits<float>::epsilon());
		}

		inline bool float_equal(float l, float r) {
			return zero_float(l - r);
		}

		inline bool operator==(const float4& l, const float4& r) noexcept {
			return float_equal(l.x, r.x) && float_equal(l.y, r.y)
				&& float_equal(l.z, r.z) && float_equal(l.w, r.w);
		}

		inline bool operator!=(const float4& l, const float4& r) noexcept {
			return !(l==r);
		}

		inline float dot(const float4&l, const float4& r) {
			return l.x*r.x + l.y*r.y + l.z*r.z + l.w*r.w;
		}

		inline float length(const float4 & l) noexcept {
			return sqrtf(dot(l, l));
		}

		inline float length_sq(const float4& l) noexcept {
			return dot(l, l);
		}

		inline float4 normalize(const float4& l) noexcept {
			auto mod = length(l);
			return{ l.x / mod,l.y / mod,l.z / mod,l.w / mod };
		}

		inline float4 operator+(const float4& l, const float4& r) noexcept {
			return{ l.x + r.x,l.y + r.y,l.z + r.z,l.w + r.w };
		}

		inline float4 operator-(const float4& l, const float4& r) noexcept {
			return{ l.x - r.x,l.y - r.y,l.z - r.z,l.w - r.w };
		}

		inline float4 operator*(const float4& l, float r) {
			return{ l.x*r,l.y*r,l.z*r,l.w*r };
		}

		inline float4 operator*(float r,const float4& l) {
			return{ l.x*r,l.y*r,l.z*r,l.w*r };
		}

		inline float4& operator+=(float4& l, const float4& r) {
			l.x += r.x;
			l.y += r.y;
			l.z += r.z;
			l.w += r.w;
			return l;
		}

		inline float3& operator+=(float3& l, const float3& r) {
			l.x += r.x;
			l.y += r.y;
			l.z += r.z;
			return l;
		}

		inline float4& operator-=(float4& l, const float4& r) {
			l.x -= r.x;
			l.y -= r.y;
			l.z -= r.z;
			l.w -= r.w;
			return l;
		}

		inline float4& operator*=(float4& l, float r) {
			l.x *= r;
			l.y *= r;
			l.z *= r;
			l.w *= r;
			return l;
		}

		inline float4& operator/=(float4& l, float r) {
			l.x /= r;
			l.y /= r;
			l.z /= r;
			l.w /= r;
			return l;
		}

		template<typename T>
		inline float3& operator/=(float3&l, T r) {
			l.x /= r;
			l.y /= r;
			l.z /= r;
			return l;
		}

		inline float4 lerp(const float4& l, const float4& r, float t)
		{
			return l*(1.f - t) + r*t;
		}

		inline float4& operator/=(float4& l, const float4& r) {
			l.x /= r.x;
			l.y /= r.y;
			l.z /= r.z;
			l.w /= r.w;
			return l;
		}

		inline float4x4 transpose(const float4x4& m) {
			return{
				{ m[0][0],m[1][0] ,m[2][0] ,m[3][0] },
				{ m[0][1],m[1][1] ,m[2][1] ,m[3][1] },
				{ m[0][2],m[1][2] ,m[2][2] ,m[3][2] },
				{ m[0][3],m[1][3] ,m[2][3] ,m[3][3] }
			};
		}

		inline float4x4 operator*(float l, const float4x4& m) {
			return{
				m[0] * l,
				m[1] * l,
				m[2] * l,
				m[3] * l,
			};
		}

		inline float4x4 operator-(const float4x4&  l, const float4x4& r) {
			return{
				l[0] - r[0],
				l[1] - r[1],
				l[2] - r[2],
				l[3] - r[3],
			};
		}

		inline float4 transform(const float4& l, const float4x4& m) {
			auto tm = transpose(m);
			return{ dot(l,tm[0]),dot(l,tm[1]) ,dot(l,tm[2]) ,dot(l,tm[3]) };
		}

		using std::max;
		using std::min;

		inline float2 max(const float2& l, const float2& r) noexcept {
			return float2(max(l.x, r.x), max(l.y, r.y));
		}

		inline float2 min(const float2& l, const float2& r) noexcept {
			return float2(min(l.x, r.x), min(l.y, r.y));
		}

		inline float3 max(const float3& l, const float3& r) noexcept {
			return float3(max(l.x, r.x), max(l.y, r.y), max(l.z, r.z));
		}

		inline float3 min(const float3& l, const float3& r) noexcept {
			return float3(min(l.x, r.x), min(l.y, r.y), min(l.z, r.z));
		}

		inline float4 max(const float4& l, const float4& r) noexcept {
			return float4(max(l.x, r.x), max(l.y, r.y), max(l.b, r.b), max(l.a, r.a));
		}

		inline float4 min(const float4& l, const float4& r) noexcept {
			return float4(min(l.x, r.x), min(l.y, r.y), min(l.b, r.b), min(l.a, r.a));
		}

		using std::abs;

		inline float2 abs(const float2& f2) noexcept {
			return float2(abs(f2.x), abs(f2.y));
		}

		inline float3 abs(const float3& f3) noexcept {
			return float3(abs(f3.x), abs(f3.y), abs(f3.z));
		}

		inline float4 abs(const float4& f4) noexcept {
			return float4(abs(f4.x), abs(f4.y), abs(f4.z), abs(f4.w));
		}
	}
}

//data depend-base function
namespace leo {
	namespace math {

		template<typename vec>
		inline vec clamp(const vec& _Min, const vec& _Max, const vec & _X) {
			return max(_Min, min(_Max, _X));
		}

		template<typename vec>
		inline vec lerp(const vec& a, const vec& b, const vec & t) {
			return a*(1-t)+b*t;
		}

		inline float2 saturate(const float2& x) {
			const static auto zero = float2(0.f, 0.f);
			const static auto one = float2(1.f, 1.f);
			return  clamp(zero, one, x);
		}

		inline float3 saturate(const float3& x) {
			const static auto zero = float3(0.f, 0.f, 0.f);
			const static auto one = float3(1.f, 1.f, 1.f);
			return  clamp(zero, one, x);
		}

		inline float4 saturate(const float4& x) {
			const static auto zero = float4(0.f, 0.f, 0.f, 0.f);
			const static auto one = float4(1.f, 1.f, 1.f, 1.f);
			return  clamp(zero, one, x);
		}
	}
}

#endif
