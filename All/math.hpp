#ifndef leomath_hpp
#define leomath_hpp
#include <algorithm>
#include <cmath>
//增加插值函数
#include "LeoMathutility.hpp"
#include "memory.hpp"
#pragma warning(push)
#pragma warning(disable : 4838)
#pragma warning(disable : 4458)
#include <DirectXMath.h>
#include <DirectXCollision.h>
#pragma warning(pop)

//#include "declara.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifdef max
#undef max
#undef min
#endif
namespace leo
{
	using namespace DirectX;
	
	inline XMMATRIX loadfloat4x4(const float4x4* m){
#if defined(LM_ARM_NEON_INTRINSICS)
		XMMATRIX M;
		M.r[0] = vld1q_f32_ex( reinterpret_cast<const float*>(&m->r[0]), 128 );
		M.r[1] = vld1q_f32_ex( reinterpret_cast<const float*>(&m->r[1]), 128 );
		M.r[2] = vld1q_f32_ex( reinterpret_cast<const float*>(&m->r[2]), 128 );
		M.r[3] = vld1q_f32_ex( reinterpret_cast<const float*>(&m->r[3]), 128 );
		return M;
#elif defined(LM_SSE_INTRINSICS)
		XMMATRIX M;
		M.r[0] = _mm_load_ps( &m->r[0].x);
		M.r[1] = _mm_load_ps(&m->r[1].x);
		M.r[2] = _mm_load_ps(&m->r[2].x);
		M.r[3] = _mm_load_ps(&m->r[3].x);
		return M;
#else // _XM_VMX128_INTRINSICS_
#endif
	}

	inline void savefloat4x4(float4x4* dst,const XMMATRIX& src){
		XMStoreFloat4x4A((XMFLOAT4X4A*)dst, src);
	}
#if defined LEO_PACK_STORGE
	namespace storge
	{
		using float2 = XMFLOAT2A;
		using float3 = XMFLOAT3A;
		using float4 = XMFLOAT4A;
		using float4x4 = XMFLOAT4X4A;
	};
#else
	namespace storge
	{
		struct float3 {
			float3(const leo::float3& mem) {
				leo::memcpy(*this, mem);
			}
			float3(float _x, float _y, float _z)
				:x(_x), y(_y), z(_z) {

			}
			float3() = default;
			float x, y, z;
		};
		struct float2 {
			float2(const leo::float2& mem) {
				leo::memcpy(*this, mem);
			}
			float2(float _x, float _y)
				:x(_x), y(_y) {

			}
			float2() = default;
			float x, y;
		};
		struct float4 {
			float4(const leo::float4& mem) {
				leo::memcpy(*this, mem);
			}
			float4(float _x, float _y, float _z, float _w)
							:x(_x), y(_y),z(_z),w(_w) {

						}
			float4() = default;
			float x, y, z,w;
		};
		using float4x4 = XMFLOAT4X4;
	};
#endif

	template<typename T>
	struct pack_type :public T{
	};

	template<>
	struct pack_type < storge::float2 > :public float2{
	};

	template<>
	struct pack_type < storge::float3 > :public float3{
	};

	//单位:角度
	inline float sincosd(float* pcos, float degree)
	{
		return sincosr(pcos, degree*LM_RPD);
	}

	// Returns the forward vector from a transform matrix
	inline XMVECTOR XM_CALLCONV  ForwardVec(FXMMATRIX matrix)
	{
#if defined(_XM_NO_INTRINSICS_)
		return XMLoadFloat3(&XMFLOAT3(matrix._31, matrix._32, matrix._33));
#endif
		return matrix.r[2];
	}

	// Returns the back vector from a transform matrix
	inline XMVECTOR XM_CALLCONV  BackVec(FXMMATRIX matrix)
	{
#if defined(_XM_NO_INTRINSICS_)
		return XMLoadFloat3(&XMFLOAT3(-matrix._31, -matrix._32, -matrix._33));
#endif
		return matrix.r[2];
	}

	// Returns the Right vector from a transform matrix
	inline XMVECTOR XM_CALLCONV  RightVec(FXMMATRIX matrix)
	{
#if defined(_XM_NO_INTRINSICS_)
		return XMLoadFloat3(&XMFLOAT3(matrix._11, matrix._12, matrix._13));
#endif
		return matrix.r[0];
	}

	// Returns the left vector from a transform matrix
	inline XMVECTOR XM_CALLCONV  LeftVec(FXMMATRIX matrix)
	{
#if defined(_XM_NO_INTRINSICS_)
		return XMLoadFloat3(&XMFLOAT3(matrix._11, matrix._12, matrix._13));
#endif
		return matrix.r[0];
	}

