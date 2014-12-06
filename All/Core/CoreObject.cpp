#include "CoreObject.hpp"

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
		:s(lvalue.s), t(lvalue.t), q(lvalue.q)
	{
	}

	SQT::SQT()
	{
		std::memset(this, 0, sizeof(SQT));
		s = 1.f;
	}

	void SQT::operator=(const SeqSQT& lvalue)
	{
		s = lvalue.s;
		q = lvalue.q;
		t = lvalue.t;
	}

	SeqSQT::SeqSQT()
	{
		std::memset(this, 0, sizeof(SeqSQT));
		s = 1.f;
	}

	SeqSQT::SeqSQT(const SQT& lvalue)
		:s(lvalue.s)
	{
		std::memcpy(&t, &lvalue.t, sizeof(float) * 3);
		auto rvalue = QuaternionToEulerAngle(lvalue.q);
		std::memcpy(&q, &rvalue, sizeof(float) * 4);
	}

	void SeqSQT::operator=(const SQT& lvalue)
	{
		SeqSQT::SeqSQT(lvalue);
	}

	SQT::operator float4x4() const
	{
		auto result = QuaternionToMatrix(q);

		result(0, 0) *= s;
		result(1, 1) *= s;
		result(2, 2) *= s;
		result(3, 0) = t.x;
		result(3, 1) = t.y;
		result(3, 2) = t.z;




		return result;
	}
}