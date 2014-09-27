#ifndef leomath_hpp
#define leomath_hpp
#include <algorithm>
#include <cmath>
#pragma warning(push)
#pragma warning(disable : 4838)
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
#if defined LEO_PROFILE_MATH
	using float2 = XMFLOAT2A;
	using float3 = XMFLOAT3A;
	using float4 = XMFLOAT4A;
	using float4x4 = XMFLOAT4X4A;

#define loadfloat2 XMLoadFloat2A
#define savefloat2 XMStoreFloat2A
#define loadfloat3 XMLoadFloat3A
#define savefloat3 XMStoreFloat3A
#define loadfloat4 XMLoadFloat4A
#define savefloat4 XMStoreFloat4A
#define loadfloat4x4 XMLoadFloat4x4A
#define savefloat4x4 XMStoreFloat4x4A

#else
	using float2 = XMFLOAT2;
	using float3 = XMFLOAT3;
	using float4 = XMFLOAT4;
	using float4x4 = XMFLOAT4X4;
#define loadfloat2 XMLoadFloat2
#define savefloat2 XMStoreFloat2
#define loadfloat3 XMLoadFloat3
#define savefloat3 XMStoreFloat3
#define loadfloat4 XMLoadFloat4
#define savefloat4 XMStoreFloat4
#define loadfloat4x4 XMLoadFloat4x4
#define savefloat4x4 XMStoreFloat4x4
#endif

	template<typename T>
	struct pack_type : T
	{
	};

	template<>
	struct pack_type < XMFLOAT2 > : XMFLOAT2
	{
		XMFLOAT2 pad;
	};

	template<>
	struct pack_type < XMFLOAT3 > : XMFLOAT3
	{
		float pad;
	};

	using std::max;
	using std::min;
	const float LM_PI = 3.14159265f;
	const float LM_HALFPI = LM_PI / 2.0f;
	const float LM_QUARPI = LM_PI / 4.0f;
	//radian per degree
	const float LM_RPD = LM_PI / 180.0f;

	template<typename _Ty>
	inline void clamp(const _Ty& _Min, const _Ty& _Max,_Ty & _X)
	{
		_X = max(_Min, min(_Max, _X));
	}

	template<typename _Ty>
	//单位:角度
	inline _Ty sind(const _Ty& degree)
	{
		return std::sin(degree *LM_RPD);
	}

	template<typename _Ty>
	//单位:角度
	inline _Ty cosd(const _Ty& degree)
	{
		return std::cos(degree*LM_RPD);
	}

	template<typename _Ty>
	//单位:弧度
	inline _Ty sinr(const _Ty& radian)
	{
		return std::sin(radian);
	}

	template<typename _Ty>
	//单位:弧度
	inline _Ty cosr(const _Ty& radian)
	{
		return std::cos(radian);
	}

	template<typename _Ty>
	//单位:角度
	inline _Ty tand(const _Ty& degree)
	{
		return std::tan(degree *LM_RPD);
	}

	template<typename _Ty>
	//单位:弧度
	inline _Ty tanr(const _Ty& radian)
	{
		return std::tan(radian);
	}

	template<typename _Tx,typename _Ty>
	//单位:弧度
	inline auto atanr(const _Tx& x, const _Ty& y)->decltype(atan(y/x))
	{
		decltype(atan(y / x)) theta = 0.f;
		if (x >= 0.f)
		{
			theta = atan(y / x);

			if (theta < 0.f)
				theta += 2.0f*LM_PI;
		}
		else
			theta = atan(y / x) + LM_PI;
		return theta;
	}

#if defined WIN32 &&  defined PLATFORM_64BIT
	inline double sincosr(double *pcos, double rad)
	{
		*pcos = cosr(rad);
		return sinr(rad);
	}
#else
	//单位:弧度
	inline __declspec(naked) double __stdcall sincosr(double *pcos, double rad)
	{
		__asm
		{
			mov eax, dword ptr[esp+4]
			fld qword ptr[esp+8]
			fsincos
			fstp qword ptr[eax]
			ret 12
		}
	}
