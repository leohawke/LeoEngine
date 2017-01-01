#include "ShaderCompose.h"
#include "Context.h"

using namespace platform_ex::Windows;

void platform_ex::Windows::D3D12::ShaderCompose::Bind()
{
	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	for (leo::uint8 st = 0; st != NumTypes; ++st) {
		//param bind
	}

	if (!barriers.empty()) {
		D3D12::Context::Instance().GetCommandList(D3D12::Device::Command_Render)
			->ResourceBarrier(static_cast<UINT>(barriers.size()),barriers.data());
	}


	SwapAndPresent();
		
	//update cbuffer
}

void platform_ex::Windows::D3D12::ShaderCompose::UnBind()
{
	SwapAndPresent();
}

ID3D12RootSignature * platform_ex::Windows::D3D12::ShaderCompose::RootSignature() const
{
	return nullptr;
}

void platform_ex::Windows::D3D12::ShaderCompose::CreateRootSignature()
{

}

void platform_ex::Windows::D3D12::ShaderCompose::CreateBarriers()
{
	for (leo::uint8 st = 0; st != NumTypes; ++st) {

		D3D12_RESOURCE_BARRIER barrier_before;
		barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		barrier_before.Transition.StateBefore
			= (Type::PixelShader == (Type)st) ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			: D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		//srv barrier

		barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier_before.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		//uav barrier
	}
}

void platform_ex::Windows::D3D12::ShaderCompose::SwapAndPresent()
{
	for (auto & barrier : barriers) {
		std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
	}

	if (!barriers.empty()) {
		D3D12::Context::Instance().GetCommandList(D3D12::Device::Command_Render)
			->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
	}
}
