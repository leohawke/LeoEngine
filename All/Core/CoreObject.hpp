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
#include "..\leomath.hpp"
#include "..\IndePlatform\memory.hpp"
namespace leo
{
	typedef  DataAllocatedObject<GeneralAllocPolicy> SQTAllocated;
	class SQTObject :public SQT, public SQTAllocated
	{
	public:
		SQTObject() = default;

		SQTObject(const SQT& sqt)
			:SQT(sqt)
		{}
		~SQTObject() = default;

		using SQT::operator=;
	public:
		void inline Scale(float s)
		{
			this->s *= s;
		}

		void inline Rotation(const float4& quaternion)
		{
			auto o = XMMatrixRotationQuaternion(load(q));
			auto n = XMMatrixRotationQuaternion(load(quaternion));
			save(q, XMQuaternionRotationMatrix(o*n));
		}

		void inline Translation(const float3& offset)
		{
			auto vt = load(this->t);
			auto off = load(offset);
			save(t, vt + off);
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
	};


}

#endif