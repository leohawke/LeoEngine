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

	
}