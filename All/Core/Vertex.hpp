// CopyRight 2014. LeoHawke. All wrongs reserved.

#ifndef Core_Vertex_Hpp
#define Core_Vertex_Hpp

#include "memory.hpp"
#include "leomathutility.hpp"
#include <type_traits>
#include <vector>
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
				pos = load(basic.pos);
				normal = load(basic.normal);
				tex = load(basic.tex);
				return *this;
			}
			__m128 pos;
			__m128 normal;
			__m128 tex;
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

			void operator=(const NormalMap& lvalue) {
				memcpy(*this, lvalue);
			}
		};
		struct NormalMapV
		{
			NormalMapV operator=(const NormalMap& NormalMap)
			{
				pos = load(NormalMap.pos);
				normal = load(NormalMap.normal);
				tex = load(NormalMap.tex);
				tangent = load(NormalMap.tangent);
				return *this;
			}
			__m128 pos;
			__m128 normal;
			__m128 tex;
			__m128 tangent;
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

		struct SkeAdjInfo{
			SkeAdjInfo(uint32 indices,const float3& weights)
				:mIndices(indices), mWeights(weights)
			{}
			uint32 mIndices;
			float3 mWeights;
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

		extern const D3D11_INPUT_ELEMENT_DESC Skinned[6];

		extern const D3D11_INPUT_ELEMENT_DESC Terrain[1];
	}


	namespace helper
	{
		struct MeshData
		{
			std::vector < Vertex::NormalMap> Vertices;
			std::vector<uint32> Indices;

			MeshData(std::vector < Vertex::NormalMap> && v, std::vector<uint32> && i)
				:Vertices(std::move(v)), Indices(std::move(i)) {
			}

			MeshData() {
			}
		};

		struct SimpleMeshData
		{
			std::vector <float3,leo::aligned_alloc<float3,16>> Vertices;
			std::vector<uint32> Indices;

			SimpleMeshData(std::vector < float3, leo::aligned_alloc<float3, 16>> && v, std::vector<uint32> && i)
				:Vertices(std::move(v)), Indices(std::move(i)) {
			}

			SimpleMeshData() {
			}
		};

		///<summary>
		/// Creates a box centered at the origin with the given dimensions.
		///</summary>
		MeshData CreateBox(float width, float height, float depth);

		///<summary>
		/// Creates a sphere centered at the origin with the given radius.  The
		/// slices and stacks parameters control the degree of tessellation.
		///</summary>
		MeshData CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);
		SimpleMeshData CreateSphere(uint32 sliceCount, uint32 stackCount);

		///<summary>
		/// Creates a geosphere centered at the origin with the given radius.  The
		/// depth controls the level of tessellation.
		///</summary>
		MeshData CreateGeosphere(float radius, uint32 numSubdivisions);


		///<summary>
		/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
		/// The bottom and top radius can vary to form various cone shapes rather than true
		// cylinders.  The slices and stacks parameters control the degree of tessellation.
		///</summary>
		//MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);

		///<summary>
		/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
		/// at the origin with the specified width and depth.
		///</summary>
		MeshData CreateGrid(float width, float depth, uint32 m, uint32 n);

		///<summary>
		/// Creates a quad covering the screen in NDC coordinates.  This is useful for
		/// postprocessing effects.
		///IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP)
		///</summary>
		std::array<Vertex::PostEffect, 4>& CreateFullscreenQuad();

	}
}
#endif