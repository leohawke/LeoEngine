#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <functional>

namespace DirectX
{
	// 3D Vector; 32 bit floating point components
	struct XMFLOAT3
	{
		float x;
		float y;
		float z;

		XMFLOAT3() {}
		XMFLOAT3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
		explicit XMFLOAT3(const float *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]) {}

		XMFLOAT3& operator= (const XMFLOAT3& Float3) { x = Float3.x; y = Float3.y; z = Float3.z; return *this; }

		bool operator==(const XMFLOAT3& float3) const
		{
			return (x == float3.x) && (y == float3.y) && (z == float3.z);
		}

		bool operator!=(const XMFLOAT3& float3) const
		{
			return !(*this == float3);
		}
	};

	// 2D Vector; 32 bit floating point components
	struct XMFLOAT2
	{
		float x ;
		float y ;

		XMFLOAT2() {}
		XMFLOAT2(float _x, float _y) : x(_x), y(_y) {}
		explicit XMFLOAT2(const float *pArray) : x(pArray[0]), y(pArray[1]) {}

		XMFLOAT2& operator= (const XMFLOAT2& Float2) { x = Float2.x; y = Float2.y; return *this; }

		bool operator==(const XMFLOAT2& float3) const
		{
			return (x == float3.x) && (y == float3.y);
		}
	};
}

struct MeshFileHeader
{
	std::uint8_t numsubset;
	std::uint32_t numvertice;
	std::uint32_t numindex;

	MeshFileHeader()
	{
		std::memset(this, 0, sizeof(MeshFileHeader));
	}
};
struct MeshMaterial
{
	DirectX::XMFLOAT3 ambient;
	DirectX::XMFLOAT3 diffuse;
	DirectX::XMFLOAT3 specular;
	DirectX::XMFLOAT3 reflect;
	float specPow;
	float alphaclip;
	wchar_t diffusefile[260];
	wchar_t normalmapfile[260];

	MeshMaterial()
	{
		std::memset(this, 0, sizeof(MeshMaterial));
		
	}

	struct Ctor
	{};
	MeshMaterial(const Ctor&)
	{
		std::memset(this, 0, sizeof(MeshMaterial));
		using namespace DirectX;
		ambient = XMFLOAT3(0.3f, 0.3f, 0.3f);
		diffuse = XMFLOAT3(0.4f, 0.6f, 0.4f);
		reflect = XMFLOAT3(0.2f, 0.4f, 0.2f);
		specular = XMFLOAT3(0.1f, 0.1f, 0.1f);
		specPow = 2;
		std::memcpy(normalmapfile, L"DefaultNormalMap.dds", sizeof(L"DefaultNormalMap.dds"));
		std::memcpy(diffusefile, L"DefaultDiffuse.dds", sizeof(L"DefaultDiffuse.dds"));
	}
	bool operator==(const MeshMaterial& mat) const
	{
		return (ambient == mat.ambient) &&
			(diffuse == mat.diffuse) &&
			(specular == mat.specular) &&
			(reflect == mat.reflect) &&
			(specPow == mat.specPow) &&
			(alphaclip == mat.alphaclip) &&
			!(wcscmp(diffusefile, mat.diffusefile)) &&
			!(wcscmp(normalmapfile,mat.normalmapfile));
	}
};
struct MeshSubSet
{
	std::uint32_t indexoffset;
	std::uint32_t indexcount;

	MeshSubSet()
	{
		std::memset(this, 0, sizeof(MeshSubSet));
	}

	MeshSubSet(std::uint32_t offset,std::uint32_t count)
		:indexoffset(offset), indexcount(count)
	{}
};
struct MeshVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 tex;
	DirectX::XMFLOAT3 tangent;

	MeshVertex()
	{
		std::memset(this, 0, sizeof(MeshVertex));
	}
};