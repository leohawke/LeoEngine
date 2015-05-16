#include "RenderSystem\d3dx11.hpp"
#include <cstddef>
#include "Vertex.hpp"

namespace leo
{
	namespace InputLayoutDesc
	{
		extern const D3D11_INPUT_ELEMENT_DESC Basic[3]
			=
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,loffsetof(Vertex::Basic,pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,loffsetof(Vertex::Basic,normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, loffsetof(Vertex::Basic,tex), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		extern const D3D11_INPUT_ELEMENT_DESC BasicV[3]
			=
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		extern const D3D11_INPUT_ELEMENT_DESC NormalMap[4]
			=
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, loffsetof(Vertex::NormalMap, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, loffsetof(Vertex::NormalMap, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, loffsetof(Vertex::NormalMap, tex), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, loffsetof(Vertex::NormalMap, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		extern const D3D11_INPUT_ELEMENT_DESC NormalMapV[4]
			=
		{
			BasicV[0], BasicV[1], BasicV[2],
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(Vertex::BasicV), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		extern const D3D11_INPUT_ELEMENT_DESC BillBoard[2]
			=
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float3), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		extern const D3D11_INPUT_ELEMENT_DESC Sky[1] =
		{
			{ "POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 }
		};

		extern const D3D11_INPUT_ELEMENT_DESC PostEffect[2] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float3), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		extern const D3D11_INPUT_ELEMENT_DESC Skinned[6] =
		{
			NormalMap[0], NormalMap[1], NormalMap[2], NormalMap[3],
			{ "JOINTINDICES", 0, DXGI_FORMAT_R32_UINT, 1, loffsetof(Vertex::SkeAdjInfo,mIndices), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "JOINTWEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, loffsetof(Vertex::SkeAdjInfo, mWeights),
			 D3D11_INPUT_PER_VERTEX_DATA,0}
		};
	}

	namespace helper
	{
		MeshData Subdivide(const MeshData& result);

		/*
		void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& result);
		void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& result);
		*/
		MeshData CreateBox(float width, float height, float depth)
		{
			using Vertex::NormalMap;
			std::vector<NormalMap> v;
			v.reserve(24);

			float w2 = width * 0.5f;
			float h2 = height * 0.5f;
			float d2 = depth * 0.5f;

			// Fill in the front face vertex data.
			v.emplace_back(float3(-w2, -h2, -d2), float3(0.0f, 0.0f, -1.0f), float2(0.0f, 1.0f), float3(1.0f, 0.0f, 0.f));
			v.emplace_back(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
			v.emplace_back(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f);
			v.emplace_back(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);

			// Fill in the back face vertex data.
			v.emplace_back(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f);
			v.emplace_back(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f);
			v.emplace_back(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
			v.emplace_back(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f);

			// Fill in the top face vertex data.
			v.emplace_back(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);
			v.emplace_back(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
			v.emplace_back(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f);
			v.emplace_back(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);

			// Fill in the bottom face vertex data.
			v.emplace_back(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f);
			v.emplace_back(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f);
			v.emplace_back(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
			v.emplace_back(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f);

			// Fill in the left face vertex data.
			v.emplace_back(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f);
			v.emplace_back(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
			v.emplace_back(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f);
			v.emplace_back(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f);

			// Fill in the right face vertex data.
			v.emplace_back(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
			v.emplace_back(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
			v.emplace_back(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
			v.emplace_back(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);

			std::vector<uint32> i(36);

			// Fill in the front face index data
			i[0] = 0; i[1] = 1; i[2] = 2;
			i[3] = 0; i[4] = 2; i[5] = 3;

			// Fill in the back face index data
			i[6] = 4; i[7] = 5; i[8] = 6;
			i[9] = 4; i[10] = 6; i[11] = 7;

			// Fill in the top face index data
			i[12] = 8; i[13] = 9; i[14] = 10;
			i[15] = 8; i[16] = 10; i[17] = 11;

			// Fill in the bottom face index data
			i[18] = 12; i[19] = 13; i[20] = 14;
			i[21] = 12; i[22] = 14; i[23] = 15;

			// Fill in the left face index data
			i[24] = 16; i[25] = 17; i[26] = 18;
			i[27] = 16; i[28] = 18; i[29] = 19;

			// Fill in the right face index data
			i[30] = 20; i[31] = 21; i[32] = 22;
			i[33] = 20; i[34] = 22; i[35] = 23;

			return{ std::move(v), std::move(i) };
		}

		MeshData CreateSphere(float radius, uint32 sliceCount, uint32 stackCount)
		{
			using Vertex::NormalMap;
			//
			// Compute the vertices stating at the top pole and moving down the stacks.
			//

			// Poles: note that there will be texture coordinate distortion as there is
			// not a unique point on the texture map to assign to the pole when mapping
			// a rectangular texture onto a sphere.
			NormalMap topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
			NormalMap bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);

			std::vector<NormalMap> Vertices;
			Vertices.reserve(1 + (stackCount - 1)*(sliceCount + 1) + 1);//top + slice + bottom
			Vertices.emplace_back(topVertex);

			float phiStep = LM_PI / stackCount;
			float thetaStep = 2.0f*LM_PI / sliceCount;

			// Compute vertices for each stack ring (do not count the poles as rings).
			for (uint32 i = 1; i <= stackCount - 1; ++i)
			{
				float phi = i*phiStep;

				// Vertices of ring.
				for (uint32 j = 0; j <= sliceCount; ++j)
				{
					float theta = j*thetaStep;

					float3 pos;

					// spherical to cartesian
					pos.x = radius*sinf(phi)*cosf(theta);
					pos.y = radius*cosf(phi);
					pos.z = radius*sinf(phi)*sinf(theta);

					// Partial derivative of P with respect to theta
					float3 tangent;
					tangent.x = -radius*sinf(phi)*sinf(theta);
					tangent.y = 0.0f;
					tangent.z = +radius*sinf(phi)*cosf(theta);

					auto T = load(tangent);
					save(tangent, Normalize<3>(T));

					auto p = load(pos);
					float3 normal;
					save(normal, Normalize<3>(p));

					float2 tex;
					tex.x = theta / LM_TWOPI;
					tex.y = phi / LM_PI;

					Vertices.emplace_back(pos, normal, tex, tangent);
				}
			}

			Vertices.emplace_back(bottomVertex);

			std::vector<uint32> Indices;
			Indices.reserve(sliceCount * 3 + (stackCount - 2)*sliceCount * 3 + sliceCount * 3);//top + slice + bottom
			//
			// Compute indices for top stack.  The top stack was written first to the vertex buffer
			// and connects the top pole to the first ring.
			//
			for (uint32 i = 1; i <= sliceCount; ++i)
			{
				Indices.push_back(0);
				Indices.push_back(i + 1);
				Indices.push_back(i);
			}

			//
			// Compute indices for inner stacks (not connected to poles).
			//

			// Offset the indices to the index of the first vertex in the first ring.
			// This is just skipping the top pole vertex.
			uint32 baseIndex = 1;
			uint32 ringVertexCount = sliceCount + 1;
			for (uint32 i = 0; i < stackCount - 2; ++i)
			{
				for (uint32 j = 0; j < sliceCount; ++j)
				{
					Indices.push_back(baseIndex + i*ringVertexCount + j);
					Indices.push_back(baseIndex + i*ringVertexCount + j + 1);
					Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

					Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
					Indices.push_back(baseIndex + i*ringVertexCount + j + 1);
					Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
				}
			}

			//
			// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
			// and connects the bottom pole to the bottom ring.
			//

			// South pole vertex was added last.
			uint32 southPoleIndex = (uint32)Vertices.size() - 1;

			// Offset the indices to the index of the first vertex in the last ring.
			baseIndex = southPoleIndex - ringVertexCount;

			for (uint32 i = 0; i < sliceCount; ++i)
			{
				Indices.push_back(southPoleIndex);
				Indices.push_back(baseIndex + i);
				Indices.push_back(baseIndex + i + 1);
			}


			return MeshData{ std::move(Vertices), std::move(Indices) };

			
		}

		MeshData Subdivide(const MeshData& result)
		{
			using Vertex::NormalMap;

			std::vector<NormalMap> Vertices;
			Vertices.reserve(result.Vertices.size() * 2);
			std::vector<uint32> Indices;
			Indices.reserve(result.Indices.size() * 4);

			//       v1
			//       *
			//      / \
							//     /   \
							//  m0*-----*m1
			//   / \   / \
							//  /   \ /   \
							// *-----*-----*
			// v0    m2     v2

			auto numTris = static_cast<uint32>(result.Indices.size() / 3);
			for (uint32 i = 0; i < numTris; ++i)
			{
				NormalMap v0 = result.Vertices[result.Indices[i * 3 + 0]];
				NormalMap v1 = result.Vertices[result.Indices[i * 3 + 1]];
				NormalMap v2 = result.Vertices[result.Indices[i * 3 + 2]];

				//
				// Generate the midpoints.
				//

				NormalMap m0, m1, m2;

				// For subdivision, we just care about the position component.  We derive the other
				// vertex components in CreateGeosphere.

				m0.pos = float3(
					0.5f*(v0.pos.x + v1.pos.x),
					0.5f*(v0.pos.y + v1.pos.y),
					0.5f*(v0.pos.z + v1.pos.z));

				m1.pos = float3(
					0.5f*(v1.pos.x + v2.pos.x),
					0.5f*(v1.pos.y + v2.pos.y),
					0.5f*(v1.pos.z + v2.pos.z));

				m2.pos = float3(
					0.5f*(v0.pos.x + v2.pos.x),
					0.5f*(v0.pos.y + v2.pos.y),
					0.5f*(v0.pos.z + v2.pos.z));

				//
				// Add new geometry.
				//

				Vertices.push_back(v0); // 0
				Vertices.push_back(v1); // 1
				Vertices.push_back(v2); // 2
				Vertices.push_back(m0); // 3
				Vertices.push_back(m1); // 4
				Vertices.push_back(m2); // 5

				Indices.push_back(i * 6 + 0);
				Indices.push_back(i * 6 + 3);
				Indices.push_back(i * 6 + 5);

				Indices.push_back(i * 6 + 3);
				Indices.push_back(i * 6 + 4);
				Indices.push_back(i * 6 + 5);

				Indices.push_back(i * 6 + 5);
				Indices.push_back(i * 6 + 4);
				Indices.push_back(i * 6 + 2);

				Indices.push_back(i * 6 + 3);
				Indices.push_back(i * 6 + 1);
				Indices.push_back(i * 6 + 4);
			}

			return{ std::move(Vertices), std::move(Indices) };
		}

		MeshData CreateGeosphere(float radius, uint32 numSubdivisions)
		{
			// Put a cap on the number of subdivisions.
			numSubdivisions = leo::min(numSubdivisions, 5u);

			// Approximate a sphere by tessellating an icosahedron.

			const float X = 0.525731f;
			const float Z = 0.850651f;

			float3 pos[12] =
			{
				float3(-X, 0.0f, Z), float3(X, 0.0f, Z),
				float3(-X, 0.0f, -Z), float3(X, 0.0f, -Z),
				float3(0.0f, Z, X), float3(0.0f, Z, -X),
				float3(0.0f, -Z, X), float3(0.0f, -Z, -X),
				float3(Z, X, 0.0f), float3(-Z, X, 0.0f),
				float3(Z, -X, 0.0f), float3(-Z, -X, 0.0f)
			};
			uint32 k[60] =
			{
				1, 4, 0, 4, 9, 0, 4, 5, 9, 8, 5, 4, 1, 8, 4,
				1, 10, 8, 10, 3, 8, 8, 3, 5, 3, 2, 5, 3, 7, 2,
				3, 10, 7, 10, 6, 7, 6, 11, 7, 6, 0, 11, 6, 1, 0,
				10, 1, 6, 11, 0, 9, 2, 11, 9, 5, 2, 9, 11, 2, 7
			};

			MeshData result;

			result.Vertices.reserve(12);
			for (const auto& p : pos)
				result.Vertices.emplace_back(p);
			result.Indices.reserve(60);
			result.Indices.assign(std::begin(k), std::end(k));//leo::begin,leo::end

			switch (numSubdivisions)
			{
			case 5:
				result = Subdivide(result);
			case 4:
				result = Subdivide(result);
			case 3:
				result = Subdivide(result);
			case 2:
				result = Subdivide(result);
			case 1:
				result = Subdivide(result);
			default:
				break;
			}

			// Project vertices onto sphere and scale.
			for (uint32 i = 0; i < result.Vertices.size(); ++i)
			{
				// Project onto unit sphere.
				auto n = Normalize<3>(load(result.Vertices[i].pos));

				// Project onto sphere.
				auto p =Multiply(n,radius);

				save(result.Vertices[i].pos, p);
				save(result.Vertices[i].normal, n);

				// Derive texture coordinates from spherical coordinates.
				auto theta = atanr(
					result.Vertices[i].pos.x,
					result.Vertices[i].pos.z);

				float phi = acosf(result.Vertices[i].pos.y / radius);

				result.Vertices[i].tex.x = theta / LM_TWOPI;
				result.Vertices[i].tex.y = phi / LM_PI;

				// Partial derivative of P with respect to theta
				result.Vertices[i].tangent.x = -radius*sinf(phi)*sinf(theta);
				result.Vertices[i].tangent.y = 0.0f;
				result.Vertices[i].tangent.z = +radius*sinf(phi)*cosf(theta);

				auto T = load(result.Vertices[i].tangent);
				save(result.Vertices[i].tangent, Normalize<3>(T));
			}

			return std::move(result);
		}

		MeshData CreateGrid(float width, float depth, uint32 m, uint32 n)
		{
			uint32 vertexCount = m*n;
			uint32 faceCount = (m - 1)*(n - 1) * 2;

			//
			// Create the vertices.
			//

			float halfWidth = 0.5f*width;
			float halfDepth = 0.5f*depth;

			float dx = width / (n - 1);
			float dz = depth / (m - 1);

			float du = 1.0f / (n - 1);
			float dv = 1.0f / (m - 1);

			std::vector<Vertex::NormalMap> Vertices(vertexCount);

			for (uint32 i = 0; i < m; ++i)
			{
				float z = halfDepth - i*dz;
				for (uint32 j = 0; j < n; ++j)
				{
					float x = -halfWidth + j*dx;

					Vertices[i*n + j].pos = float3(x, 0.0f, z);
					Vertices[i*n + j].normal = float3(0.0f, 1.0f, 0.0f);
					Vertices[i*n + j].tangent = float3(1.0f, 0.0f, 0.0f);

					// Stretch texture over grid.
					Vertices[i*n + j].tex.x = j*du;
					Vertices[i*n + j].tex.y = i*dv;
				}
			}

			//
			// Create the indices.
			//

			std::vector<uint32> Indices(faceCount * 3); // 3 indices per face

			// Iterate over each quad and compute indices.
			uint32 k = 0;
			for (uint32 i = 0; i < m - 1; ++i)
			{
				for (uint32 j = 0; j < n - 1; ++j)
				{
					Indices[k] = i*n + j;
					Indices[k + 1] = i*n + j + 1;
					Indices[k + 2] = (i + 1)*n + j;

					Indices[k + 3] = (i + 1)*n + j;
					Indices[k + 4] = i*n + j + 1;
					Indices[k + 5] = (i + 1)*n + j + 1;

					k += 6; // next quad
				}
			}
			return{ std::move(Vertices), std::move(Indices) };
		}

		std::array<Vertex::PostEffect, 4>& CreateFullscreenQuad()
		{
			using Vertex::PostEffect;
			 static std::array<Vertex::PostEffect, 4> result = {
				PostEffect(float4(+1.f, +1.f, 1.f, 1.f)),
				PostEffect(float4(+1.f, -1.f, 1.f, 1.f)),
				PostEffect(float4(-1.f, +1.f, 1.f, 1.f)),
				PostEffect(float4(-1.f, -1.f, 1.f, 1.f))
			};
			return result;
		}
	}
}