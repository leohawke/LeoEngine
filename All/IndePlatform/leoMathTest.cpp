#include "LeoMath.h"

#include "..\leomath.hpp"
int main(){
	leo::float4 q0;
	leo::float4 q1;

	auto r0 = leo::XMMatrixRotationY(1.f);
	auto r1 = leo::XMMatrixRotationY(0.5f);

	leo::XMStoreFloat4A((leo::XMFLOAT4A*)(&q0),leo::XMQuaternionRotationMatrix(r0));
	leo::XMStoreFloat4A((leo::XMFLOAT4A*)(&q1), leo::XMQuaternionRotationMatrix(r1));

	auto drt = leo::XMQuaternionSlerp(load(q0), load(q1), 0.5f);
	leo::float4 dqt;
	save(dqt, drt);

	auto rt = leo::QuaternionSlerp(load(q0), load(q1), 0.5f);
	leo::float4 qt;
	save(qt, rt);

	qt.x;
	return 0;
}