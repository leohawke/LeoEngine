
#ifndef Core_Geometry_Hpp
#define Core_Geometry_Hpp

#include "..\leomath.hpp"

#include "..\IndePlatform\BaseMacro.h"
#include "..\IndePlatform\leoint.hpp"
#include "..\IndePlatform\memory.hpp"

#include "Vertex.hpp"

#include <vector>
namespace leo
{
	enum class FRUSTUM_PLANE_TYPE : std::uint8_t
	{
		FRUSTUM_NEAR, FRUSTUM_FAR,
		FRUSTUM_RIGHT, FRUSTUM_LEFT,
		FRUSTUM_TOP, FRUSTUM_BOTTOM,
		FRUSTUM_PLANES = 6
	};

	enum class PROJECTION_TYPE : std::uint8_t
	{
		ORTHOGRAPHIC,
		PERSPECTIVE
	};

	class Box;
	class Sphere;
	class OrientedBox;
	class Frustum;


	namespace def
	{
		const float frustum_near = 0.25f;
		const float frustum_far = 1024.0f;
		const float frustum_fov = 75.f / 180.f*LM_PI;
		const float frustum_aspect = 16.f / 9.f;
	}

	class Box :public BoundingBox
	{
	public:
		Box() = default;
		Box(const float3& center, const float3& extents)
		{
			assert(extents.x >= 0 && extents.y >= 0 && extents.z >= 0);
			memcpy(Center, center);
			memcpy(Extents, extents);
		}
		Box(const Box& box) = default;

		void operator=(const Box&box)
		{
			std::memcpy(this, &box, sizeof(Box));
		}

	public:
		using BoundingBox::Transform;
		void XM_CALLCONV Transform(Box& out, const SQT& sqt) const
		{
			Transform(out, sqt.operator DirectX::XMMATRIX());
		}

	public:
		using BoundingBox::Contains;

		void GetCorners(float3* corners) const
		{
			XMFLOAT3 _corners[CORNER_COUNT];
			BoundingBox::GetCorners(_corners);
			for (std::uint8_t i = 0; i != CORNER_COUNT; ++i)
				memcpy(corners[i], _corners[i]);
		}

		ContainmentType Contains(const OrientedBox& box) const;

		using BoundingBox::Intersects;

		bool Intersects(const OrientedBox& box) const;

		using BoundingBox::ContainedBy;
	public:
		
		static void CreateFromPoints(Box& Out, std::size_t Count,
			const float3* pPoints)
		{
			BoundingBox::CreateFromPoints(Out, Count, reinterpret_cast<const XMFLOAT3*>(pPoints), sizeof(float3));
		}
	};

	class Sphere : public BoundingSphere
	{
	public:
		Sphere() = default;
		Sphere(const float3& center, float radius)
		{
			memcpy(Center, center);
			Radius = radius;
		}
		Sphere(const Sphere& sp) = default;

		void operator = (const Sphere& sp)
		{
			std::memcpy(this, &sp, sizeof(sp));
		}
	public:
		using BoundingSphere::Transform;
		void XM_CALLCONV Transform(Sphere& out, const SQT& sqt) const
		{
			Transform(out, sqt.operator DirectX::XMMATRIX());
		}
		
		float& radius(float r)
		{
			return Radius = r;
		}
		float radius() const
		{
			return Radius;
		}
		
		float3 center(const float3& c)
		{
			memcpy(Center,c);
			return c;
		}
		float3 center() const
		{
			float3 _center;
			memcpy(_center,Center);
			return _center;
		}
	public:
		using BoundingSphere::Contains;

		ContainmentType Contains(const OrientedBox& box) const;

		using BoundingSphere::Intersects;

		bool Intersects(const OrientedBox& box) const;

		using BoundingSphere::ContainedBy;
	public:

		static void CreateFromBoundingBox(Sphere& Out, const OrientedBox& box);

		static void CreateFromPoints(Sphere& Out, std::size_t Count,
			const float3* pPoints)
		{
			BoundingSphere::CreateFromPoints(Out, Count, reinterpret_cast<const XMFLOAT3*>(pPoints), sizeof(float3));
		}
	};
	
	class OrientedBox : public Box
	{
	public:
		float4 mOrientation;

