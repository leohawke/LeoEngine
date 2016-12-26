#include "RenderView.h"
#include "Context.h"

using namespace platform_ex;
using namespace platform_ex::Windows::D3D12;

ViewSimulation::ViewSimulation(COMPtr<ID3D12Resource> & Res, D3D12_DESCRIPTOR_HEAP_TYPE Type)
	:res(Res), type(Type), device(Context::Instance().GetDevice()) {
	handle = device.AllocDescriptor(type);
}
ViewSimulation::ViewSimulation(COMPtr<ID3D12Resource> & resource, D3D12_SHADER_RESOURCE_VIEW_DESC const & desc)
	:ViewSimulation(resource, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) {
	device->CreateShaderResourceView(resource.Get(), &desc, handle);
}
ViewSimulation::ViewSimulation(COMPtr<ID3D12Resource> & resource, D3D12_RENDER_TARGET_VIEW_DESC const & desc)
	:ViewSimulation(resource, D3D12_DESCRIPTOR_HEAP_TYPE_RTV) {
	device->CreateRenderTargetView(resource.Get(), &desc, handle);
}
ViewSimulation::ViewSimulation(COMPtr<ID3D12Resource> & resource, D3D12_DEPTH_STENCIL_VIEW_DESC const & desc)
	:ViewSimulation(resource, D3D12_DESCRIPTOR_HEAP_TYPE_DSV) {
	device->CreateDepthStencilView(resource.Get(), &desc, handle);
}

ViewSimulation::ViewSimulation(COMPtr<ID3D12Resource> & resource, D3D12_UNORDERED_ACCESS_VIEW_DESC const & desc)
	:ViewSimulation(resource, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) {
	ID3D12Resource* counter = nullptr;
	if (D3D12_UAV_DIMENSION_BUFFER == desc.ViewDimension) {
		if (auto counter_offset = desc.Buffer.CounterOffsetInBytes)
			counter = resource.Get();
	}

	device->CreateUnorderedAccessView(resource.Get(), counter, &desc, handle);
}

ViewSimulation::~ViewSimulation() {
	auto & device = Context::Instance().GetDevice();
	device.DeallocDescriptor(type, handle);
}