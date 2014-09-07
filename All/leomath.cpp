#include "leomath.hpp"

namespace leo
{
	/*	i = angle(x)/2 j = angle(y)/2 k = angle(z)/2
		 *	
		|x|	  |sin(i)*cos(j)*cos(k)+cos(i)*sin(j)*sin(k)|
	q = |y| = |cos(i)*sin(j)*cos(k)+sin(i)*cos(j)*sin(k)|
		|z|	  |cos(i)*cos(j)*sin(k)-sin(i)*sin(j)*cos(k)|
		|w|	  |cos(i)*cos(j)*cos(k)+sin(i)*sin(j)*sin(k)|
		 *
	ref:
		http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
		http://www.cnblogs.com/wqj1212/archive/2010/11/21/1883033.html
	*/
	SQT::SQT(const SeqSQT& lvalue)
		:s(lvalue.s), t(lvalue.t)
	{
		q = EulerAngleToQuaternion(lvalue.q);
	}

	SQT::SQT()
	{
		std::memset(this, 0, sizeof(SQT));
		s = 1.f;
	}

	void SQT::operator=(const SeqSQT& lvalue)
	{
		SQT::SQT(lvalue);
	}

	SeqSQT::SeqSQT()
	{
		std::memset(this, 0, sizeof(SeqSQT));
		s = 1.f;
	}

	SeqSQT::SeqSQT(const SQT& lvalue)
		:s(lvalue.s), t(lvalue.t)
	{
		q = QuaternionToEulerAngle(lvalue.q);
	}

	void SeqSQT::operator=(const SQT& lvalue)
	{
		SeqSQT::SeqSQT(lvalue);
	}

	SQT::operator float4x4() const
	{
		auto result = QuaternionToMatrix(q);

		result._11 *= s;
		result._22 *= s;
		result._33 *= s;
		result._41 = t.x;
		result._42 = t.y;
		result._43 = t.z;

		


		return result;
	}

	SQT::operator XMMATRIX() const
	{
		auto result = operator leo::float4x4();
		return loadfloat4x4(&result);
	}
}