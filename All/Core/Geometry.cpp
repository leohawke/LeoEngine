#include "Geometry.hpp"

namespace leo
{
	///begin Box
	ContainmentType Box::Contains(const OrientedBox&  box) const
	{
		if (!box.Intersects(*this))
			return DISJOINT;

		XMVECTOR vCenter = XMLoadFloat3(&Center);
		XMVECTOR vExtents = XMLoadFloat3(&Extents);

		// Subtract off the AABB center to remove a subtract below
		XMVECTOR oCenter = XMLoadFloat3(&box.Center) - vCenter;

		XMVECTOR oExtents = XMLoadFloat3(&box.Extents);
		XMVECTOR oOrientation =load(box.mOrientation);

		assert(DirectX::Internal::XMQuaternionIsUnit(oOrientation));

		XMVECTOR Inside = XMVectorTrueInt();

		for (size_t i = 0; i < BoundingOrientedBox::CORNER_COUNT; ++i)
		{
			XMVECTOR C = XMVector3Rotate(oExtents * g_BoxOffset[i], oOrientation) + oCenter;
			XMVECTOR d = XMVector3LengthSq(C);
			Inside = XMVectorAndInt(Inside, XMVectorLessOrEqual(d, vExtents));
		}

		return (XMVector3EqualInt(Inside, XMVectorTrueInt())) ? CONTAINS : INTERSECTS;
	}
	bool Box::Intersects(const OrientedBox& box) const
	{
		return box.Intersects(*this);
	}
	///end Box

	///begin sphere
	ContainmentType Sphere::Contains(const OrientedBox& box) const
	{
		if (!box.Intersects(*this))
			return DISJOINT;

		XMVECTOR vCenter = XMLoadFloat3(&Center);
		XMVECTOR vRadius = XMVectorReplicatePtr(&Radius);
		XMVECTOR RadiusSq = vRadius * vRadius;

		XMVECTOR boxCenter = XMLoadFloat3(&box.Center);
		XMVECTOR boxExtents = XMLoadFloat3(&box.Extents);
		XMVECTOR boxOrientation =load(box.mOrientation);

		assert(DirectX::Internal::XMQuaternionIsUnit(boxOrientation));

		XMVECTOR InsideAll = XMVectorTrueInt();

		for (size_t i = 0; i < BoundingOrientedBox::CORNER_COUNT; ++i)
		{
			XMVECTOR C = XMVector3Rotate(boxExtents * g_BoxOffset[i], boxOrientation) + boxCenter;
			XMVECTOR d = XMVector3LengthSq(XMVectorSubtract(vCenter, C));
			InsideAll = XMVectorAndInt(InsideAll, XMVectorLessOrEqual(d, RadiusSq));
		}

		return (XMVector3EqualInt(InsideAll, XMVectorTrueInt())) ? CONTAINS : INTERSECTS;
	}
	bool Sphere::Intersects(const OrientedBox& box) const
	{
		return box.Intersects(*this);
	}

	void  Sphere::CreateFromBoundingBox(Sphere& Out, const OrientedBox& box)
	{
		BoundingSphere::CreateFromBoundingBox(Out, box);//impl by box
	}
	///end sphere

