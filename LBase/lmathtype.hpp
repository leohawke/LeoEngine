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
#include "type_traits.hpp"

#include <cmath>
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

			constexpr data_storage() noexcept = default;

			constexpr data_storage(scalar X, scalar Y, scalar Z, scalar W) noexcept
				:x(X), y(Y), z(Z), w(W)
			{}

			constexpr size_t size() const {
				return 4;
			}

			constexpr const scalar_type& operator[](std::size_t index) const noexcept {
				lassume(index < size());
				return data[index];
			}

			constexpr scalar_type& operator[](std::size_t index) noexcept {
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
		namespace type_details {
			//type structurces
			/*\brief holds type info about component and subparts
			*/
			template<typename scalar>
			struct type_vector2d {
				using component_type = scalar;
				using vector2d_type = data_storage<scalar, 2>;
			};

			template<typename scalar>
			struct type_vector3d {
				using component_type = scalar;
				using vector2d_type = data_storage<scalar, 2>;
				using vector3d_type = data_storage<scalar, 3>;
			};

			template<typename scalar>
			struct type_vector4d {
				using component_type = scalar;
				using vector2d_type = data_storage<scalar, 2>;
				using vector3d_type = data_storage<scalar, 3>;
				using vector4d_type = data_storage<scalar, 3>;
			};

			template<typename type_struct, int index>
			struct component {
				using component_type = typename type_struct::component_type;

				component_type data[1];
				operator component_type&(void) { return (data[index]); }
				operator const component_type&(void) const { return (data[index]); }

				component& operator=(const component_type&value) {
					data[index] = value;
				}
			};

			template<typename type_struct, int index_x, int index_y>
			struct converter_vector2d {
				using component_type = typename type_struct::component_type;
				using vector2d_type = typename type_struct::vector2d_type;

				static vector2d_type convert(component_type* data) {
					return vector2d_type(data[index_x], data[index_y]);
				}
			};

			template<typename type_struct>
			struct converter_vector2d<type_struct, 0, 1> {
				using component_type = typename type_struct::component_type;
				using vector2d_type = typename type_struct::vector2d_type;

				static vector2d_type convert(component_type* data) {
					return reinterpret_cast<vector2d_type>(data[0]);
				}
			};

			template<typename type_struct>
			struct converter_vector2d<type_struct, 1, 2> {
				using component_type = typename type_struct::component_type;
				using vector2d_type = typename type_struct::vector2d_type;

				static vector2d_type convert(component_type* data) {
					return reinterpret_cast<vector2d_type>(data[1]);
				}
			};

			template<typename type_struct>
			struct converter_vector2d<type_struct, 2, 3> {
				using component_type = typename type_struct::component_type;
				using vector2d_type = typename type_struct::vector2d_type;

				static vector2d_type convert(component_type* data) {
					return reinterpret_cast<vector2d_type>(data[2]);
				}
			};

			template<typename type_struct, bool anti, int index_x, int index_y>
			struct sub_vector2d {
				using component_type = typename type_struct::component_type;
				using vector2d_type = typename type_struct::vector2d_type;

				component_type data[2];

				operator typename converter_vector2d<type_struct, index_x, index_y>::vector2d_type(void)
				{
					return converter_vector2d<type_struct, index_x, index_y>::convert(data);
				}

				template<typename type, int ind_x, int ind_y>
				sub_vector2d& operator=(const sub_vector2d<type, anti, ind_x, ind_y>& value) {
					data[index_x] = value.data[ind_x];
					data[index_y] = value.data[ind_y];
					return *this;
				}

				sub_vector2d& operator=(const vector2d_type& value) {
					data[index_x] = value.x;
					data[index_y] = value.y;
					return *this;
				}
			};

			template<typename type_struct, bool anti = false>
			struct vector_2d_impl {
				using component_type = typename type_struct::component_type;
				using scalar_type = component_type;
				using vec_type = vector_2d_impl;

				union {
					type_details::component<type_struct, 0> x;
					type_details::component<type_struct, 1> y;
					type_details::sub_vector2d<type_struct, anti, 0, 1> xy;
					type_details::sub_vector2d<type_struct, anti, 1, 0> yx;

					scalar_type data[2];
				};

				constexpr vector_2d_impl(scalar_type x, scalar_type y) noexcept
					:data{x,y}
				{
				}

				constexpr vector_2d_impl() noexcept = default;

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

				const scalar_type* begin() const noexcept {
					return data + 0;
				}

				const scalar_type* end() const noexcept {
					return data + size();
				}

				scalar_type* begin() noexcept {
					return data + 0;
				}

				scalar_type* end() noexcept {
					return data + size();
				}
			};
		}

		template<typename scalar>
		using vector_2d = type_details::vector_2d_impl<type_details::type_vector2d<scalar>>;

		template<typename _type>
		struct is_lmathtype :false_
		{};

		template<typename _type>
		constexpr auto is_lmathtype_v = is_lmathtype<_type>::value;

		//The float2 data type
		struct lalignas(16) float2 :vector_2d<float>
		{
			using vec_type::vec_type;
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

		template<>
		struct is_lmathtype<float2> :true_
		{};

		template<>
		struct is_lmathtype<float3> :true_
		{};

		template<>
		struct is_lmathtype<float4> :true_
		{};

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

		struct lalignas(4) half2 :vector_2d<half>
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
				return float2(((half)x).operator float(), ((half)y).operator float());
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


#endif
