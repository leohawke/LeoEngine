#include "RayContext.h"
#include "RayDevice.h"
#include "Context.h"
#include "GraphicsBuffer.hpp"

namespace D12 = platform_ex::Windows::D3D12;
namespace R = platform::Render;

using namespace D12;

bool D12::IsDirectXRaytracingSupported(ID3D12Device* device)
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};

	return SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,&featureSupportData,sizeof(featureSupportData)))
		&& featureSupportData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

D12::RayDevice& D12::RayContext::GetDevice()
{
	return *ray_device;
}

ID3D12GraphicsCommandList4* D12::RayContext::RayTracingCommandList() const
{
	return raytracing_command_list.Get();
}

D12::RayDevice::RayDevice(Device* pDevice, Context* pContext)
	:device(pDevice), context(pContext)
{
	if (!SUCCEEDED(device->d3d_device->QueryInterface(COMPtr_RefParam(d3d_ray_device, IID_ID3D12Device5))))
		throw leo::unsupported();
}

D12::RayTracingGeometry* D12::RayDevice::CreateRayTracingGeometry(const R::RayTracingGeometryInitializer& initializer)
{
	D12::RayTracingGeometry* Geometry = new D12::RayTracingGeometry();

	Geometry->IndexBuffer = static_cast<GraphicsBuffer*>(initializer.IndexBuffer);

	leo::uint32 IndexStride = Geometry->IndexBuffer ? NumFormatBytes(Geometry->IndexBuffer->GetFormat()) : 0;
	Geometry->IndexStride = IndexStride;
	Geometry->IndexOffsetInBytes = initializer.IndexBufferOffset;

	switch (initializer.GeometryType)
	{
	case R::ERayTracingGeometryType::Triangles:
		Geometry->GeometryType = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		break;
	case R::ERayTracingGeometryType::Procedural:
		Geometry->GeometryType = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
		break;
	default:
		break;
	}

	bool fastBuild = false;
	bool allowUpdate = false;

	if (fastBuild)
	{
		Geometry->BuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
	}
	else
	{
		Geometry->BuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	}

	if (allowUpdate)
	{
		Geometry->BuildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	}

	Geometry->Segement = initializer.Segement;

	return Geometry;
}

RayTracingScene* D12::RayDevice::CreateRayTracingScene(const R::RayTracingSceneInitializer& initializer)
{
	return nullptr;
}

void D12::RayDevice::BuildAccelerationStructure(R::RayTracingGeometry* geometry)
{
	RayTracingGeometry* pGeometry = static_cast<RayTracingGeometry*>(geometry);


	//TODO:transition all vbs and ibs to readable state

	pGeometry->BuildAccelerationStructure();
}

D12::RayContext::RayContext(Device* pDevice, Context* pContext)
	:device(pDevice),context(pContext)
{
	ray_device = std::make_shared<RayDevice>(pDevice,pContext);

	context->GetCommandList(Device::Command_Render)->QueryInterface(COMPtr_RefParam(raytracing_command_list, IID_ID3D12GraphicsCommandList4));

	if(!IsDirectXRaytracingSupported(pDevice->d3d_device.Get()))
		throw leo::unsupported();
}