	ContainmentType OrientedBox::Contains(const Frustum& fr) const
	{
		if (!fr.Intersects(*this))
			return DISJOINT;

		float3 Corners[BoundingFrustum::CORNER_COUNT];
		fr.GetCorners(Corners);

		// Load the box
		XMVECTOR vCenter = XMLoadFloat3(&Center);
		XMVECTOR vExtents = XMLoadFloat3(&Extents);
		XMVECTOR vOrientation =load(mOrientation);

		assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

		for (size_t i = 0; i < BoundingFrustum::CORNER_COUNT; ++i)
		{
			XMVECTOR C = XMVector3InverseRotate(load(Corners[i]) - vCenter, vOrientation);

			if (!XMVector3InBounds(C, vExtents))
				return INTERSECTS;
		}

		return CONTAINS;
	}
	bool OrientedBox::Intersects(const Frustum& fr) const
	{
		return fr.Intersects(*this);
	}
	ContainmentType Frustum::Contains(const OrientedBox& box) const
	{
		// Load origin and orientation of the frustum.
		XMVECTOR vOrigin = XMLoadFloat3(&Origin);
		XMVECTOR vOrientation = XMLoadFloat4(&Orientation);

		// Create 6 planes (do it inline to encourage use of registers)
		XMVECTOR NearPlane = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
		NearPlane = DirectX::Internal::XMPlaneTransform(NearPlane, vOrientation, vOrigin);
		NearPlane = XMPlaneNormalize(NearPlane);

		XMVECTOR FarPlane = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
		FarPlane = DirectX::Internal::XMPlaneTransform(FarPlane, vOrientation, vOrigin);
		FarPlane = XMPlaneNormalize(FarPlane);

		XMVECTOR RightPlane = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
		RightPlane = DirectX::Internal::XMPlaneTransform(RightPlane, vOrientation, vOrigin);
		RightPlane = XMPlaneNormalize(RightPlane);

		XMVECTOR LeftPlane = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
		LeftPlane = DirectX::Internal::XMPlaneTransform(LeftPlane, vOrientation, vOrigin);
		LeftPlane = XMPlaneNormalize(LeftPlane);

		XMVECTOR TopPlane = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
		TopPlane = DirectX::Internal::XMPlaneTransform(TopPlane, vOrientation, vOrigin);
		TopPlane = XMPlaneNormalize(TopPlane);

		XMVECTOR BottomPlane = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);
		BottomPlane = DirectX::Internal::XMPlaneTransform(BottomPlane, vOrientation, vOrigin);
		BottomPlane = XMPlaneNormalize(BottomPlane);

		return box.ContainedBy(NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
	}
	bool Frustum::Intersects(const OrientedBox& box) const
	{
		return Contains(box) != DISJOINT;
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

			float phiStep = XM_PI / stackCount;
			float thetaStep = 2.0f*XM_PI / sliceCount;

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

					XMVECTOR T = load(tangent);
					save(tangent, XMVector3Normalize(T));

					XMVECTOR p = load(pos);
					float3 normal;
					save(normal, XMVector3Normalize(p));

					float2 tex;
					tex.x = theta / XM_2PI;
					tex.y = phi / XM_PI;

					Vertices.emplace_back(pos,normal,tex,tangent);
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

			return{ std::move(Vertices), std::move(Indices) };
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

				m0.pos = float3	(
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
				XMVECTOR n = XMVector3Normalize(load(result.Vertices[i].pos));

				// Project onto sphere.
				XMVECTOR p = radius*n;

				save(result.Vertices[i].pos, p);
				save(result.Vertices[i].normal, n);

				// Derive texture coordinates from spherical coordinates.
				auto theta = atanr(
					result.Vertices[i].pos.x,
					result.Vertices[i].pos.z);

				float phi = acosf(result.Vertices[i].pos.y / radius);

				result.Vertices[i].tex.x = theta / XM_2PI;
				result.Vertices[i].tex.y = phi / LM_PI;

				// Partial derivative of P with respect to theta
				result.Vertices[i].tangent.x = -radius*sinf(phi)*sinf(theta);
				result.Vertices[i].tangent.y = 0.0f;
				result.Vertices[i].tangent.z = +radius*sinf(phi)*cosf(theta);

				XMVECTOR T = load(result.Vertices[i].tangent);
				save(result.Vertices[i].tangent, XMVector3Normalize(T));
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

		lconstexpr std::array<Vertex::PostEffect, 4>& CreateFullscreenQuad()
		{
			using Vertex::PostEffect;
			lconstexpr static std::array<Vertex::PostEffect, 4> result = { 
				PostEffect(float4(+1.f, +1.f, 1.f, 1.f)),
				PostEffect(float4(+1.f, -1.f, 1.f, 1.f)),
				PostEffect(float4(-1.f, +1.f, 1.f, 1.f)),
				PostEffect(float4(-1.f, -1.f, 1.f, 1.f))
			};
			return result;
		}
	}
}