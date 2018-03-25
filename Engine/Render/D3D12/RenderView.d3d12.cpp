#include "RenderView.h"
#include "Context.h"
#include "GraphicsBuffer.hpp"
#include "Texture.h"

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


GPUDataStructView::GPUDataStructView(ID3D12Resource* res_,ViewSimulation * view_, uint16 first_subres_, uint16 num_subres_)
	:view(view_),first_subres(first_subres_),num_subres(num_subres_),res(res_)
{
}

observer_ptr<ViewSimulation> GPUDataStructView::View()
{
	return view;
}

uint16 GPUDataStructView::FirstSubResIndex()
{
	return first_subres;
}

uint16 GPUDataStructView::SubResNum()
{
	return num_subres;
}

ID3D12Resource * GPUDataStructView::Resource()
{
	return res.get();
}

RenderTargetView::RenderTargetView(Texture2D & texture, uint8 first_array_index, uint8 array_size, uint8 level)
	:GPUDataStructView(
		texture.Resource(),
		texture.RetriveRenderTargetView(first_array_index,array_size,level),
		first_array_index *texture.GetNumMipMaps() + level,
		1)
	, base(texture.GetWidth(level), texture.GetHeight(level), texture.GetFormat())
{
}

RenderTargetView::RenderTargetView(Texture3D & texture, uint8 array_index, uint8 first_slice, uint8 num_slices, uint8 level):GPUDataStructView(
	texture.Resource(),
	texture.RetriveRenderTargetView(array_index,first_slice,num_slices,level),
	(array_index * texture.GetDepth(level) + first_slice) * texture.GetNumMipMaps() + level,
	num_slices * texture.GetNumMipMaps() + level)
	, base(texture.GetWidth(level), texture.GetHeight(level), texture.GetFormat())
{
}

RenderTargetView::RenderTargetView(TextureCube & texture, uint8 array_index, platform::Render::TextureCubeFaces face, uint8 level):
	GPUDataStructView(
		texture.Resource(),
		texture.RetriveRenderTargetView(array_index,face,level),
		(array_index * 6 + face) * texture.GetNumMipMaps() + level,
		1)
	, base(texture.GetWidth(level), texture.GetHeight(level), texture.GetFormat())
{
}

RenderTargetView::RenderTargetView(GraphicsBuffer & gb, uint16 width, uint16 height, platform::Render::EFormat pf)
	:GPUDataStructView(
		gb.Resource(),
		gb.RetriveRenderTargetView(width,height,pf),
		0,
		1)
	, base(width,height, pf)
{
}

DepthStencilView::DepthStencilView(Texture2D & texture, uint8 first_array_index, uint8 array_size, uint8 level)
	:GPUDataStructView(
		texture.Resource(),
		texture.RetriveDepthStencilView(first_array_index, array_size, level),
		first_array_index *texture.GetNumMipMaps() + level,
		1)
	, base(texture.GetWidth(level), texture.GetHeight(level), texture.GetFormat())
{
}

DepthStencilView::DepthStencilView(Texture3D & texture, uint8 array_index, uint8 first_slice, uint8 num_slices, uint8 level) :GPUDataStructView(
	texture.Resource(),
	texture.RetriveDepthStencilView(array_index, first_slice, num_slices, level),
	(array_index * texture.GetDepth(level) + first_slice) * texture.GetNumMipMaps() + level,
	num_slices * texture.GetNumMipMaps() + level)
	, base(texture.GetWidth(level), texture.GetHeight(level), texture.GetFormat())
{
}

DepthStencilView::DepthStencilView(TextureCube & texture, uint8 array_index, platform::Render::TextureCubeFaces face, uint8 level) :
	GPUDataStructView(
		texture.Resource(),
		texture.RetriveDepthStencilView(array_index, face, level),
		(array_index * 6 + face) * texture.GetNumMipMaps() + level,
		1)
	, base(texture.GetWidth(level), texture.GetHeight(level), texture.GetFormat())
{
}


UnorderedAccessView::UnorderedAccessView(Texture2D & texture, uint8 first_array_index, uint8 array_size, uint8 level)
	:GPUDataStructView(
		texture.Resource(),
		texture.RetriveUnorderedAccessView(first_array_index, array_size, level),
		first_array_index *texture.GetNumMipMaps() + level,
		1)
	, base(texture.GetWidth(level), texture.GetHeight(level), texture.GetFormat())
{
}

UnorderedAccessView::UnorderedAccessView(Texture3D & texture, uint8 array_index, uint8 first_slice, uint8 num_slices, uint8 level) :GPUDataStructView(
	texture.Resource(),
	texture.RetriveUnorderedAccessView(array_index, first_slice, num_slices, level),
	(array_index * texture.GetDepth(level) + first_slice) * texture.GetNumMipMaps() + level,
	num_slices * texture.GetNumMipMaps() + level)
	, base(texture.GetWidth(level), texture.GetHeight(level), texture.GetFormat())
{
}

UnorderedAccessView::UnorderedAccessView(TextureCube & texture, uint8 array_index, platform::Render::TextureCubeFaces face, uint8 level)
	:
	GPUDataStructView(
		texture.Resource(),
		texture.RetriveUnorderedAccessView(array_index, 0,face,1, level),
		(array_index * 6 + face) * texture.GetNumMipMaps() + level,
		1)
	, base(texture.GetWidth(level), texture.GetHeight(level), texture.GetFormat())
{
}

UnorderedAccessView::UnorderedAccessView(GraphicsBuffer & gb, platform::Render::EFormat pf):
GPUDataStructView(
	gb.Resource(),
	gb.RetriveUnorderedAccessView(),
	0,
	1)
	, base(width, height, pf)
{
}

void UnorderedAccessView::ResetInitCount()
{
}
