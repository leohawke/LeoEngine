/*!	\file lmath.hpp
\ingroup LBase
\brief 定义在lmathtype上的操作。
*/

#include "LBase/lmathtype.hpp"
#include "LBase/lmathquaternion.hpp"

#ifndef LBase_lmath_hpp
#define LBase_lmath_hpp 1

//data base function
namespace leo {
	namespace math {
		inline bool zero_float(float l) {
			return (l <= std::numeric_limits<float>::epsilon()) && (l >= -std::numeric_limits<float>::epsilon());
		}

		inline bool float_equal(float l, float r) {
			return zero_float(l - r);
		}

		/* \!brief float4运算符重载
		*/
		//@{
		inline bool operator==(const float4& l, const float4& r) noexcept {
			return float_equal(l.x, r.x) && float_equal(l.y, r.y)
				&& float_equal(l.z, r.z) && float_equal(l.w, r.w);
		}

		inline bool operator!=(const float4& l, const float4& r) noexcept {
			return !(l == r);
		}

		inline float4& operator+=(float4& l, const float4& r) {
			l.x += r.x;
			l.y += r.y;
			l.z += r.z;
			l.w += r.w;
			return l;
		}

		inline float4& operator-=(float4& l, const float4& r) {
			l.x -= r.x;
			l.y -= r.y;
			l.z -= r.z;
			l.w -= r.w;
			return l;
		}

		template<typename _type>
		inline float4& operator*=(float4& l, _type r) {
			l.x *= r;
			l.y *= r;
			l.z *= r;
			l.w *= r;
			return l;
		}

		template<typename _type>
		inline float4& operator/=(float4& l, _type r) {
			l.x /= r;
			l.y /= r;
			l.z /= r;
			l.w /= r;
			return l;
		}

		inline float4& operator/=(float4& l, const float4& r) {
			l.x /= r.x;
			l.y /= r.y;
			l.z /= r.z;
			l.w /= r.w;
			return l;
		}

		inline float dot(const float4&l, const float4& r) {
			return l.x*r.x + l.y*r.y + l.z*r.z + l.w*r.w;
		}
		//@}

		inline bool operator==(const float3& l, const float3& r) noexcept {
			return float_equal(l.x, r.x) && float_equal(l.y, r.y)
				&& float_equal(l.z, r.z);
		}

		inline bool operator!=(const float3& l, const float3& r) noexcept {
			return !(l == r);
		}

		inline float3& operator+=(float3& l, const float3& r) {
			l.x += r.x;
			l.y += r.y;
			l.z += r.z;
			return l;
		}

		inline float3& operator-=(float3& l, const float3& r) {
			l.x -= r.x;
			l.y -= r.y;
			l.z -= r.z;
			return l;
		}

		template<typename _type>
		inline float3& operator*=(float3& l, _type r) {
			l.x *= r;
			l.y *= r;
			l.z *= r;
			return l;
		}

		template<typename T>
		inline float3& operator/=(float3&l, T r) {
			l.x /= r;
			l.y /= r;
			l.z /= r;
			return l;
		}

		inline float dot(const float3&l, const float3& r) {
			return l.x*r.x + l.y*r.y + l.z*r.z;
		}

		inline float3 cross(const float3&l, const float3& r) {
			return {
				l.y*r.z - l.z*r.y,
				l.z*r.x - l.x*r.z,
				l.x*r.y - l.y*r.x };
		}

		template<typename _type, limpl(typename = enable_if_t<is_lmathtype_v<_type>>)>
		inline _type operator+(const _type& l, const _type& r) noexcept {
			auto ret = l;
			ret += r;
			return ret;
		}

		template<typename _type, limpl(typename = enable_if_t<is_lmathtype_v<_type>>)>
		inline _type operator-(const _type& l, const _type& r) noexcept {
			auto ret = l;
			ret -= r;
			return ret;
		}

		template<typename _type, limpl(typename = enable_if_t<is_lmathtype_v<_type>>)>
		inline _type operator*(const _type& l, float r) {
			auto ret = l;
			ret *= r;
			return ret;
		}

		template<typename _type, limpl(typename = enable_if_t<is_lmathtype_v<_type>>)>
		inline _type operator*(float r, const _type& l) {
			auto ret = l;
			ret *= r;
			return ret;
		}

		template<typename _type, limpl(typename = enable_if_t<is_lmathtype_v<_type>>)>
		inline float length(const _type & l) noexcept {
			return sqrtf(dot(l, l));
		}

		template<typename _type, limpl(typename = enable_if_t<is_lmathtype_v<_type>>)>
		inline float length_sq(const _type& l) noexcept {
			return dot(l, l);
		}

		template<typename _type, limpl(typename = enable_if_t<is_lmathtype_v<_type>>)>
		inline _type normalize(const _type& l) noexcept {
			auto mod = length(l);
			auto ret = l;
			ret /= mod;
			return ret;
		}

		template<typename _type, limpl(typename = enable_if_t<is_lmathtype_v<_type>>)>
		inline _type lerp(const _type& l, const _type& r, float t)
		{
			return l * (1.f - t) + r * t;
		}

		inline bool operator==(const float4x4& l, const float4x4& r) noexcept {
			return (l[0] == r[0]) && (l[1] == r[1])
				&& (l[2] == r[2]) && (l[3] == r[3]);
		}

