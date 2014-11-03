#include "..\Core\MeshLoad.hpp"
int main(){

	leo::XMFLOAT4X4A inv(1.331581E-06, 1, 4.16881E-11, 0,
		-5.551115E-17, -4.16881E-11, 1, 0,
		1, -1.331581E-06, 8.412415E-23, 0, 
		-2.230549,-0.03756041, -37.46099, 1);

	auto mtoParent = leo::XMMatrixMultiply(leo::XMMatrixRotationQuaternion(leo::XMVectorSet(-0.45f, -0.54f, -0.48f, 0.51f)), leo::XMMatrixTranslation(0.037f, 37.4f, 2.23f));
	auto mtoRoot = leo::XMMatrixMultiply(leo::XMLoadFloat4x4A(&inv), mtoParent);

	leo::float4x4 finv(&inv._11);

	auto r = leo::QuaternionToMatrix(leo::float4(-0.45f, -0.54f, -0.48f, 0.51f));
	r.r[3] = leo::float4(0.037f, 37.4f, 2.23f, 1.f);
	auto toRoot = leo::Multiply(load(finv), load(r));
	return 0;
}