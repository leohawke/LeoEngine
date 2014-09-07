// CopyRight 2014. LeoHawke. All wrongs reserved.

#ifndef Core_Vertex_Hpp
#define Core_Vertex_Hpp

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif
#include "..\IndePlatform\memory.hpp"
#include "..\leomath.hpp"
#include <type_traits>

struct D3D11_INPUT_ELEMENT_DESC;

namespace leo
{
	namespace Coord
	{
		inline float2 NDCtoUV(float x, float y)
		{
			return float2((x + 1.f) / 2.f, 1 - (y + 1.f) / 2.f);
		}
		inline float2 NDCtoUV(const float2& pos)
		{
			return float2((pos.x + 1.f) / 2.f, 1 - (pos.y + 1.f) / 2.f);
		}
	}

	namespace Vertex
	{
		struct Basic
		{
			float3 pos;
			float3 normal;
			float2 tex;
		};
		struct BasicV
		{
			BasicV operator=(const Basic& basic)
			{
				pos = loadfloat3(&basic.pos);
				normal = loadfloat3(&basic.normal);
				tex = loadfloat2(&basic.tex);
				return *this;
			}
			XMVECTOR pos;
			XMVECTOR normal;
			XMVECTOR tex;
		};

		struct NormalMap
		{
			float3 pos;
			float3 normal;
			float2 tex;
			float3 tangent;

			NormalMap(float x,float y,float z,float i,float j,float k,float u,float v,float ii,float ij,float ik)
				:pos(x, y, z), normal(i, j, k), tex(u, v), tangent(ii,ij,ik)
			{}

			NormalMap(const float3& pos,const float3& nor,const float2& tex,const float3& tan)
				:pos(pos), normal(nor), tex(tex), tangent(tan)
			{}

			NormalMap() = default;

			NormalMap(const float3& pos)
				:pos(pos)
			{}

			template<typename OtherVertexType>
			void operator=(const OtherVertexType& lvalue);
		};
		struct NormalMapV
		{
			NormalMapV operator=(const NormalMap& NormalMap)
			{
				pos = loadfloat3(&NormalMap.pos);
				normal = loadfloat3(&NormalMap.normal);
				tex = loadfloat2(&NormalMap.tex);
				tangent = loadfloat3(&NormalMap.tangent);
				return *this;
			}
			XMVECTOR pos;
			XMVECTOR normal;
			XMVECTOR tex;
			XMVECTOR tangent;
		};

		struct PostEffect
		{
			//µÑ¿¨¶û×ø±êÏµ Ó³ÉäÖÁ UV×ø±ê
			PostEffect(const float4& pos)
				:pos(pos), tex(Coord::NDCtoUV(pos.x,pos.y))
			{}
			float4 pos;
			float2 tex;
		};

		template<typename T>
		struct is_vertex : std::false_type
		{};

		template<typename T>
		struct is_ssevertex : std::false_type
		{};

		template<>
		struct is_vertex<Basic> : std::true_type
		{};

		template<>
		struct is_vertex<BasicV> : std::true_type
		{};

		template<>
		struct is_vertex<NormalMap> : std::true_type
		{};

		template<>
		struct is_vertex<NormalMapV> : std::true_type
		{};

		template<>
		struct is_ssevertex<BasicV> : std::true_type
		{};

		template<>
		struct is_ssevertex<NormalMapV> : std::true_type
		{};
	}
	namespace InputLayoutDesc
	{
		extern const D3D11_INPUT_ELEMENT_DESC Basic[3];

		extern const D3D11_INPUT_ELEMENT_DESC BasicV[3];

		extern const D3D11_INPUT_ELEMENT_DESC NormalMap[4];

		extern const D3D11_INPUT_ELEMENT_DESC NormalMapV[4];

		extern const D3D11_INPUT_ELEMENT_DESC BillBoard[2];

		extern const D3D11_INPUT_ELEMENT_DESC Sky[1];

		extern const D3D11_INPUT_ELEMENT_DESC PostEffect[2];
	}
}
#endif