	public:
		OrientedBox() = default;
		OrientedBox(const float3& center, const float3& extents, const float4& orientation)
			:Box(center, extents), mOrientation(orientation)
		{}
		OrientedBox(const OrientedBox& box)
		{
			std::memcpy(this, &box, sizeof(OrientedBox));
		}
		OrientedBox(const Box& box)
			:Box(box), mOrientation(float4(0.f, 0.f, 0.f, 1.0f))
		{}
		void operator=(const OrientedBox& box)
		{
			std::memcpy(this, &box, sizeof(OrientedBox));
		}
	public:
		void XM_CALLCONV Transform(OrientedBox& out, FXMMATRIX m) const
		{
			using DirectX::operator*;
			// Load the box.
			XMVECTOR vCenter = XMLoadFloat3(&Center);
			XMVECTOR vExtents = XMLoadFloat3(&Extents);
			XMVECTOR vOrientation = load(mOrientation);

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Composite the box rotation and the transform rotation.
			XMMATRIX nM;
			nM.r[0] = XMVector3Normalize(m.r[0]);
			nM.r[1] = XMVector3Normalize(m.r[1]);
			nM.r[2] = XMVector3Normalize(m.r[2]);
			nM.r[3] = g_XMIdentityR3;
			XMVECTOR Rotation = XMQuaternionRotationMatrix(nM);
			vOrientation = XMQuaternionMultiply(vOrientation, Rotation);

			// Transform the center.
			vCenter = XMVector3Transform(vCenter, m);

			// Scale the box extents.
			XMVECTOR dX = XMVector3Length(m.r[0]);
			XMVECTOR dY = XMVector3Length(m.r[1]);
			XMVECTOR dZ = XMVector3Length(m.r[2]);

			XMVECTOR VectorScale = XMVectorSelect(dX, dY, g_XMSelect1000);
			VectorScale = XMVectorSelect(VectorScale, dZ, g_XMSelect1100);
			vExtents *= VectorScale;

			// Store the box.
			XMStoreFloat3(&out.Center, vCenter);
			XMStoreFloat3(&out.Extents, vExtents);
			save(out.mOrientation, vOrientation);
		}
		void XM_CALLCONV Transform(OrientedBox& out, const SQT& sqt) const
		{
			Transform(out, sqt.operator DirectX::XMMATRIX());
		}
	public:
		void GetCorners(float3* corners) const
		{
			assert(corners);

			// Load the box
			XMVECTOR vCenter = XMLoadFloat3(&Center);
			XMVECTOR vExtents = XMLoadFloat3(&Extents);
			XMVECTOR vOrientation = load(mOrientation);

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			for (size_t i = 0; i < CORNER_COUNT; ++i)
			{
				XMVECTOR C = XMVector3Rotate(vExtents * g_BoxOffset[i], vOrientation) + vCenter;
				save(corners[i], C);
			}
		}

		ContainmentType    XM_CALLCONV     Contains(FXMVECTOR Point) const
		{
			XMVECTOR center = XMLoadFloat3(&Center);
			XMVECTOR extents = XMLoadFloat3(&Extents);
			XMVECTOR orientation = load(mOrientation);
			XMVECTOR local_point = XMVector3InverseRotate(Point - center, orientation);
			return XMVector3InBounds(local_point, extents) ? CONTAINS : DISJOINT;
		}
		ContainmentType    XM_CALLCONV     Contains(FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2) const
		{
			// Load the box center & orientation.
			XMVECTOR center = XMLoadFloat3(&Center);
			XMVECTOR orientation = load(mOrientation);

			// Transform the triangle vertices into the space of the box.
			XMVECTOR TV0 = XMVector3InverseRotate(V0 - center, orientation);
			XMVECTOR TV1 = XMVector3InverseRotate(V1 - center, orientation);
			XMVECTOR TV2 = XMVector3InverseRotate(V2 - center, orientation);

			BoundingBox box;
			box.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
			box.Extents = Extents;

			// Use the triangle vs axis aligned box intersection routine.
			return box.Contains(TV0, TV1, TV2);
		}
		ContainmentType Contains(const Sphere& sh) const
		{
			XMVECTOR SphereCenter = XMLoadFloat3(&sh.Center);
			XMVECTOR SphereRadius = XMVectorReplicatePtr(&sh.Radius);

			XMVECTOR BoxCenter = XMLoadFloat3(&Center);
			XMVECTOR BoxExtents = XMLoadFloat3(&Extents);
			XMVECTOR BoxOrientation = load(mOrientation);

			assert(DirectX::Internal::XMQuaternionIsUnit(BoxOrientation));

			// Transform the center of the sphere to be local to the box.
			// BoxMin = -BoxExtents
			// BoxMax = +BoxExtents
			SphereCenter = XMVector3InverseRotate(SphereCenter - BoxCenter, BoxOrientation);

			// Find the distance to the nearest point on the box.
			// for each i in (x, y, z)
			// if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
			// else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2
			XMVECTOR d = XMVectorZero();

			// Compute d for each dimension.
			XMVECTOR LessThanMin = XMVectorLess(SphereCenter, -BoxExtents);
			XMVECTOR GreaterThanMax = XMVectorGreater(SphereCenter, BoxExtents);

			XMVECTOR MinDelta = SphereCenter + BoxExtents;
			XMVECTOR MaxDelta = SphereCenter - BoxExtents;

			// Choose value for each dimension based on the comparison.
			d = XMVectorSelect(d, MinDelta, LessThanMin);
			d = XMVectorSelect(d, MaxDelta, GreaterThanMax);

			// Use a dot-product to square them and sum them together.
			XMVECTOR d2 = XMVector3Dot(d, d);
			XMVECTOR SphereRadiusSq = XMVectorMultiply(SphereRadius, SphereRadius);

			if (XMVector4Greater(d2, SphereRadiusSq))
				return DISJOINT;

			// See if we are completely inside the box
			XMVECTOR SMin = SphereCenter - SphereRadius;
			XMVECTOR SMax = SphereCenter + SphereRadius;

			return (XMVector3InBounds(SMin, BoxExtents) && XMVector3InBounds(SMax, BoxExtents)) ? CONTAINS : INTERSECTS;
		}
		ContainmentType Contains(const Box& box) const
		{
			OrientedBox obox(box);
			return Contains(obox);
		}
		ContainmentType Contains(const OrientedBox& box) const
		{
			if (!Intersects(box))
				return DISJOINT;

			// Load the boxes
			XMVECTOR aCenter = XMLoadFloat3(&Center);
			XMVECTOR aExtents = XMLoadFloat3(&Extents);
			XMVECTOR aOrientation = load(mOrientation);

			assert(DirectX::Internal::XMQuaternionIsUnit(aOrientation));

			XMVECTOR bCenter = XMLoadFloat3(&box.Center);
			XMVECTOR bExtents = XMLoadFloat3(&box.Extents);
			XMVECTOR bOrientation = load(box.mOrientation);

			assert(DirectX::Internal::XMQuaternionIsUnit(bOrientation));

			XMVECTOR offset = bCenter - aCenter;

			for (size_t i = 0; i < CORNER_COUNT; ++i)
			{
				// Cb = rotate( bExtents * corneroffset[i], bOrientation ) + bcenter
				// Ca = invrotate( Cb - aCenter, aOrientation )

				XMVECTOR C = XMVector3Rotate(bExtents * g_BoxOffset[i], bOrientation) + offset;
				C = XMVector3InverseRotate(C, aOrientation);

				if (!XMVector3InBounds(C, aExtents))
					return INTERSECTS;
			}

			return CONTAINS;
		}
		ContainmentType Contains(const Frustum& fr) const;