	// Returns the up vector from a transform matrix
	inline XMVECTOR XM_CALLCONV  UpVec(FXMMATRIX matrix)
	{
#if defined(_XM_NO_INTRINSICS_)
		return XMLoadFloat3(&XMFLOAT3(matrix._21, matrix._22, matrix._23));
#endif
		return matrix.r[1];
	}

	// Returns the down vector from a transform matrix
	inline XMVECTOR XM_CALLCONV  DownVec(FXMMATRIX matrix)
	{
#if defined(_XM_NO_INTRINSICS_)
		return XMLoadFloat3(&XMFLOAT3(matrix._21, matrix._22, matrix._23));
#endif
		return matrix.r[1];
	}

	// Returns the translation vector from a transform matrix
	inline XMVECTOR XM_CALLCONV  TranslationVec(FXMMATRIX matrix)
	{
#if defined(_XM_NO_INTRINSICS_)
		return XMLoadFloat3(&XMFLOAT3(matrix._41, matrix._42, matrix._43));
#endif
		return matrix.r[3];
	}

	template<typename T>
	inline T Normalize(const T& val)
	{
		float len = std::sqrtf(val.x * val.x + val.y * val.y + val.z * val.z);
		T ret(val.x / len, val.y / len, val.z / len);
		return ret;
	}

