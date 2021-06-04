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
#include "ref.hpp"
#include "cassert.h"

#include <cmath>
#include <algorithm> //std::max,std::min
#include <cstring> //std::memcpy 
#include <array> //std::array
#include <limits>
//data type
namespace leo {
	namespace math {
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
			template<typename scalar, typename vector2d_impl>
			struct type_vector2d {
				using component_type = scalar;
				using vector2d_type = vector2d_impl;
			};

			template<typename scalar, typename vector2d_impl, typename vector3d_impl>
			struct type_vector3d {
				using component_type = scalar;
				using vector2d_type = vector2d_impl;
				using vector3d_type = vector3d_impl;
			};

			template<typename scalar, typename vector2d_impl, typename vector3d_impl,typename vector4d_impl>
			struct type_vector4d {
				using component_type = scalar;
				using vector2d_type = vector2d_impl;
				using vector3d_type = vector3d_impl;
				using vector4d_type = vector4d_impl;
			};

			template<typename type_struct, int index>
			struct component {
				using component_type = typename type_struct::component_type;

				component_type data[1];

				operator component_type() const { return (data[index]); }
				operator component_type&() { return (data[index]); }

				const component_type* operator&() const { return data+index; }
				component_type* operator&() { return data+index; }

				component& operator=(const component_type&value) {
					data[index] = value;
					return *this;
				}
			};

			template<typename type_struct, int index_l,int index_r>
			void swap(component<type_struct, index_l>& lhs, component<type_struct, index_r>& rhs) {
				using component_type = typename type_struct::component_type;

				component_type T = rhs;
				rhs = lhs;
				lhs = T;
			}

			template<typename type_struct, int index_x, int index_y>
			struct converter_vector2d {
				using component_type = typename type_struct::component_type;
				using vector2d_type = typename type_struct::vector2d_type;

				static constexpr vector2d_type convert(const component_type* data) {
					return vector2d_type(data[index_x], data[index_y]);
				}
			};


			template<typename type_struct, bool anti, int index_x, int index_y>
			struct sub_vector2d {
				using component_type = typename type_struct::component_type;
				using vector2d_type = typename type_struct::vector2d_type;

				component_type data[2];

				constexpr operator typename converter_vector2d<type_struct, index_x, index_y>::vector2d_type()
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
					:data{ x,y }
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

			template<typename type_struct, int index_x, int index_y, int index_z>
			struct converter_vector3d {
				using component_type = typename type_struct::component_type;
				using vector3d_type = typename type_struct::vector3d_type;

				static constexpr vector3d_type convert(const component_type* data) {
					return vector3d_type(data[index_x], data[index_y], data[index_z]);
				}
			};

			template<typename type_struct, bool anti, int index_x, int index_y, int index_z>
			struct sub_vector3d {
				using component_type = typename type_struct::component_type;
				using vector3d_type = typename type_struct::vector3d_type;

				component_type data[3];

				constexpr operator typename converter_vector3d<type_struct, index_x, index_y, index_z>::vector3d_type() const
				{
					return converter_vector3d<type_struct, index_x, index_y, index_z>::convert(data);
				}

				template<typename type, int ind_x, int ind_y, int ind_z>
				sub_vector3d& operator=(const sub_vector3d<type, anti, ind_x, ind_y, ind_z>& value) {
					data[index_x] = value.data[ind_x];
					data[index_y] = value.data[ind_y];
					data[index_z] = value.data[ind_z];
					return *this;
				}

				sub_vector3d& operator=(const vector3d_type& value) {
					data[index_x] = value.x;
					data[index_y] = value.y;
					data[index_z] = value.z;
					return *this;
				}
			};

			template<typename type_struct, bool anti = false>
			struct vector_3d_impl {
				using component_type = typename type_struct::component_type;
				using scalar_type = component_type;
				using vec_type = vector_3d_impl;

				union {
					type_details::component<type_struct, 0> x;
					type_details::component<type_struct, 1> y;
					type_details::component<type_struct, 2> z;
					type_details::sub_vector2d<type_struct, anti, 0, 1> xy;
					type_details::sub_vector2d<type_struct, anti, 0, 2> xz;
					type_details::sub_vector2d<type_struct, anti, 1, 0> yx;
					type_details::sub_vector2d<type_struct, anti, 1, 2> yz;
					type_details::sub_vector2d<type_struct, anti, 2, 0> zx;
					type_details::sub_vector2d<type_struct, anti, 2, 1> zy;

