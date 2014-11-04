#include "..\Core\MeshLoad.hpp"
int main(){
	leo::XMFLOAT4X4A offset(1.331581E-06, 1, 4.16881E-11, 0,
		-5.551115E-17, -4.16881E-11, 1, 0,
		1, -1.331581E-06, 8.412415E-23, 0,
		-2.230549, -0.03756041, -37.46099, 1);

	auto toInv = leo::XMLoadFloat4x4A(&offset);

	auto toRoot = leo::XMMatrixAffineTransformation(leo::XMVectorSplatOne(), leo::XMVectorZero(),
		leo::XMVectorSet(-0.5000003, -0.4999996, -0.4999996, 0.5000004), leo::XMVectorSet(0.03756343, 37.46099, 2.230549, 1.f));

	auto OthertoRoot = leo::XMMatrixRotationQuaternion(leo::XMVectorSet(-0.5000003, -0.4999996, -0.4999996, 0.5000004));
	OthertoRoot = leo::XMMatrixMultiply(OthertoRoot, leo::XMMatrixTranslation(0.03756343, 37.46099, 2.230549));

	auto Skin = leo::XMMatrixMultiply(toInv, toRoot);
	return 0;
}