		bool Intersects(const Sphere& sh) const
		{
			return Contains(sh) != DISJOINT;
		}
		bool Intersects(const Box& box) const
		{
			return Intersects(OrientedBox(box));
		}
		//主实现代码
		bool Intersects(const OrientedBox& box) const
		{
			// Build the 3x3 rotation matrix that defines the orientation of B relative to A.
			XMVECTOR A_quat = load(mOrientation);
			XMVECTOR B_quat = load(box.mOrientation);

			assert(DirectX::Internal::XMQuaternionIsUnit(A_quat));
			assert(DirectX::Internal::XMQuaternionIsUnit(B_quat));

			XMVECTOR Q = XMQuaternionMultiply(A_quat, XMQuaternionConjugate(B_quat));
			XMMATRIX R = XMMatrixRotationQuaternion(Q);

			// Compute the translation of B relative to A.
			XMVECTOR A_cent = XMLoadFloat3(&Center);
			XMVECTOR B_cent = XMLoadFloat3(&box.Center);
			XMVECTOR t = XMVector3InverseRotate(B_cent - A_cent, A_quat);

			//
			// h(A) = extents of A.
			// h(B) = extents of B.
			//
			// a(u) = axes of A = (1,0,0), (0,1,0), (0,0,1)
			// b(u) = axes of B relative to A = (r00,r10,r20), (r01,r11,r21), (r02,r12,r22)
			//  
			// For each possible separating axis l:
			//   d(A) = sum (for i = u,v,w) h(A)(i) * abs( a(i) dot l )//首先将a（i） b（i） 
			//   d(B) = sum (for i = u,v,w) h(B)(i) * abs( b(i) dot l )//投影至轴l，然后将Extents投影
			//   if abs( t dot l ) > d(A) + d(B) then disjoint//计算Extends的投影长度
			//

			// Load extents of A and B.
			XMVECTOR h_A = XMLoadFloat3(&Extents);
			XMVECTOR h_B = XMLoadFloat3(&box.Extents);

			// Rows. Note R[0,1,2]X.w = 0.
			XMVECTOR R0X = R.r[0];
			XMVECTOR R1X = R.r[1];
			XMVECTOR R2X = R.r[2];

			R = XMMatrixTranspose(R);

			// Columns. Note RX[0,1,2].w = 0.
			XMVECTOR RX0 = R.r[0];
			XMVECTOR RX1 = R.r[1];
			XMVECTOR RX2 = R.r[2];

			// Absolute value of rows.
			XMVECTOR AR0X = XMVectorAbs(R0X);
			XMVECTOR AR1X = XMVectorAbs(R1X);
			XMVECTOR AR2X = XMVectorAbs(R2X);

			// Absolute value of columns.
			XMVECTOR ARX0 = XMVectorAbs(RX0);
			XMVECTOR ARX1 = XMVectorAbs(RX1);
			XMVECTOR ARX2 = XMVectorAbs(RX2);

			// Test each of the 15 possible seperating axii.
			XMVECTOR d, d_A, d_B;

			// l = a(u) = (1, 0, 0)
			// t dot l = t.x
			// d(A) = h(A).x
			// d(B) = h(B) dot abs(r00, r01, r02)
			d = XMVectorSplatX(t);
			d_A = XMVectorSplatX(h_A);
			d_B = XMVector3Dot(h_B, AR0X);
			XMVECTOR NoIntersection = XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B));