		inline bool operator!=(const float4x4& l, const float4x4& r) noexcept {
			return !(l == r);
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

		inline float4x4 mul(const float4x4& lhs, const float4x4& rhs) {
			auto tmp(transpose(rhs));

			return {
				{dot(lhs[0],tmp[0]),dot(lhs[0],tmp[1]),dot(lhs[0],tmp[2]),dot(lhs[0],tmp[3])},
				{ dot(lhs[1],tmp[0]),dot(lhs[1],tmp[1]),dot(lhs[1],tmp[2]),dot(lhs[1],tmp[3]) },
				{ dot(lhs[2],tmp[0]),dot(lhs[2],tmp[1]),dot(lhs[2],tmp[2]),dot(lhs[2],tmp[3]) },
				{ dot(lhs[3],tmp[0]),dot(lhs[3],tmp[1]),dot(lhs[3],tmp[2]),dot(lhs[3],tmp[3]) },
			};
		}

		inline float4x4 operator*(const float4x4& lhs, const float4x4& rhs) {
			return mul(lhs, rhs);
		}

		inline float4 transform(const float4& l, const float4x4& m) {
			auto tm = transpose(m);
			return{ dot(l,tm[0]),dot(l,tm[1]) ,dot(l,tm[2]) ,dot(l,tm[3]) };
		}

		inline float3 transformpoint(const float3&l, const float4x4&m) {
			auto v = float4(l, 1);
			auto tm = transpose(m);
			return { dot(v,tm[0]),dot(v,tm[1]) ,dot(v,tm[2]) };
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
			return float4(max(l.x, r.x), max(l.y, r.y), max(l.z, r.z), max(l.w, r.w));
		}

		inline float4 min(const float4& l, const float4& r) noexcept {
			return float4(min(l.x, r.x), min(l.y, r.y), min(l.z, r.z), min(l.w, r.w));
		}

		using std::fabs;

		inline float2 abs(const float2& f2) noexcept {
			return float2(fabs(f2.x), fabs(f2.y));
		}

		inline float3 abs(const float3& f3) noexcept {
			return float3(fabs(f3.x), fabs(f3.y), fabs(f3.z));
		}

		inline float4 abs(const float4& f4) noexcept {
			return float4(fabs(f4.x), fabs(f4.y), fabs(f4.z), fabs(f4.w));
		}


		inline uint32 dot(const uint4& l, const uint4& r)
		{
			return l.x*r.x + l.y*r.y + l.z*r.z + l.w*r.w;
		}
	}
}

//quaternion
namespace leo::math {
	template<typename scalar>
	inline constexpr basic_quaternion<scalar> mul(basic_quaternion<scalar> lhs, basic_quaternion<scalar> rhs) noexcept
	{
		return{
			lhs.x * rhs.w - lhs.y * rhs.z + lhs.z * rhs.y + lhs.w * rhs.x,
			lhs.x * rhs.z + lhs.y * rhs.w - lhs.z * rhs.x + lhs.w * rhs.y,
			lhs.y * rhs.x - lhs.x * rhs.y + lhs.z * rhs.w + lhs.w * rhs.z,
			lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
		};
	}

	/* !\brief make a Quaternion of which the sign of the scalar element encodes the Reflection
	*/
	template<typename scalar>
	inline constexpr basic_quaternion< scalar> make_qtangent(const vector3<scalar>& normal, scalar signw) {
		return  basic_quaternion<scalar>{ normal.x, normal.y, normal.z, signw };
	}

	template<typename scalar>
	inline constexpr basic_quaternion<scalar> operator+(basic_quaternion<scalar> lhs, basic_quaternion<scalar> rhs) noexcept {
		return { lhs.x + rhs.x,lhs.y + rhs.y,lhs.z + rhs.z,lhs.w + rhs.w };
	}
	template<typename scalar>
	inline constexpr basic_quaternion<scalar> operator-(basic_quaternion<scalar> lhs, basic_quaternion<scalar> rhs) noexcept {
		return { lhs.x - rhs.x,lhs.y - rhs.y,lhs.z - rhs.z,lhs.w - rhs.w };
	}

	template<typename scalar>
	inline constexpr basic_quaternion<scalar> operator*(basic_quaternion<scalar> lhs, basic_quaternion<scalar> rhs) noexcept {
		return mul(lhs, rhs);
	}

	template<typename scalar>
	inline constexpr basic_quaternion<scalar> operator*(basic_quaternion<scalar> lhs, scalar rhs) noexcept {
		return { lhs.x * rhs,lhs.y * rhs,lhs.z * rhs,lhs * rhs.w };
	}
	template<typename scalar>
	inline constexpr basic_quaternion<scalar> operator/(basic_quaternion<scalar> lhs, scalar rhs) noexcept {
		return { lhs.x / rhs,lhs.y / rhs,lhs.z / rhs,lhs / rhs.w };
	}

	template<typename scalar>
	inline constexpr basic_quaternion<scalar> basic_quaternion<scalar>::operator+() const noexcept {
		return *this;
	}
	template<typename scalar>
	inline constexpr basic_quaternion<scalar> basic_quaternion<scalar>::operator-() const noexcept {
		return { -x,-y,-z,-w };
	}

	template<typename scalar>
	constexpr bool basic_quaternion<scalar>::operator==(const basic_quaternion<scalar>& rhs) const noexcept {
		return x == rhs.x && y == rhs.y && z == rhs.z &&w == rhs.w;
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
			return a * (1 - t) + b * t;
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