#include "ray.hpp"
#include <DirectXMath.h>
#include <DirectXCollision.h>
int main(){
	using leo::float3;
	using leo::float4;
	using leo::load;
	leo::Ray ray(float3(1.f,0.f,0.f),float4(0.f,0.f,1.f,0.f));

	auto result = ray.Intersect(load(float3(0.f, 0.f, 0.f)), load(float3(2.f, 0.f, 0.f)), load(float3(1.f, 0.f, 1.f)));

	float dist = 0.f;
	auto dresult =	DirectX::TriangleTests::Intersects(load(float3(1.f, 0.f, 0.f)), load(float4(0.f, 0.f, 1.f, 0.f)), load(float3(0.f, 0.f, 0.f)), load(float3(2.f, 0.f, 0.f)), load(float3(1.f, 0.f, 1.f)), dist);

	assert(dresult == result.first);

	return 0;
}