			// l = a(v) = (0, 1, 0)
			// t dot l = t.y
			// d(A) = h(A).y
			// d(B) = h(B) dot abs(r10, r11, r12)
			d = XMVectorSplatY(t);
			d_A = XMVectorSplatY(h_A);
			d_B = XMVector3Dot(h_B, AR1X);
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(w) = (0, 0, 1)
			// t dot l = t.z
			// d(A) = h(A).z
			// d(B) = h(B) dot abs(r20, r21, r22)
			d = XMVectorSplatZ(t);
			d_A = XMVectorSplatZ(h_A);
			d_B = XMVector3Dot(h_B, AR2X);
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = b(u) = (r00, r10, r20)
			// d(A) = h(A) dot abs(r00, r10, r20)
			// d(B) = h(B).x
			d = XMVector3Dot(t, RX0);
			d_A = XMVector3Dot(h_A, ARX0);
			d_B = XMVectorSplatX(h_B);
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = b(v) = (r01, r11, r21)
			// d(A) = h(A) dot abs(r01, r11, r21)
			// d(B) = h(B).y
			d = XMVector3Dot(t, RX1);
			d_A = XMVector3Dot(h_A, ARX1);
			d_B = XMVectorSplatY(h_B);
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = b(w) = (r02, r12, r22)
			// d(A) = h(A) dot abs(r02, r12, r22)
			// d(B) = h(B).z
			d = XMVector3Dot(t, RX2);
			d_A = XMVector3Dot(h_A, ARX2);
			d_B = XMVectorSplatZ(h_B);
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(u) x b(u) = (0, -r20, r10)
			// d(A) = h(A) dot abs(0, r20, r10)
			// d(B) = h(B) dot abs(0, r02, r01)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0W, XM_PERMUTE_1Z, XM_PERMUTE_0Y, XM_PERMUTE_0X>(RX0, -RX0));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(ARX0));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(AR0X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(u) x b(v) = (0, -r21, r11)
			// d(A) = h(A) dot abs(0, r21, r11)
			// d(B) = h(B) dot abs(r02, 0, r00)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0W, XM_PERMUTE_1Z, XM_PERMUTE_0Y, XM_PERMUTE_0X>(RX1, -RX1));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(ARX1));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(AR0X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(u) x b(w) = (0, -r22, r12)
			// d(A) = h(A) dot abs(0, r22, r12)
			// d(B) = h(B) dot abs(r01, r00, 0)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0W, XM_PERMUTE_1Z, XM_PERMUTE_0Y, XM_PERMUTE_0X>(RX2, -RX2));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(ARX2));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(AR0X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(v) x b(u) = (r20, 0, -r00)
			// d(A) = h(A) dot abs(r20, 0, r00)
			// d(B) = h(B) dot abs(0, r12, r11)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0Z, XM_PERMUTE_0W, XM_PERMUTE_1X, XM_PERMUTE_0Y>(RX0, -RX0));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(ARX0));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(AR1X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(v) x b(v) = (r21, 0, -r01)
			// d(A) = h(A) dot abs(r21, 0, r01)
			// d(B) = h(B) dot abs(r12, 0, r10)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0Z, XM_PERMUTE_0W, XM_PERMUTE_1X, XM_PERMUTE_0Y>(RX1, -RX1));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(ARX1));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(AR1X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(v) x b(w) = (r22, 0, -r02)
			// d(A) = h(A) dot abs(r22, 0, r02)
			// d(B) = h(B) dot abs(r11, r10, 0)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0Z, XM_PERMUTE_0W, XM_PERMUTE_1X, XM_PERMUTE_0Y>(RX2, -RX2));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(ARX2));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(AR1X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(w) x b(u) = (-r10, r00, 0)
			// d(A) = h(A) dot abs(r10, r00, 0)
			// d(B) = h(B) dot abs(0, r22, r21)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_1Y, XM_PERMUTE_0X, XM_PERMUTE_0W, XM_PERMUTE_0Z>(RX0, -RX0));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(ARX0));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(AR2X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(w) x b(v) = (-r11, r01, 0)
			// d(A) = h(A) dot abs(r11, r01, 0)
			// d(B) = h(B) dot abs(r22, 0, r20)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_1Y, XM_PERMUTE_0X, XM_PERMUTE_0W, XM_PERMUTE_0Z>(RX1, -RX1));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(ARX1));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(AR2X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(w) x b(w) = (-r12, r02, 0)
			// d(A) = h(A) dot abs(r12, r02, 0)
			// d(B) = h(B) dot abs(r21, r20, 0)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_1Y, XM_PERMUTE_0X, XM_PERMUTE_0W, XM_PERMUTE_0Z>(RX2, -RX2));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(ARX2));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(AR2X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// No seperating axis found, boxes must intersect.
			return XMVector4NotEqualInt(NoIntersection, XMVectorTrueInt()) ? true : false;
		}
		bool Intersects(const Frustum& fr) const;

		bool    XM_CALLCONV     Intersects(FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2) const
		{
			return Contains(V0, V1, V2) != DISJOINT;
		}
		// Triangle-OrientedBox test

		PlaneIntersectionType    XM_CALLCONV     Intersects(FXMVECTOR Plane) const
		{
			assert(DirectX::Internal::XMPlaneIsUnit(Plane));

			// Load the box.
			XMVECTOR vCenter = XMLoadFloat3(&Center);
			XMVECTOR vExtents = XMLoadFloat3(&Extents);
			XMVECTOR BoxOrientation = load(mOrientation);

			assert(DirectX::Internal::XMQuaternionIsUnit(BoxOrientation));

			// Set w of the center to one so we can dot4 with a plane.
			vCenter = XMVectorInsert<0, 0, 0, 0, 1>(vCenter, XMVectorSplatOne());

			// Build the 3x3 rotation matrix that defines the box axes.
			XMMATRIX R = XMMatrixRotationQuaternion(BoxOrientation);

			XMVECTOR Outside, Inside;
			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], Plane, Outside, Inside);

			// If the box is outside any plane it is outside.
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return FRONT;

			// If the box is inside all planes it is inside.
			if (XMVector4EqualInt(Inside, XMVectorTrueInt()))
				return BACK;

			// The box is not inside all planes or outside a plane it intersects.
			return INTERSECTING;
		}
		// Plane-OrientedBox test

		bool    XM_CALLCONV     Intersects(FXMVECTOR Origin, FXMVECTOR Direction, float& Dist) const
		{
			assert(DirectX::Internal::XMVector3IsUnit(Direction));

			static const XMVECTORI32 SelectY =
			{
				static_cast<int32_t>(XM_SELECT_0), static_cast<int32_t>(XM_SELECT_1), static_cast<int32_t>(XM_SELECT_0), static_cast<int32_t>(XM_SELECT_0)
			};
			static const XMVECTORI32 SelectZ =
			{
				static_cast<int32_t>(XM_SELECT_0), static_cast<int32_t>(XM_SELECT_0), static_cast<int32_t>(XM_SELECT_1), static_cast<int32_t>(XM_SELECT_0)
			};

			// Load the box.
			XMVECTOR vCenter = XMLoadFloat3(&Center);
			XMVECTOR vExtents = XMLoadFloat3(&Extents);
			XMVECTOR vOrientation = load(mOrientation);

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Get the boxes normalized side directions.
			XMMATRIX R = XMMatrixRotationQuaternion(vOrientation);

			// Adjust ray origin to be relative to center of the box.
			XMVECTOR TOrigin = vCenter - Origin;

			// Compute the dot product againt each axis of the box.
			XMVECTOR AxisDotOrigin = XMVector3Dot(R.r[0], TOrigin);
			AxisDotOrigin = XMVectorSelect(AxisDotOrigin, XMVector3Dot(R.r[1], TOrigin), SelectY);
			AxisDotOrigin = XMVectorSelect(AxisDotOrigin, XMVector3Dot(R.r[2], TOrigin), SelectZ);

			XMVECTOR AxisDotDirection = XMVector3Dot(R.r[0], Direction);
			AxisDotDirection = XMVectorSelect(AxisDotDirection, XMVector3Dot(R.r[1], Direction), SelectY);
			AxisDotDirection = XMVectorSelect(AxisDotDirection, XMVector3Dot(R.r[2], Direction), SelectZ);

			// if (fabs(AxisDotDirection) <= Epsilon) the ray is nearly parallel to the slab.
			XMVECTOR IsParallel = XMVectorLessOrEqual(XMVectorAbs(AxisDotDirection), g_RayEpsilon);

			// Test against all three axes simultaneously.
			XMVECTOR InverseAxisDotDirection = XMVectorReciprocal(AxisDotDirection);
			XMVECTOR t1 = (AxisDotOrigin - vExtents) * InverseAxisDotDirection;
			XMVECTOR t2 = (AxisDotOrigin + vExtents) * InverseAxisDotDirection;

			// Compute the max of min(t1,t2) and the min of max(t1,t2) ensuring we don't
			// use the results from any directions parallel to the slab.
			XMVECTOR t_min = XMVectorSelect(XMVectorMin(t1, t2), g_FltMin, IsParallel);
			XMVECTOR t_max = XMVectorSelect(XMVectorMax(t1, t2), g_FltMax, IsParallel);

			// t_min.x = maximum( t_min.x, t_min.y, t_min.z );
			// t_max.x = minimum( t_max.x, t_max.y, t_max.z );
			t_min = XMVectorMax(t_min, XMVectorSplatY(t_min));  // x = max(x,y)
			t_min = XMVectorMax(t_min, XMVectorSplatZ(t_min));  // x = max(max(x,y),z)
			t_max = XMVectorMin(t_max, XMVectorSplatY(t_max));  // x = min(x,y)
			t_max = XMVectorMin(t_max, XMVectorSplatZ(t_max));  // x = min(min(x,y),z)

			// if ( t_min > t_max ) return false;
			XMVECTOR NoIntersection = XMVectorGreater(XMVectorSplatX(t_min), XMVectorSplatX(t_max));

			// if ( t_max < 0.0f ) return false;
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(XMVectorSplatX(t_max), XMVectorZero()));

			// if (IsParallel && (-Extents > AxisDotOrigin || Extents < AxisDotOrigin)) return false;
			XMVECTOR ParallelOverlap = XMVectorInBounds(AxisDotOrigin, vExtents);
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorAndCInt(IsParallel, ParallelOverlap));

			if (!DirectX::Internal::XMVector3AnyTrue(NoIntersection))
			{
				// Store the x-component to *pDist
				XMStoreFloat(&Dist, t_min);
				return true;
			}

			Dist = 0.f;
			return false;
		}
		// Ray-OrientedBox test

		ContainmentType     XM_CALLCONV     ContainedBy(FXMVECTOR Plane0, FXMVECTOR Plane1, FXMVECTOR Plane2,
			GXMVECTOR Plane3, HXMVECTOR Plane4, HXMVECTOR Plane5) const
		{
			// Load the box.
			XMVECTOR vCenter = XMLoadFloat3(&Center);
			XMVECTOR vExtents = XMLoadFloat3(&Extents);
			XMVECTOR BoxOrientation = load(mOrientation);

			assert(DirectX::Internal::XMQuaternionIsUnit(BoxOrientation));

			// Set w of the center to one so we can dot4 with a plane.
			vCenter = XMVectorInsert<0, 0, 0, 0, 1>(vCenter, XMVectorSplatOne());

			// Build the 3x3 rotation matrix that defines the box axes.
			XMMATRIX R = XMMatrixRotationQuaternion(BoxOrientation);

			XMVECTOR Outside, Inside;

			// Test against each plane.
			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], Plane0, Outside, Inside);

			XMVECTOR AnyOutside = Outside;
			XMVECTOR AllInside = Inside;

			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], Plane1, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], Plane2, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], Plane3, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], Plane4, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], Plane5, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			// If the box is outside any plane it is outside.
			if (XMVector4EqualInt(AnyOutside, XMVectorTrueInt()))
				return DISJOINT;

			// If the box is inside all planes it is inside.
			if (XMVector4EqualInt(AllInside, XMVectorTrueInt()))
				return CONTAINS;

			// The box is not inside all planes or outside a plane, it may intersect.
			return INTERSECTS;
		}
		// Test OrientedBox against six planes (see BoundingFrustum::GetPlanes)

		// Static methods
		static void CreateFromBoundingBox(OrientedBox& Out, const Box& box)
		{
			Out = OrientedBox(box);
		}

		static void CreateFromPoints(OrientedBox& Out, std::size_t Count,
			const float3 *pPoints)
		{
			assert(Count > 0);
			assert(pPoints != 0);

			XMVECTOR CenterOfMass = XMVectorZero();

			// Compute the center of mass and inertia tensor of the points.
			for (size_t i = 0; i < Count; ++i)
			{
				XMVECTOR Point = load(*(pPoints + i));

				CenterOfMass += Point;
			}

			CenterOfMass *= XMVectorReciprocal(XMVectorReplicate(float(Count)));

			// Compute the inertia tensor of the points around the center of mass.
			// Using the center of mass is not strictly necessary, but will hopefully
			// improve the stability of finding the eigenvectors.
			XMVECTOR XX_YY_ZZ = XMVectorZero();
			XMVECTOR XY_XZ_YZ = XMVectorZero();

			for (size_t i = 0; i < Count; ++i)
			{
				XMVECTOR Point = load(*(pPoints + i)) - CenterOfMass;

				XX_YY_ZZ += Point * Point;

				XMVECTOR XXY = XMVectorSwizzle<XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_Y, XM_SWIZZLE_W>(Point);
				XMVECTOR YZZ = XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_W>(Point);

				XY_XZ_YZ += XXY * YZZ;
			}

			XMVECTOR v1, v2, v3;

			// Compute the eigenvectors of the inertia tensor.
			DirectX::Internal::CalculateEigenVectorsFromCovarianceMatrix(XMVectorGetX(XX_YY_ZZ), XMVectorGetY(XX_YY_ZZ),
				XMVectorGetZ(XX_YY_ZZ),
				XMVectorGetX(XY_XZ_YZ), XMVectorGetY(XY_XZ_YZ),
				XMVectorGetZ(XY_XZ_YZ),
				&v1, &v2, &v3);

			// Put them in a matrix.
			XMMATRIX R;

			R.r[0] = XMVectorSetW(v1, 0.f);
			R.r[1] = XMVectorSetW(v2, 0.f);
			R.r[2] = XMVectorSetW(v3, 0.f);
			R.r[3] = g_XMIdentityR3.v;

			// Multiply by -1 to convert the matrix into a right handed coordinate 
			// system (Det ~= 1) in case the eigenvectors form a left handed 
			// coordinate system (Det ~= -1) because XMQuaternionRotationMatrix only 
			// works on right handed matrices.
			XMVECTOR Det = XMMatrixDeterminant(R);

			if (XMVector4Less(Det, XMVectorZero()))
			{
				R.r[0] *= g_XMNegativeOne.v;
				R.r[1] *= g_XMNegativeOne.v;
				R.r[2] *= g_XMNegativeOne.v;
			}

			// Get the rotation quaternion from the matrix.
			XMVECTOR vOrientation = XMQuaternionRotationMatrix(R);

			// Make sure it is normal (in case the vectors are slightly non-orthogonal).
			vOrientation = XMQuaternionNormalize(vOrientation);

			// Rebuild the rotation matrix from the quaternion.
			R = XMMatrixRotationQuaternion(vOrientation);

			// Build the rotation into the rotated space.
			XMMATRIX InverseR = XMMatrixTranspose(R);

			// Find the minimum OBB using the eigenvectors as the axes.
			XMVECTOR vMin, vMax;

			vMin = vMax = XMVector3TransformNormal(load(*pPoints), InverseR);

			for (size_t i = 1; i < Count; ++i)
			{
				XMVECTOR Point = XMVector3TransformNormal(load(*(pPoints + i)),
					InverseR);

				vMin = XMVectorMin(vMin, Point);
				vMax = XMVectorMax(vMax, Point);
			}

			// Rotate the center into world space.
			XMVECTOR vCenter = (vMin + vMax) * 0.5f;
			vCenter = XMVector3TransformNormal(vCenter, R);

			// Store center, extents, and orientation.
			XMStoreFloat3(&Out.Center, vCenter);
			XMStoreFloat3(&Out.Extents, (vMax - vMin) * 0.5f);
			save(Out.mOrientation, vOrientation);
		}
	};

	

	class Frustum : public  BoundingFrustum
	{
	public:
		// Creators
		Frustum() = default;
		Frustum(const float3& _Origin, const float4& _Orientation,
			 float _RightSlope,  float _LeftSlope,  float _TopSlope,  float _BottomSlope,
			 float _Near,  float _Far)
		{ 
			memcpy(Origin, _Origin);
			memcpy(Orientation, _Orientation);
			RightSlope = _RightSlope;
			LeftSlope = _LeftSlope;
			TopSlope = _TopSlope;
			BottomSlope = (_BottomSlope);
			Near = (_Near); Far = (_Far);
			assert(_Near <= _Far);
		}
		Frustum(const Frustum& fr) = default;

		Frustum(CXMMATRIX Projection) 
		{ 
			CreateFromMatrix(*this, Projection); 
		}

		// Methods
		void operator=(const Frustum& fr) 
		{
			std::memcpy(this, &fr, sizeof(Frustum));
		}

		using BoundingFrustum::Transform;
		void    XM_CALLCONV     Transform(Frustum& Out, const SQT& sqt) const
		{
			Transform(Out, sqt.operator DirectX::XMMATRIX());
		}

		void GetCorners(float3* Corners) const
		{
			XMFLOAT3 _corners[8];
			BoundingFrustum::GetCorners(_corners);
			for (std::uint8_t i = 0; i != 8; ++i)
				memcpy(Corners[i], _corners[i]);
		}
		// Gets the 8 corners of the frustum

		using BoundingFrustum::Contains;
		ContainmentType Contains( const OrientedBox& box) const;
		// Frustum-Frustum test

		using BoundingFrustum::Intersects;
		bool Intersects( const OrientedBox& box) const;

		
		using BoundingFrustum::CreateFromMatrix;
		using BoundingFrustum::GetPlanes;
		using BoundingFrustum::ContainedBy;
	};

	class ViewFrustum : public Frustum
	{
	public:
		~ViewFrustum() = default;
		ViewFrustum(const ViewFrustum& lvaue) = default;
		ViewFrustum(const float3& origin = float3(0.0f, 0.0f, 0.f), const float4& orientation = float4(0.0f, 0.0f, 0.0f, 1.0f), float fov = def::frustum_fov,
			float aspect = def::frustum_aspect, PROJECTION_TYPE projtype = PROJECTION_TYPE::PERSPECTIVE,
			float nearf = def::frustum_near, float farf = def::frustum_far)
			:mProjType(projtype), mAspect(aspect), mFov(fov)
		{
			memcpy(Orientation, orientation);
			memcpy(Origin, origin);
			Near = nearf;
			Far = farf;
			_update();
		}
	private:
		void _update()
		{
			float sinfov, cosfov;
			sinfov = sincosr(&cosfov, mFov / 2.f);
			auto cotfov = static_cast<float>(cosfov / sinfov);
			auto tanfov = static_cast<float>(sinfov / cosfov);
			//for 正交投影计算
			mNearHeight = 2 * Near / cotfov;
			auto width = mNearHeight*mAspect;
			float fRange = Far / (Far - Near);
			switch (mProjType)
			{
			case leo::PROJECTION_TYPE::PERSPECTIVE:
				mMatrix.m[0][0] = cotfov / mAspect;
				mMatrix.m[1][1] = cotfov;
				break;
			case leo::PROJECTION_TYPE::ORTHOGRAPHIC:
				mMatrix.m[0][0] = 2.0f / width;
				mMatrix.m[1][1] = 2.0f / mNearHeight;
				break;
			default:
				break;
			}
			//矩阵的相同值
			{
				mMatrix.m[0][1] = 0.0f;
				mMatrix.m[0][2] = 0.0f;
				mMatrix.m[0][3] = 0.0f;

				mMatrix.m[1][2] = 0.0f;
				mMatrix.m[1][3] = 0.0f;
				mMatrix.m[1][0] = 0.0f;
				mMatrix.m[2][0] = 0.0f;
				mMatrix.m[2][1] = 0.0f;
				mMatrix.m[2][2] = fRange;
				mMatrix.m[2][3] = 1.0f;

				mMatrix.m[3][0] = 0.0f;
				mMatrix.m[3][1] = 0.0f;
				mMatrix.m[3][2] = -fRange * Near;
				mMatrix.m[3][3] = 0.0f;
			}

			RightSlope = +tanfov*mAspect;
			LeftSlope = -tanfov*mAspect;
			TopSlope = +tanfov;
			BottomSlope = -tanfov;

			if (mProjType == PROJECTION_TYPE::ORTHOGRAPHIC)
			{
				RightSlope /= Far;
				LeftSlope /= Far;
				TopSlope /= Far;
				BottomSlope /= Far;
			}
		}
	public:
		//构造函数过长而分拆的函数
		void SetFrustum(const float3& origin, const float4& orientation = float4(0.0f, 0.0f, 0.0f, 1.0f))
		{
			memcpy(Orientation, orientation);
			memcpy(Origin, origin);
		}
		void SetFrustum(float fov, float aspect, float nearf, float farf)
		{
			Near = nearf;
			Far = farf;
			mFov = fov;
			mAspect = aspect;
			_update();
		}
		void SetFrustum(PROJECTION_TYPE projtype)
		{
			mProjType = projtype;
			_update();
		}

		DefGetter(lnoexcept(), float&, Fov, mFov);

		DefGetter(lnoexcept(), float&, Far, Far);

		DefGetter(lnoexcept(), float&, Near, Near);

		DefGetter(lnoexcept(), float&, Aspect, mAspect);

		DefGetter(lnoexcept(), float&, Height, mNearHeight);

		DefGetter(lnoexcept(), XMFLOAT3&, Origin, Origin);

		DefGetter(const lnoexcept(), XMFLOAT3, Origin, Origin);
		inline XMMATRIX Proj() const
		{
			return loadfloat4x4(&mMatrix);
		}
		//关键函数
	protected:
		using Frustum::Origin;
		using Frustum::Orientation;

		using Frustum::Far;
		using Frustum::Near;

		/// Orthographic(正交) or Perspective(投影)?
		PROJECTION_TYPE mProjType = PROJECTION_TYPE::PERSPECTIVE;

		/// y 视角域,默认 75
		float mFov = def::frustum_fov;

		///横纵比,默认 16:9
		float mAspect = 16.f / 9.f;

		/// 近裁剪面高度
		float mNearHeight = 0;

		float4x4 mMatrix;
	};

	namespace helper
	{
		struct MeshData
		{
			std::vector < Vertex::NormalMap> Vertices;
			std::vector<uint32> Indices;
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
		lconstexpr std::array<Vertex::PostEffect,4>& CreateFullscreenQuad();
		
	}
}

#endif