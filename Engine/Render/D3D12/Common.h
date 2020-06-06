#pragma once

/*

|-[Render::Context]--
					 |-[Adapter]-- (LDA)
					 |			|
					 |			|- [Device]
					 |			|
					 |			|- [Device]
					 |
					 |-[Adapter]--
					 			|
					 			|- [Device]--
					 						|
					 						|-[CommandContext]
					 						|
					 						|-[CommandContext]---
					 											|
Under this scheme an D3D12Device represents 1 node belonging to 1 physical adapter.

This structure allows a single Render::Context to control several different hardware setups. Some example arrangements:
	- Single-GPU systems (the common case)
	- Multi-GPU systems i.e. LDA (Crossfire/SLI)
	- Asymmetric Multi-GPU systems i.e. Discrete/Integrated GPU cooperation												|-[StateCache]
*/

#include <LBase/linttype.hpp>

namespace platform_ex::Windows::D3D12 {
	using namespace leo::inttype;

	class Device;

	//TODO:different hardware setups
	//CommonCase Map
	using D3D12Device = Device;

	using D3D12Adapter = Device;

	class DeviceChild
	{
	protected:
		D3D12Device* Parent;
	public:
		DeviceChild(D3D12Device* InParent = nullptr) : Parent(InParent) {}
	};

	class GPUObject
	{
	public:
		GPUObject()
			:GPUMask(0)
			, VisibilityMask(0)
		{
		}
	protected:
		leo::uint32 GPUMask;
		// Which GPUs have direct access to this object
		leo::uint32 VisibilityMask;
	};

	class SingleNodeGPUObject : GPUObject
	{
	public:
		SingleNodeGPUObject()
			:GPUObject()
			,GPUIndex(0)
		{}

	protected:
		uint32 GPUIndex;
	};
}