					type_details::sub_vector3d<type_struct, anti, 0, 1, 2> xyz;
					type_details::sub_vector3d<type_struct, anti, 0, 2, 1> xzy;
					type_details::sub_vector3d<type_struct, anti, 1, 0, 2> yxz;
					type_details::sub_vector3d<type_struct, anti, 1, 2, 0> yzx;
					type_details::sub_vector3d<type_struct, anti, 2, 0, 1> zxy;
					type_details::sub_vector3d<type_struct, anti, 2, 1, 0> zyx;

					scalar_type data[3];
				};

				constexpr vector_3d_impl(scalar_type x, scalar_type y, scalar_type z) noexcept
					:data{ x,y ,z }
				{
				}

				constexpr vector_3d_impl() noexcept = default;

				constexpr size_t size() const {
					return 3;
				}

				const constexpr scalar_type& operator[](std::size_t index) const noexcept {
					lassume(index < size());
					return data[index];
				}

				constexpr scalar_type& operator[](std::size_t index) noexcept {
					lassume(index < size());
					return data[index];
				}

				const constexpr scalar_type* begin() const noexcept {
					return data + 0;
				}

				const constexpr scalar_type* end() const noexcept {
					return data + size();
				}

				constexpr scalar_type* begin() noexcept {
					return data + 0;
				}

				constexpr scalar_type* end() noexcept {
					return data + size();
				}
			};

			template<typename type_struct, int index_x, int index_y, int index_z,int index_w>
			struct converter_vector4d {
				using component_type = typename type_struct::component_type;
				using vector4d_type = typename type_struct::vector4d_type;

				static vector4d_type convert(component_type* data) {
					return vector4d_type(data[index_x], data[index_y], data[index_z],data[index_w]);
				}
			};

			template<typename type_struct, bool anti, int index_x, int index_y, int index_z,int index_w>
			struct sub_vector4d {
				using component_type = typename type_struct::component_type;
				using vector4d_type = typename type_struct::vector4d_type;

				component_type data[4];

				operator typename converter_vector4d<type_struct, index_x, index_y, index_z,index_w>::vector4d_type(void)
				{
					return converter_vector4d<type_struct, index_x, index_y, index_z,index_w>::convert(data);
				}

				template<typename type, int ind_x, int ind_y, int ind_z,int ind_w>
				sub_vector4d& operator=(const sub_vector4d<type, anti, ind_x, ind_y, ind_z,ind_w>& value) {
					data[index_x] = value.data[ind_x];
					data[index_y] = value.data[ind_y];
					data[index_z] = value.data[ind_z];
					data[index_w] = value.data[ind_w];
					return *this;
				}

				sub_vector4d& operator=(const vector4d_type& value) {
					data[index_x] = value.x;
					data[index_y] = value.y;
					data[index_z] = value.z;
					data[index_w] = value.w;
					return *this;
				}
			};

			template<typename type_struct, bool anti = false>
			struct vector_4d_impl {
				using component_type = typename type_struct::component_type;
				using vector3d_type = typename type_struct::vector3d_type;
				using scalar_type = component_type;
				using vec_type = vector_4d_impl;

				union {
					type_details::component<type_struct, 0> x;
					type_details::component<type_struct, 1> y;
					type_details::component<type_struct, 2> z;
					type_details::component<type_struct, 3> w;

					type_details::component<type_struct, 0> r;
					type_details::component<type_struct, 1> g;
					type_details::component<type_struct, 2> b;
					type_details::component<type_struct, 3> a;