#endif
	//单位:角度
	inline double sincosd(double* pcos, double degree)
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

	template<>
	inline void clamp(const float2& min, const float2& max, float2 & val)
	{
		clamp(min.x, max.x, val.x);
		clamp(min.y, max.y, val.y);
	}
	

	template<>
	inline void clamp(const float3& min, const float3& max, float3 & val)
	{
		clamp(min.x, max.x, val.x);
		clamp(min.y, max.y, val.y);
		clamp(min.z, max.z, val.z);
	}

	template<>
	inline void clamp(const float4& min, const float4& max, float4 & val)
	{
		clamp(min.x, max.x, val.x);
		clamp(min.y, max.y, val.y);
		clamp(min.z, max.z, val.z);
		clamp(min.w, max.w, val.w);
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

	inline float Distance(const float3& a, const float3& b)
	{
		XMVECTOR x = XMLoadFloat3(&a);
		XMVECTOR y = XMLoadFloat3(&b);
		XMVECTOR length = XMVector3Length(XMVectorSubtract(x, y));
		return XMVectorGetX(length);
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

	inline float4 MatrixToQuaternion(const float4x4& M)
	{
		float4 q;
		float r22 = M.m[2][2];//matrix(2, 2) = 1 - 2 * (y2 + z2);
		if (r22 <= 0.f)  // x^2 + y^2 >= z^2 + w^2
		{
			float dif10 = M.m[1][1] - M.m[0][0];//2(y2-x2)
			float omr22 = 1.f - r22;//2(x2+y2)
			if (dif10 <= 0.f)  // x^2 >= y^2
			{
				float fourXSqr = omr22 - dif10;//4x2
				float inv4x = 0.5f / sqrtf(fourXSqr);//1/4x
				q.x = fourXSqr*inv4x;
				q.y = (M.m[0][1] + M.m[1][0])*inv4x;
				q.z = (M.m[0][2] + M.m[2][0])*inv4x;
				q.w = (M.m[1][2] - M.m[2][1])*inv4x;
			}
			else  // y^2 >= x^2
			{
				float fourYSqr = omr22 + dif10;
				float inv4y = 0.5f / sqrtf(fourYSqr);
				q.x = (M.m[0][1] + M.m[1][0])*inv4y;
				q.y = fourYSqr*inv4y;
				q.z = (M.m[1][2] + M.m[2][1])*inv4y;
				q.w = (M.m[2][0] - M.m[0][2])*inv4y;
			}
		}
		else  // z^2 + w^2 >= x^2 + y^2
		{
			float sum10 = M.m[1][1] + M.m[0][0];
			float opr22 = 1.f + r22;
			if (sum10 <= 0.f)  // z^2 >= w^2
			{
				float fourZSqr = opr22 - sum10;
				float inv4z = 0.5f / sqrtf(fourZSqr);
				q.x = (M.m[0][2] + M.m[2][0])*inv4z;
				q.y = (M.m[1][2] + M.m[2][1])*inv4z;
				q.z = fourZSqr*inv4z;
				q.w = (M.m[0][1] - M.m[1][0])*inv4z;
			}
			else  // w^2 >= z^2
			{
				float fourWSqr = opr22 + sum10;
				float inv4w = 0.5f / sqrtf(fourWSqr);
				q.x = (M.m[1][2] - M.m[2][1])*inv4w;
				q.y = (M.m[2][0] - M.m[0][2])*inv4w;
				q.z = (M.m[0][1] - M.m[1][0])*inv4w;
				q.w = fourWSqr*inv4w;
			}
		}
		return q;
	}

	inline float4 EulerAngleToQuaternion(const float3& eulerangle)
	{
		float4 q;
		double cos_i, sin_i, cos_j, sin_j, cos_k, sin_k;
		sin_i = sincosd(&cos_i, eulerangle.x / 2.f);
		sin_j = sincosd(&cos_j, eulerangle.y / 2.f);
		sin_k = sincosd(&cos_k, eulerangle.z / 2.f);
		q.x = static_cast<float>(sin_i*cos_j*cos_k + cos_i*sin_i*sin_k);
		q.y = static_cast<float>(cos_i*sin_j*cos_k - sin_i*cos_j*sin_k);
		q.z = static_cast<float>(cos_i*cos_j*sin_k - sin_i*sin_j*cos_k);
		q.w = static_cast<float>(cos_i*cos_j*cos_k + sin_i*sin_j*sin_k);
		return q;
	}

	inline float3 QuaternionToEulerAngle(const float4& quaternion)
	{
		float3 q;
		q.x = atan2(2 * (quaternion.w*quaternion.x + quaternion.y*quaternion.z), (1 - 2 * (quaternion.y*quaternion.y + quaternion.x*quaternion.x)));
		q.y = asin(2 * (quaternion.w*quaternion.y - quaternion.z*quaternion.x));
		q.z = atan2(2 * (quaternion.w*quaternion.z + quaternion.x*quaternion.y), (1 - 2 * (quaternion.z*quaternion.z + quaternion.y*quaternion.y)));
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
		float r2 = ((x - pos1.x)*(pos3.y - pos1.y) - (y-pos1.y)*(pos3.x - pos1.x)) / s;
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

	struct SQT;

	//file struct
	struct SeqSQT
	{
		SeqSQT();
		SeqSQT(const SQT& lvalue);
		void operator=(const SQT& lvalue);
		float3 q;//欧拉角
		float3 t;
		float s;
	};

	//memory struct
	struct SQT
	{
		SQT();
		SQT(const SeqSQT& lvalue);
		void operator=(const SeqSQT& lvalue);
		operator float4x4() const;
		operator XMMATRIX() const;
		float4 q;
		float3 t;
		float s;
	};

}

#endif