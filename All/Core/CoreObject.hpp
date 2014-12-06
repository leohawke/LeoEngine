////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/CoreObject.hpp
//  Version:     v1.00
//  Created:     8/15/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 核心库的基本对象
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef Core_CoreObject_HPP
#define Core_CoreObject_HPP
#include "..\IndePlatform\LeoMath.h"
#include "..\IndePlatform\memory.hpp"
namespace leo
{
	struct SQT;

	//file struct
	struct SeqSQT
	{
		SeqSQT();
		SeqSQT(const SQT& lvalue);
		void operator=(const SQT& lvalue);
		float q[4];
		float t[3];
		float s;
	};

	inline float4 EulerAngleToQuaternion(const float3& eulerangle)
	{
		float4 q;
		float cos_i, sin_i, cos_j, sin_j, cos_k, sin_k;
		sin_i = sincosr(&cos_i, eulerangle.x / 2.f);
		sin_j = sincosr(&cos_j, eulerangle.y / 2.f);
		sin_k = sincosr(&cos_k, eulerangle.z / 2.f);
		q.x = static_cast<float>(sin_i*cos_j*cos_k + cos_i*sin_i*sin_k);
		q.y = static_cast<float>(cos_i*sin_j*cos_k - sin_i*cos_j*sin_k);
		q.z = static_cast<float>(cos_i*cos_j*sin_k - sin_i*sin_j*cos_k);
		q.w = static_cast<float>(cos_i*cos_j*cos_k + sin_i*sin_j*sin_k);
		return q;
	}

	inline float3 QuaternionToEulerAngle(const float4& quaternion)
	{
		float3 q;
		q.x = ::atan2(2 * (quaternion.w*quaternion.x + quaternion.y*quaternion.z), (1 - 2 * (quaternion.y*quaternion.y + quaternion.x*quaternion.x)));
		q.y = asin(2 * (quaternion.w*quaternion.y - quaternion.z*quaternion.x));
		q.z = ::atan2(2 * (quaternion.w*quaternion.z + quaternion.x*quaternion.y), (1 - 2 * (quaternion.z*quaternion.z + quaternion.y*quaternion.y)));
		return q;
	}

	//memory struct
	//增加插值函数
	struct SQT
	{
		SQT();
		SQT(const SeqSQT& lvalue);
		void operator=(const SeqSQT& lvalue);

		operator float4x4() const;
		operator std::array<__m128, 4>() const {
			return load(operator float4x4());
		}
		float4 q;
		float3 t;
		float s;

		SQT(const float4& Q, const float3& T, float S)
			:q(Q), t(T), s(S) {
		}


	};

	inline SQT Lerp(const SQT& t1, const SQT& t2, float w) {
		auto s = t1.s*(1.f - w) + t2.s *w;

		auto T = Splat(w);

		auto t1T = load(t1.t);
		auto t2T = load(t2.t);
		float3 t;
		save(t, Lerp(t1T, t2T, T));

		auto t1Q = load(t1.q);
		auto t2Q = load(t2.q);
		float4 q;
		save(q, QuaternionSlerp(t1Q, t2Q, T));

		return SQT{ q, t, s };
	}


	class SQTObject :public SQT, public DataAllocatedObject<GeneralAllocPolicy>{
	public:
		SQTObject() = default;

		SQTObject(const SQT& sqt)
			:SQT(sqt)
		{}
		~SQTObject() = default;

		SQTObject& operator=(const SeqSQT& seq){
			q = seq.q;
			t = seq.t;
			s = seq.s;
			return *this;
		}

		SQTObject& operator=(const SQT& sqt){
			q = sqt.q;
			t = sqt.t;
			s = sqt.s;
			return *this;
		}
	public:
		void inline Scale(float S)
		{
			this->s *= S;
		}

		void inline Rotation(const float4& quaternion)
		{
			auto o = Matrix(load(q));
			auto n = Matrix(load(quaternion));
			save(q, Quaternion(Multiply(o,n)));
		}
		void inline Rotation(const float4x4& matrix) {
			auto R = load(matrix);
			auto Q = Matrix(load(q));
			save(q, Quaternion(Multiply(Q, R)));
		}
		void inline Rotation(const float3& Axis, float angle) {
			auto R = RotationAxis(load(Axis),angle);
			auto Q = Matrix(load(q));
			save(q, Quaternion(Multiply(Q, R)));
		}

		void inline Translation(const float3& offset)
		{
			auto vt = load(this->t);
			auto off = load(offset);
			save(t,Add( vt , off));
		}

		void inline Transform(const SQT& sqt)
		{
			Scale(sqt.s);
			Rotation(sqt.q);
			Translation(sqt.t);
		}

		void inline Roll(float x)
		{
			Rotation(EulerAngleToQuaternion(float3(x, 0.f, 0.f)));
		}

		void inline Pitch(float y)
		{
			Rotation(EulerAngleToQuaternion(float3(0.f,y, 0.f)));
		}

		void inline Yaw(float z)
		{
			Rotation(EulerAngleToQuaternion(float3(0.f, 0.f,z)));
		}

		inline const float3&  Pos() const {
			return t;
		}
	};

	class float4x4Object : public float4x4, public DataAllocatedObject<GeneralAllocPolicy> {
	public:
		float4x4Object() = default;

		float4x4Object(const float4x4& matrix)
			:float4x4(matrix)
		{}
		~float4x4Object() = default;
	};
}

#endif