					type_details::sub_vector2d<type_struct, anti, 0, 1> xy;
					type_details::sub_vector2d<type_struct, anti, 0, 2> xz;
					type_details::sub_vector2d<type_struct, anti, 0, 3> xw;
					type_details::sub_vector2d<type_struct, anti, 1, 0> yx;
					type_details::sub_vector2d<type_struct, anti, 1, 2> yz;
					type_details::sub_vector2d<type_struct, anti, 1, 3> yw;
					type_details::sub_vector2d<type_struct, anti, 2, 0> zx;
					type_details::sub_vector2d<type_struct, anti, 2, 1> zy;
					type_details::sub_vector2d<type_struct, anti, 2, 3> zw;
					type_details::sub_vector2d<type_struct, anti, 3, 0> wx;
					type_details::sub_vector2d<type_struct, anti, 3, 1> wy;
					type_details::sub_vector2d<type_struct, anti, 3, 2> wz;

					type_details::sub_vector3d<type_struct, anti, 0, 1, 2> xyz;
					type_details::sub_vector3d<type_struct, anti, 0, 2, 1> xzy;
					type_details::sub_vector3d<type_struct, anti, 0, 1, 3> xyw;
					type_details::sub_vector3d<type_struct, anti, 0, 3, 1> xwy;
					type_details::sub_vector3d<type_struct, anti, 0, 2, 3> xzw;
					type_details::sub_vector3d<type_struct, anti, 0, 3, 2> xwz;
					type_details::sub_vector3d<type_struct, anti, 1, 0, 2> yxz;
					type_details::sub_vector3d<type_struct, anti, 1, 2, 0> yzx;
					type_details::sub_vector3d<type_struct, anti, 1, 0, 3> yxw;
					type_details::sub_vector3d<type_struct, anti, 1, 3, 0> ywx;
					type_details::sub_vector3d<type_struct, anti, 1, 2, 3> yzw;
					type_details::sub_vector3d<type_struct, anti, 1, 3, 2> ywz;
					type_details::sub_vector3d<type_struct, anti, 2, 0, 1> zxy;
					type_details::sub_vector3d<type_struct, anti, 2, 1, 0> zyx;
					type_details::sub_vector3d<type_struct, anti, 2, 0, 3> zxw;
					type_details::sub_vector3d<type_struct, anti, 2, 3, 0> zwx;
					type_details::sub_vector3d<type_struct, anti, 2, 1, 3> zyw;
					type_details::sub_vector3d<type_struct, anti, 2, 3, 1> zwy;
					type_details::sub_vector3d<type_struct, anti, 3, 0, 1> wxy;
					type_details::sub_vector3d<type_struct, anti, 3, 1, 0> wyx;
					type_details::sub_vector3d<type_struct, anti, 3, 0, 2> wxz;
					type_details::sub_vector3d<type_struct, anti, 3, 2, 0> wzx;
					type_details::sub_vector3d<type_struct, anti, 3, 1, 2> wyz;
					type_details::sub_vector3d<type_struct, anti, 3, 2, 1> wzy;

					type_details::sub_vector4d<type_struct, anti, 0, 1, 2, 3>  xyzw;
					type_details::sub_vector4d<type_struct, anti, 0, 1, 3, 2>  xywz; 
					type_details::sub_vector4d<type_struct, anti, 0, 2, 1, 3>  xzyw;
					type_details::sub_vector4d<type_struct, anti, 0, 2, 3, 1>  xzwy;
					type_details::sub_vector4d<type_struct, anti, 0, 3, 1, 2>  xwyz; 
					type_details::sub_vector4d<type_struct, anti, 0, 3, 2, 1>  xwzy;
					type_details::sub_vector4d<type_struct, anti, 1, 0, 2, 3>  yxzw; 
					type_details::sub_vector4d<type_struct, anti, 1, 0, 3, 2>  yxwz;
					type_details::sub_vector4d<type_struct, anti, 1, 2, 0, 3>  yzxw;
					type_details::sub_vector4d<type_struct, anti, 1, 2, 3, 0>  yzwx;
					type_details::sub_vector4d<type_struct, anti, 1, 3, 0, 2>  ywxz; 
					type_details::sub_vector4d<type_struct, anti, 1, 3, 2, 0>  ywzx; 
					type_details::sub_vector4d<type_struct, anti, 2, 0, 1, 3>  zxyw; 
					type_details::sub_vector4d<type_struct, anti, 2, 0, 3, 1>  zxwy;
					type_details::sub_vector4d<type_struct, anti, 2, 1, 0, 3>  zyxw; 
					type_details::sub_vector4d<type_struct, anti, 2, 1, 3, 0>  zywx;
					type_details::sub_vector4d<type_struct, anti, 2, 3, 0, 1>  zwxy;
					type_details::sub_vector4d<type_struct, anti, 2, 3, 1, 0>  zwyx; 
					type_details::sub_vector4d<type_struct, anti, 3, 0, 1, 2>  wxyz;
					type_details::sub_vector4d<type_struct, anti, 3, 0, 2, 1>  wxzy; 
					type_details::sub_vector4d<type_struct, anti, 3, 1, 0, 2>  wyxz; 
					type_details::sub_vector4d<type_struct, anti, 3, 1, 2, 0>  wyzx; 
					type_details::sub_vector4d<type_struct, anti, 3, 2, 0, 1>  wzxy; 
					type_details::sub_vector4d<type_struct, anti, 3, 2, 1, 0>  wzyx;

