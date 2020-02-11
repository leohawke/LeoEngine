/*!	\file lmath.hpp
\ingroup LBase
\brief 定义在lmathtype上的操作。
*/

#include "LBase/lmathtype.hpp"
#include "LBase/lmathquaternion.hpp"

#ifndef LBase_lmath_hpp
#define LBase_lmath_hpp 1

//vector
namespace leo::math {
	template<typename T>
	inline vector3<T>& operator/=(vector3<T>&l, T r) {
		l.x /= r;
		l.y /= r;
		l.z /= r;
		return l;
	}

	template<typename T>
	inline vector3<T>& operator*=(vector3<T>& l, T r) {
		l.x *= r;
		l.y *= r;
		l.z *= r;
		return l;
	}

	template<typename _type>
	inline vector4<_type>& operator/=(vector4<_type>& l, _type r) {
		l.x /= r;
		l.y /= r;
		l.z /= r;
		l.w /= r;
		return l;
	}
}

//vector<float>
namespace leo {
	namespace math {
		inline bool zero_float(float l) {
			return (l <= std::numeric_limits<float>::epsilon()) && (l >= -std::numeric_limits<float>::epsilon());
		}

		inline bool float_equal(float l, float r) {
			return zero_float(l - r);
		}

		inline constexpr float
			sgn(float value) noexcept {
			return value < 0.f ? -1.f : (value > 0.f ? 1.f : +0.f);
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

		/* \!brief float3运算符重载
		*/
		//@{
		inline bool operator==(const float3& l, const float3& r) noexcept {
			return float_equal(l.x, r.x) && float_equal(l.y, r.y)
				&& float_equal(l.z, r.z);
		}

		inline bool operator!=(const float3& l, const float3& r) noexcept {
			return !(l == r);
		}

		inline float3& operator+=(float3& l, const float3& r) noexcept {
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
		inline float3& operator*=(float3& l, _type r) noexcept {
			l.x *= r;
			l.y *= r;
			l.z *= r;
			return l;
		}

		template<typename T>
		inline float3& operator/=(float3&l, T r) noexcept {
			l.x /= r;
			l.y /= r;
			l.z /= r;
			return l;
		}

		inline float dot(const vector3<float> &l, const vector3<float>& r) noexcept {
			return l.x*r.x + l.y*r.y + l.z*r.z;
		}

		inline float3 cross(const float3&l, const float3& r) noexcept {
			return {
				l.y*r.z - l.z*r.y,
				l.z*r.x - l.x*r.z,
				l.x*r.y - l.y*r.x };
		}
		//@}

		/* \!brief float2运算符重载
		*/
		//@{
		inline bool operator==(const float2& l, const float2& r) noexcept {
			return float_equal(l.x, r.x) && float_equal(l.y, r.y);
		}
		//@}

		template<typename _type, limpl(typename = enable_if_t<is_lmathtype_v<_type>>)>
		inline constexpr _type operator+(const _type& l, const _type& r) noexcept {
			auto ret = l;
			ret += r;
			return ret;
		}

		template<typename _type, limpl(typename = enable_if_t<is_lmathtype_v<_type>>)>
		inline constexpr _type operator-(const _type& l, const _type& r) noexcept {
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
	inline constexpr basic_quaternion< scalar> rotation_axis(const vector3<scalar>& axis, scalar angle) noexcept {
		float sa = sinf(angle*0.5f);
		float ca = cosf(angle*0.5f);

		if (zero_float(length_sq(axis)))
			return { sa,sa,sa,ca };
		else
			return { sa*normalize(axis),ca };
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
	inline constexpr bool basic_quaternion<scalar>::operator==(const basic_quaternion<scalar>& rhs) const noexcept {
		return x == rhs.x && y == rhs.y && z == rhs.z &&w == rhs.w;
	}


	inline constexpr float3 transformquat(float3 v, quaternion q) noexcept {
		float3 quat_v{ q.x,q.y,q.z };
		return v + cross(quat_v, cross(quat_v, v) + q.w*v) * 2;
	}

	inline float4 to_quaternion(float4x4 mat) noexcept
	{
		float4 quat {};

		float s = 0;
		const float tr = mat(0, 0) + mat(1, 1) + mat(2, 2) + 1;

		// check the diagonal
		if (tr > 1)
		{
			s = sqrt(tr);
			quat.w = s * 0.5f;
			s = 0.5f / s;
			quat.x = (mat(1, 2) - mat(2, 1)) * s;
			quat.y = (mat(2, 0) - mat(0, 2)) * s;
			quat.z = (mat(0, 1) - mat(1, 0)) * s;
		}
		else
		{
			int maxi = 0;
			float maxdiag = mat(0, 0);
			for (int i = 1; i < 3; ++i)
			{
				if (mat(i, i) > maxdiag)
				{
					maxi = i;
					maxdiag = mat(i, i);
				}
			}

			switch (maxi)
			{
			case 0:
				s = sqrt((mat(0, 0) - (mat(1, 1) + mat(2, 2))) + 1);

				quat.x = s * 0.5f;

				if (!float_equal(s, 0))
				{
					s = 0.5f / s;
				}

				quat.w = (mat(1, 2) - mat(2, 1)) * s;
				quat.y = (mat(1, 0) + mat(0, 1)) * s;
				quat.z = (mat(2, 0) + mat(0, 2)) * s;
				break;

			case 1:
				s = sqrt((mat(1, 1) - (mat(2, 2) + mat(0, 0))) + 1);
				quat.y = s * 0.5f;

				if (!float_equal(s, 0))
				{
					s = 0.5f / s;
				}

				quat.w = (mat(2, 0) - mat(0, 2)) * s;
				quat.z = (mat(2, 1) + mat(1, 2)) * s;
				quat.x = (mat(0, 1) + mat(1, 0)) * s;
				break;

			case 2:
			default:
				s = sqrt((mat(2, 2) - (mat(0, 0) + mat(1, 1))) + 1);

				quat.z = s * 0.5f;

				if (!float_equal(s, 0))
				{
					s = 0.5f / s;
				}

				quat.w = (mat(0, 1) - mat(1, 0)) * s;
				quat.x = (mat(0, 2) + mat(2, 0)) * s;
				quat.y = (mat(1, 2) + mat(2, 1)) * s;
				break;
			}
		}

		return normalize(quat);
	}

	//bits default is 8
	inline float4 to_quaternion(float3 t, float3 b, float3 n, int bits = 8)
	{
		float k = 1;
		if (dot(b, cross(n, t)) < 0)
		{
			k = -1;
		}

		float4x4 tangent_frame(
			float4(t, 0),
			float4(b, 0),
			float4(n, 0),
			float4(0,0,0,1)
		);

		auto tangent_quat = to_quaternion(tangent_frame);
		if (tangent_quat.w < 0)
			tangent_quat = float4(0, 0, 0, 0) - tangent_quat;

		if (bits > 0)
		{
			auto const bias = 1.f / ((1UL << (bits - 1)) - 1);
			if (tangent_quat.w < bias)
			{
				auto const factor = sqrt(1 - bias * bias);
				tangent_quat.x *= factor;
				tangent_quat.y *= factor;
				tangent_quat.z *= factor;
				tangent_quat.w = bias;
			}
		}

		if (k < 0)
		{
			tangent_quat = float4(0, 0, 0, 0) - tangent_quat;
		}

		return tangent_quat;
	}
}

//matrix
namespace leo::math {
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

	inline float4x4 mul(const float4x4& lhs, const float4x4& rhs) noexcept {
		auto tmp(transpose(rhs));

		return {
			{dot(lhs[0],tmp[0]),dot(lhs[0],tmp[1]),dot(lhs[0],tmp[2]),dot(lhs[0],tmp[3])},
			{ dot(lhs[1],tmp[0]),dot(lhs[1],tmp[1]),dot(lhs[1],tmp[2]),dot(lhs[1],tmp[3]) },
			{ dot(lhs[2],tmp[0]),dot(lhs[2],tmp[1]),dot(lhs[2],tmp[2]),dot(lhs[2],tmp[3]) },
			{ dot(lhs[3],tmp[0]),dot(lhs[3],tmp[1]),dot(lhs[3],tmp[2]),dot(lhs[3],tmp[3]) },
		};
	}

	inline float4x4 operator*(const float4x4& lhs, const float4x4& rhs) noexcept {
		return mul(lhs, rhs);
	}

	inline float4 transform(const float4& l, const float4x4& m) noexcept {
		auto tm = transpose(m);
		return{ dot(l,tm[0]),dot(l,tm[1]) ,dot(l,tm[2]) ,dot(l,tm[3]) };
	}

	inline float3 transformpoint(const float3&l, const float4x4&m) noexcept {
		auto v = float4(l, 1);
		auto tm = transpose(m);
		return { dot(v,tm[0]),dot(v,tm[1]) ,dot(v,tm[2]) };
	}

	inline float determinant(const float4x4& rhs) noexcept {
		float const _3142_3241(rhs(2, 0) * rhs(3, 1) - rhs(2, 1) * rhs(3, 0));
		float const _3143_3341(rhs(2, 0) * rhs(3, 2) - rhs(2, 2) * rhs(3, 0));
		float const _3144_3441(rhs(2, 0) * rhs(3, 3) - rhs(2, 3) * rhs(3, 0));
		float const _3243_3342(rhs(2, 1) * rhs(3, 2) - rhs(2, 2) * rhs(3, 1));
		float const _3244_3442(rhs(2, 1) * rhs(3, 3) - rhs(2, 3) * rhs(3, 1));
		float const _3344_3443(rhs(2, 2) * rhs(3, 3) - rhs(2, 3) * rhs(3, 2));

		return rhs(0, 0) * (rhs(1, 1) * _3344_3443 - rhs(1, 2) * _3244_3442 + rhs(1, 3) * _3243_3342)
			- rhs(0, 1) * (rhs(1, 0) * _3344_3443 - rhs(1, 2) * _3144_3441 + rhs(1, 3) * _3143_3341)
			+ rhs(0, 2) * (rhs(1, 0) * _3244_3442 - rhs(1, 1) * _3144_3441 + rhs(1, 3) * _3142_3241)
			- rhs(0, 3) * (rhs(1, 0) * _3243_3342 - rhs(1, 1) * _3143_3341 + rhs(1, 2) * _3142_3241);
	}

	inline float4x4 inverse(const float4x4& rhs) noexcept {
		float const _2132_2231(rhs(1, 0) * rhs(2, 1) - rhs(1, 1) * rhs(2, 0));
		float const _2133_2331(rhs(1, 0) * rhs(2, 2) - rhs(1, 2) * rhs(2, 0));
		float const _2134_2431(rhs(1, 0) * rhs(2, 3) - rhs(1, 3) * rhs(2, 0));
		float const _2142_2241(rhs(1, 0) * rhs(3, 1) - rhs(1, 1) * rhs(3, 0));
		float const _2143_2341(rhs(1, 0) * rhs(3, 2) - rhs(1, 2) * rhs(3, 0));
		float const _2144_2441(rhs(1, 0) * rhs(3, 3) - rhs(1, 3) * rhs(3, 0));
		float const _2233_2332(rhs(1, 1) * rhs(2, 2) - rhs(1, 2) * rhs(2, 1));
		float const _2234_2432(rhs(1, 1) * rhs(2, 3) - rhs(1, 3) * rhs(2, 1));
		float const _2243_2342(rhs(1, 1) * rhs(3, 2) - rhs(1, 2) * rhs(3, 1));
		float const _2244_2442(rhs(1, 1) * rhs(3, 3) - rhs(1, 3) * rhs(3, 1));
		float const _2334_2433(rhs(1, 2) * rhs(2, 3) - rhs(1, 3) * rhs(2, 2));
		float const _2344_2443(rhs(1, 2) * rhs(3, 3) - rhs(1, 3) * rhs(3, 2));
		float const _3142_3241(rhs(2, 0) * rhs(3, 1) - rhs(2, 1) * rhs(3, 0));
		float const _3143_3341(rhs(2, 0) * rhs(3, 2) - rhs(2, 2) * rhs(3, 0));
		float const _3144_3441(rhs(2, 0) * rhs(3, 3) - rhs(2, 3) * rhs(3, 0));
		float const _3243_3342(rhs(2, 1) * rhs(3, 2) - rhs(2, 2) * rhs(3, 1));
		float const _3244_3442(rhs(2, 1) * rhs(3, 3) - rhs(2, 3) * rhs(3, 1));
		float const _3344_3443(rhs(2, 2) * rhs(3, 3) - rhs(2, 3) * rhs(3, 2));

		float const det(determinant(rhs));
		if (zero_float(det))
		{
			return rhs;
		}
		else
		{
			float invDet(1 / det);

			return {
				{
					+invDet * (rhs(1, 1) * _3344_3443 - rhs(1, 2) * _3244_3442 + rhs(1, 3) * _3243_3342),
					-invDet * (rhs(0, 1) * _3344_3443 - rhs(0, 2) * _3244_3442 + rhs(0, 3) * _3243_3342),
					+invDet * (rhs(0, 1) * _2344_2443 - rhs(0, 2) * _2244_2442 + rhs(0, 3) * _2243_2342),
					-invDet * (rhs(0, 1) * _2334_2433 - rhs(0, 2) * _2234_2432 + rhs(0, 3) * _2233_2332)
				},

				{
					-invDet * (rhs(1, 0) * _3344_3443 - rhs(1, 2) * _3144_3441 + rhs(1, 3) * _3143_3341),
					+invDet * (rhs(0, 0) * _3344_3443 - rhs(0, 2) * _3144_3441 + rhs(0, 3) * _3143_3341),
					-invDet * (rhs(0, 0) * _2344_2443 - rhs(0, 2) * _2144_2441 + rhs(0, 3) * _2143_2341),
					+invDet * (rhs(0, 0) * _2334_2433 - rhs(0, 2) * _2134_2431 + rhs(0, 3) * _2133_2331) },

				{
					+invDet * (rhs(1, 0) * _3244_3442 - rhs(1, 1) * _3144_3441 + rhs(1, 3) * _3142_3241),
					-invDet * (rhs(0, 0) * _3244_3442 - rhs(0, 1) * _3144_3441 + rhs(0, 3) * _3142_3241),
					+invDet * (rhs(0, 0) * _2244_2442 - rhs(0, 1) * _2144_2441 + rhs(0, 3) * _2142_2241),
					-invDet * (rhs(0, 0) * _2234_2432 - rhs(0, 1) * _2134_2431 + rhs(0, 3) * _2132_2231)
				},

				{
					-invDet * (rhs(1, 0) * _3243_3342 - rhs(1, 1) * _3143_3341 + rhs(1, 2) * _3142_3241),
					+invDet * (rhs(0, 0) * _3243_3342 - rhs(0, 1) * _3143_3341 + rhs(0, 2) * _3142_3241),
					-invDet * (rhs(0, 0) * _2243_2342 - rhs(0, 1) * _2143_2341 + rhs(0, 2) * _2142_2241),
					+invDet * (rhs(0, 0) * _2233_2332 - rhs(0, 1) * _2133_2331 + rhs(0, 2) * _2132_2231)
				}
			};
		}
	}

	inline float4x4 to_matrix(quaternion quat) {
		float const x2(quat.x + quat.x);
		float const y2(quat.y + quat.y);
		float const z2(quat.z + quat.z);

		float const xx2(quat.x * x2), xy2(quat.x * y2), xz2(quat.x * z2);
		float const yy2(quat.y * y2), yz2(quat.y * z2), zz2(quat.z * z2);
		float const wx2(quat.w * x2), wy2(quat.w * y2), wz2(quat.w * z2);

		return {
			{1 - yy2 - zz2,	xy2 + wz2,		xz2 - wy2,		0},
			{xy2 - wz2,		1 - xx2 - zz2,	yz2 + wx2,		0},
			{xz2 + wy2,		yz2 - wx2,		1 - xx2 - yy2,	0},
			{0,				0,				0,				1}
		};
	}

	inline float4x4 translation(float x, float y, float z) {
		return {
			{1,0,0,0},
			{0,1,0,0},
			{0,0,1,0},
			{x,y,z,1}
		};
	}

	inline float4x4 translation(float3 trans) {
		return translation(trans.x, trans.y, trans.z);
	}

	inline float4x4 make_matrix(quaternion rotation, float3 trans) {
		return to_matrix(rotation)*translation(trans);
	}

	inline float4x4 make_matrix(float3 rotation_center, quaternion rotation, float3 trans) {
		return  translation(-rotation_center) * to_matrix(rotation)*translation(trans + rotation_center);
	}
}

//template function
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