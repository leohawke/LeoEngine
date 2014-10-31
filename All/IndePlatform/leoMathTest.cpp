#include "LeoMath.h"

#include "..\leomath.hpp"
int main(){

	leo::SQT sqt;

	auto r0 = leo::XMMatrixRotationY(1.f);

	

	r0 = leo::XMMatrixMultiply(r0, leo::XMMatrixScaling(2.f, 2.f, 2.f));

	auto q = leo::XMQuaternionRotationMatrix(r0);
	auto invq = leo::XMMatrixRotationQuaternion(q);
	auto invinvq = leo::XMQuaternionRotationMatrix(invq);
	leo::float4 checkq;
	leo::save(checkq, q);

	leo::float4x4 f4x4;
	leo::XMStoreFloat4x4A((leo::XMFLOAT4X4A*)&f4x4, r0);

	sqt = f4x4;

	auto check = leo::MatrixToQuaternion(f4x4);
	return 0;
}