					scalar_type data[4];
				};

				constexpr vector_4d_impl(scalar_type x, scalar_type y, scalar_type z,scalar_type w) noexcept
					:data{ x,y ,z ,w}
				{
				}

				constexpr vector_4d_impl(vector3d_type xyz, scalar_type w) noexcept
					:data{ xyz.x,xyz.y,xyz.z,w }
				{
				}

				constexpr vector_4d_impl() noexcept = default;

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

		template<typename scalar, typename type>
		using vector_2d = type_details::vector_2d_impl<type_details::type_vector2d<scalar, type>>;

		template<typename scalar,typename type1, typename type2>
		using vector_3d = type_details::vector_3d_impl<type_details::type_vector3d<scalar, type1,type2>>;

		template<typename scalar, typename vector2d_impl, typename vector3d_impl, typename vector4d_impl>
		using vector_4d = type_details::vector_4d_impl<type_details::type_vector4d<scalar, vector2d_impl, vector3d_impl, vector4d_impl>>;

		template<typename _type>
		struct is_lmathtype :false_
		{};

		template<typename _type>
		constexpr auto is_lmathtype_v = is_lmathtype<_type>::value;

		template<typename scalar>
		struct vector2 : vector_2d<scalar, vector2<scalar>>
		{
			using base = vector_2d<scalar, vector2<scalar>>;
			using base::base;
		};

		//The float2 data type
		using float2 = vector2<float>;

		template<typename scalar>
		struct vector3 : vector_3d<scalar, vector2<scalar>,vector3<scalar>>
		{
			using base = vector_3d<scalar, vector2<scalar>, vector3<scalar>>;
			using base::base;
		};

		using float3 = vector3<float>;
		

		template<typename scalar>
		struct vector4 : vector_4d<scalar, vector2<scalar>, vector3<scalar>,vector4<scalar>>
		{
			using base = vector_4d<scalar, vector2<scalar>, vector3<scalar>,vector4<scalar>>;
			using base::base;
		};

		using float4 = vector4<float>;

		template<>
		struct is_lmathtype<vector2<float>> :true_
		{};

		template<>
		struct is_lmathtype<vector3<float>> :true_
		{};

		template<>
		struct is_lmathtype<vector4<float>> :true_
		{};

		//The float3x3 data type
		struct float3x3 {
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
		struct float4x4 {
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

			float3 GetColumn(int32 i) const noexcept
			{
				return float3(r[0][i], r[1][i], r[2][i]);
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

			const static float4x4 identity;
		};


		lselectany const float4x4 float4x4::identity = {
			{1,0,0,0},
			{0,1,0,0},
			{0,0,1,0},
			{0,0,0,1},
			};

		using int2 = vector2<int>;

		struct int3 : vector3<int>
		{
			using base = vector3<int>;
			using base::base;

			int3 operator+(const int3& rhs) const {
				return int3(x + rhs.x, y + rhs.y, z + rhs.z);
			}

			int3& operator+=(const int3& rhs) {
				*this = *this + rhs;
				return *this;
			}

			explicit int3(int value)
				:base(value,value,value)
			{}
		};

		struct uint2 :vector2<uint32>
		{
			using base = vector2<uint32>;
			using base::base;
		};

		struct uint4 :vector4<uint32>
		{
			using base = vector4<uint32>;
			using base::base;
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

		struct lalignas(4) half2 :vector_2d<half, half2>
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