	template<typename T>
	inline T Add(const T& lhs, const T& rhs)
	{
		return T(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
	}

	template<typename T>
	inline T Subtract(const T& lhs, const T& rhs)
	{
		return T(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
	}


	template<typename T>
	inline T Cross(const T& lhs, const T& rhs)
	{
		return T(
			(lhs.y * rhs.z) - (lhs.z * rhs.y),
			(lhs.z * rhs.x) - (lhs.x * rhs.z),
			(lhs.x * rhs.y) - (lhs.y * rhs.x)
			);
	}

	template<typename T, typename U>
	inline float Dot(const T& lhs, const U& rhs)
	{
		return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
	}

	template<typename T, typename U>
	inline T MultiplyAdd(float s, const T& rhs, const U& add)
	{
		auto x = s*rhs.x + add.x;
		auto y = s*rhs.y + add.y;
		auto z = s*rhs.z + add.z;
		return T(x, y, z);
	}

	template<typename T, typename U>
	inline T MultiplyAdd(const T& s, const T& rhs, const U& add)
	{
		auto x = s.x*rhs.x + add.x;
		auto y = s.y*rhs.y + add.y;
		auto z = s.z*rhs.z + add.z;
		return T(x, y, z);
	}



	//3*3
	inline void QuaternionToMatrix(const float4& quaternion, float4x4& matrix)
	{
		auto x2 = quaternion.x*quaternion.x, y2 = quaternion.y*quaternion.y, z2 = quaternion.z*quaternion.z;
		auto xy = quaternion.x*quaternion.y, wz = quaternion.w*quaternion.z, wx = quaternion.w*quaternion.x;
		auto xz = quaternion.x*quaternion.z, yz = quaternion.y*quaternion.z, yw = quaternion.y*quaternion.w;

		matrix(0, 0) = 1 - 2 * (y2 + z2);	matrix(0, 1) = 2 * (xy + wz);	matrix(0, 2) = 2 * (xz - yw);
		matrix(1, 0) = 2 * (xy - wz);		matrix(1, 1) = 1 - 2 * (x2 + z2); matrix(1, 2) = 2 * (yz + wx);
		matrix(2, 0) = 2 * (xz + yw);		matrix(2, 1) = 2 * (yz - wx);	matrix(2, 2) = 1 - 2 * (x2 + y2);
	}

	inline float4x4 QuaternionToMatrix(const float4 & quaternion)
	{
		float4x4 matrix;
		QuaternionToMatrix(quaternion, matrix);
		matrix(0, 3) = 0; matrix(1, 3) = 0; matrix(2, 3) = 0;
		matrix(3, 0) = 0; matrix(3, 1) = 0; matrix(3, 2) = 0;
		matrix(3, 3) = 1;
		return matrix;
	}

	//This function only uses the upper 3x3 portion of the float4x4. Note if the input matrix contains scales, shears,
	//or other non-rotation transformations in the upper 3x3 matrix, then the output of this function is ill-defined.
	inline float4 MatrixToQuaternion(const float4x4& M)
	{
		float4 q;
		float r22 = M(2, 2);//matrix(2, 2) = 1 - 2 * (y2 + z2);
		if (r22 <= 0.f)  // x^2 + y^2 >= z^2 + w^2
		{
			float dif10 = M(1, 1) - M(0, 0);//2(y2-x2)
			float omr22 = 1.f - r22;//2(x2+y2)
			if (dif10 <= 0.f)  // x^2 >= y^2
			{
				float fourXSqr = omr22 - dif10;//4x2
				float inv4x = 0.5f / sqrtf(fourXSqr);//1/4x
				q.x = fourXSqr*inv4x;
				q.y = (M(0, 1) + M(1, 0))*inv4x;
				q.z = (M(0, 2) + M(2, 0))*inv4x;
				q.w = (M(1, 2) - M(2, 1))*inv4x;
			}
			else  // y^2 >= x^2
			{
				float fourYSqr = omr22 + dif10;
				float inv4y = 0.5f / sqrtf(fourYSqr);
				q.x = (M(0, 1) + M(1, 0))*inv4y;
				q.y = fourYSqr*inv4y;
				q.z = (M(1, 2) + M(2, 1))*inv4y;
				q.w = (M(2, 0) - M(0, 2))*inv4y;
			}
		}
		else  // z^2 + w^2 >= x^2 + y^2
		{
			float sum10 = M(1, 1) + M(0, 0);
			float opr22 = 1.f + r22;
			if (sum10 <= 0.f)  // z^2 >= w^2
			{
				float fourZSqr = opr22 - sum10;
				float inv4z = 0.5f / sqrtf(fourZSqr);
				q.x = (M(0, 2) + M(2, 0))*inv4z;
				q.y = (M(1, 2) + M(2, 1))*inv4z;
				q.z = fourZSqr*inv4z;
				q.w = (M(0, 1) - M(1, 0))*inv4z;
			}
			else  // w^2 >= z^2
			{
				float fourWSqr = opr22 + sum10;
				float inv4w = 0.5f / sqrtf(fourWSqr);
				q.x = (M(1, 2) - M(2, 1))*inv4w;
				q.y = (M(2, 0) - M(0, 2))*inv4w;
				q.z = (M(0, 1) - M(1, 0))*inv4w;
				q.w = fourWSqr*inv4w;
			}
		}
		return q;
	}

	

	inline float3 CartesianToBarycentric(float x, float y, const float2& pos1, const float2& pos2, const float2& pos3)
	{
		//(a,b) = (pos2 - pos1)
		//float a = pos2.x - pos1.x; float b = pos2.y - pos1.y;
		//(c,d) = (pos3-pos1)
		//float c = pos3.x - pos1.x; float d = pos3.y - pos1.y;
		//s = a*d - b*c;
		float s = (pos2.x - pos1.x)*(pos3.y - pos1.y) - (pos2.y - pos1.y)*(pos3.x - pos1.x);
		//pos2 - pos1 :: 1,pos3 - pos2 : 2
		//r3 : (a,b) = (pos2 - pos1) ,(c,d) = ((x,y) - pos1)
		float r3 = ((pos2.x - pos1.x)*(x - pos1.y) - (pos2.y - pos1.y)*(y - pos1.x)) / s;
		//r2 : (a,b) = (x,y) -pos1 ,(c,d) = (pos3-pos1)
		float r2 = ((x - pos1.x)*(pos3.y - pos1.y) - (y - pos1.y)*(pos3.x - pos1.x)) / s;
		float r1 = 1.f - r3 - r2;
		return float3(r1, r2, r3);
	}

	inline float3 CartesianToBarycentric(const float2& p, const float2& pos1, const float2& pos2, const float2& pos3)
	{
		return CartesianToBarycentric(p.x, p.y, pos1, pos2, pos3);
	}

	inline float2 BarycentricToCartesian(const float3& r, const float2& pos1, const float2& pos2, const float2& pos3)
	{
		float x = r.x * pos1.x + r.y * pos2.x + r.z * pos3.x;
		float y = r.x * pos1.y + r.y * pos2.y + r.z * pos3.y;

		return float2(x, y);
	}

	inline XMVECTOR XM_CALLCONV  BarycentricToCartesian(const float3& r, FXMVECTOR pos1, FXMVECTOR pos2, FXMVECTOR pos3)
	{
		XMVECTOR rvec = XMVectorScale(pos1, r.x);
		rvec += XMVectorScale(pos2, r.y);
		rvec += XMVectorScale(pos3, r.z);
		return rvec;
	}

	//barycentric coordinate
	inline  bool PointInTriangle(const float3& r, float epsilon)
	{
		float minr = 0.f - epsilon;
		float maxr = 1.f + epsilon;
		if (r.x < minr || r.x>maxr)
			return false;
		if (r.y < minr || r.y>maxr)
			return false;
		if (r.z < minr || r.z>maxr)
			return false;
		float rsum = r.x + r.y + r.z;
		return rsum >= minr && rsum <= maxr;
	}

	

}

#endif