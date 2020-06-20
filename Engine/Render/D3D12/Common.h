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
					 						|-[CommandContext]--
					 										  |-[StateCache]
Under this scheme an NodeDevice represents 1 node belonging to 1 physical adapter.

This structure allows a single Render::Context to control several different hardware setups. Some example arrangements:
	- Single-GPU systems (the common case)
	- Multi-GPU systems i.e. LDA (Crossfire/SLI)
	- Asymmetric Multi-GPU systems i.e. Discrete/Integrated GPU cooperation												
*/

#include <LBase/cassert.h>
#include <LBase/linttype.hpp>

namespace platform_ex::Windows::D3D12 {
	using namespace leo::inttype;

	constexpr auto MAX_NUM_GPUNODES = 4;

	class Device;

	//TODO:different hardware setups
	//CommonCase Map
	class NodeDevice;

	using D3D12Adapter = Device;

	using GPUMaskType = leo::uint32;

	enum class CommandQueueType
	{
		Default,
		Copy,
		Async
	};

	class DeviceChild
	{
	protected:
		NodeDevice* Parent;
	public:
		DeviceChild(NodeDevice* InParent = nullptr) : Parent(InParent) {}

		NodeDevice* GetParentDevice() const{ return Parent; }

		void SetParentDevice(NodeDevice* InParent)
		{
			lconstraint(Parent == nullptr);
			Parent = InParent;
		}
	};

	class AdapterChild
	{
	protected:
		D3D12Adapter* ParentAdapter;

	public:
		AdapterChild(D3D12Adapter* InParent = nullptr) : ParentAdapter(InParent) {}

		D3D12Adapter* GetParentAdapter() { return ParentAdapter; }

		// To be used with delayed setup
		void SetParentAdapter(D3D12Adapter* InParent)
		{
			ParentAdapter = InParent;
		}
	};

	class GPUObject
	{
	public:
		GPUObject()
			:GPUMask(0)
			, VisibilityMask(0)
		{
		}

		GPUObject(GPUMaskType InGPUMask, GPUMaskType InVisibiltyMask)
			: GPUMask(InGPUMask)
			, VisibilityMask(InVisibiltyMask)
		{
			// Note that node mask can't be null.
		}

		const GPUMaskType GetGPUMask() const { return GPUMask; }
	protected:
		GPUMaskType GPUMask;
		// Which GPUs have direct access to this object
		GPUMaskType VisibilityMask;
	};

	class SingleNodeGPUObject : GPUObject
	{
	public:
		SingleNodeGPUObject(GPUMaskType GPUMask = 0)
			:GPUObject(GPUMask,GPUMask)
			,GPUIndex(0)
		{}

	protected:
		uint32 GPUIndex;
	};

	class MultiNodeGPUObject :public GPUObject
	{
	public:
		MultiNodeGPUObject(GPUMaskType NodeMask, GPUMaskType VisibiltyMask)
			:GPUObject(NodeMask, VisibiltyMask)
		{}
	};

	NodeDevice* GetDefaultNodeDevice();
}