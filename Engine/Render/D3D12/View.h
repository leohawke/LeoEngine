#pragma once

#include "d3d12_dxgi.h"
#include "Common.h"
#include "ResourceHolder.h"
#include "../IGPUResourceView.h"

namespace platform_ex::Windows::D3D12 {
	class NodeDevice;

	template<typename TDesc>
	constexpr bool TIsD3D12SRVDescriptorHandleValue = false;

	template<>
	constexpr bool TIsD3D12SRVDescriptorHandleValue<D3D12_SHADER_RESOURCE_VIEW_DESC> = true;

	template <typename TDesc>
	class TViewDescriptorHandle : public DeviceChild
	{
		template <typename TDesc> struct TCreateViewMap;
		template<> struct TCreateViewMap<D3D12_SHADER_RESOURCE_VIEW_DESC> { static decltype(&ID3D12Device::CreateShaderResourceView)	GetCreate() { return &ID3D12Device::CreateShaderResourceView; } };
		template<> struct TCreateViewMap<D3D12_RENDER_TARGET_VIEW_DESC> { static decltype(&ID3D12Device::CreateRenderTargetView)	GetCreate() { return &ID3D12Device::CreateRenderTargetView; } };
		template<> struct TCreateViewMap<D3D12_DEPTH_STENCIL_VIEW_DESC> { static decltype(&ID3D12Device::CreateDepthStencilView)	GetCreate() { return &ID3D12Device::CreateDepthStencilView; } };
		template<> struct TCreateViewMap<D3D12_UNORDERED_ACCESS_VIEW_DESC> { static decltype(&ID3D12Device::CreateUnorderedAccessView)	GetCreate() { return &ID3D12Device::CreateUnorderedAccessView; } };

		CD3DX12_CPU_DESCRIPTOR_HANDLE Handle;
		uint32 Index;

	public:
		TViewDescriptorHandle(NodeDevice* InParentDevice)
			: DeviceChild(InParentDevice)
		{
			Handle.ptr = 0;
			AllocateDescriptorSlot();
		}

		~TViewDescriptorHandle()
		{
			FreeDescriptorSlot();
		}

		void SetParentDevice(NodeDevice* InParent)
		{
			lconstraint(!Parent && !Handle.ptr);
			DeviceChild::SetParentDevice(InParent);
			AllocateDescriptorSlot();
		}

		void CreateView(const TDesc& Desc, ID3D12Resource* Resource);

		void CreateViewWithCounter(const TDesc& Desc, ID3D12Resource* Resource, ID3D12Resource* CounterResource);

		inline const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetHandle() const { return Handle; }
		inline uint32 GetIndex() const { return Index; }

	private:
		// Implemented in NodeDevice.h due to dependencies on NodeDevice
		void AllocateDescriptorSlot();
		void FreeDescriptorSlot();
	};

	typedef TViewDescriptorHandle<D3D12_SHADER_RESOURCE_VIEW_DESC>		DescriptorHandleSRV;
	typedef TViewDescriptorHandle<D3D12_RENDER_TARGET_VIEW_DESC>		DescriptorHandleRTV;
	typedef TViewDescriptorHandle<D3D12_DEPTH_STENCIL_VIEW_DESC>		DescriptorHandleDSV;
	typedef TViewDescriptorHandle<D3D12_UNORDERED_ACCESS_VIEW_DESC>	DescriptorHandleUAV;

	template <typename TDesc>
	class TView
	{
	private:
		TViewDescriptorHandle<TDesc> Descriptor;

	protected:
		ResourceHolder* ResourceLocation;
		TDesc Desc;


		explicit TView(NodeDevice* InParent)
			: Descriptor(InParent)
		{}

		virtual ~TView()
		{
		}

	private:
		void Initialize(const TDesc& InDesc, ResourceHolder& InResourceLocation)
		{
			ResourceLocation = &InResourceLocation;
			auto* Resource = ResourceLocation->Resource();
			lconstraint(Resource);

			Desc = InDesc;
		}

	protected:
		void CreateView(const TDesc& InDesc, ResourceHolder& InResourceLocation)
		{
			Initialize(InDesc, InResourceLocation);

			ID3D12Resource* D3DResource = ResourceLocation->Resource();
			Descriptor.CreateView(Desc, D3DResource);
		}

		void CreateViewWithCounter(const TDesc& InDesc, ResourceHolder& InResourceLocation, ID3D12Resource* InCounterResource)
		{
			Initialize(InDesc, InResourceLocation);

			ID3D12Resource* D3DResource = ResourceLocation->Resource();
			ID3D12Resource* D3DCounterResource = InCounterResource ? InCounterResource : nullptr;
			Descriptor.CreateViewWithCounter(Desc, D3DResource, D3DCounterResource);
		}

	public:
		inline NodeDevice* GetParentDevice()			const { return Descriptor.GetParentDevice(); }
		inline const TDesc& GetDesc()					const { return Desc; }
		inline CD3DX12_CPU_DESCRIPTOR_HANDLE	GetView()					const { return Descriptor.GetHandle(); }
		inline uint32							GetDescriptorHeapIndex()	const { return Descriptor.GetIndex(); }
		inline ID3D12Resource* GetResource()				const { return ResourceLocation->Resource(); }
		inline ResourceHolder* GetResourceLocation()		const { return ResourceLocation; }

		void SetParentDevice(NodeDevice* InParent)
		{
			Descriptor.SetParentDevice(InParent);
		}
	};

	class RenderTargetView : public TView<D3D12_RENDER_TARGET_VIEW_DESC>
	{
	public:
		RenderTargetView(NodeDevice* InParent, const D3D12_RENDER_TARGET_VIEW_DESC& InRTVDesc, ResourceHolder& InResourceLocation)
			: TView(InParent)
		{
			CreateView(InRTVDesc, InResourceLocation);
		}
	};

	class DepthStencilView :public TView<D3D12_DEPTH_STENCIL_VIEW_DESC>
	{
		const bool bHasDepth;
		const bool bHasStencil;
	public:
		DepthStencilView(NodeDevice* InParent, const D3D12_DEPTH_STENCIL_VIEW_DESC& InRTVDesc, ResourceHolder& InResourceLocation, bool InHasStencil)
			: TView(InParent)
			, bHasDepth(true)				// Assume all DSVs have depth bits in their format
			, bHasStencil(InHasStencil)		// Only some DSVs have stencil bits in their format
		{
			CreateView(InRTVDesc, InResourceLocation);
		}

		bool HasDepth() const
		{
			return bHasDepth;
		}

		bool HasStencil() const
		{
			return bHasStencil;
		}
	};

	class ShaderResourceView : public platform::Render::ShaderResourceView, public TView<D3D12_SHADER_RESOURCE_VIEW_DESC>
	{
		bool bContainsDepthPlane;
		bool bContainsStencilPlane;
		bool bSkipFastClearFinalize;
		uint32 Stride;

	public:
		// Used for all other SRV resource types. Initialization is immediate on the calling thread.
		// Should not be used for dynamic resources which can be renamed.
		ShaderResourceView(NodeDevice* InParent, D3D12_SHADER_RESOURCE_VIEW_DESC& InDesc, ResourceHolder& InResourceLocation, uint32 InStride = -1, bool InSkipFastClearFinalize = false)
			:TView(InParent)
		{
			Initialize(InDesc, InResourceLocation, InStride, InSkipFastClearFinalize);
		}

		~ShaderResourceView()
		{
		}

		void Initialize(D3D12_SHADER_RESOURCE_VIEW_DESC& InDesc, ResourceHolder& InResourceLocation, uint32 InStride, bool InSkipFastClearFinalize = false)
		{
			Stride = InStride;
			bContainsDepthPlane = InResourceLocation.IsDepthStencilResource() && GetPlaneSliceFromViewFormat(InResourceLocation.GetDesc().Format, InDesc.Format) == 0;
			bContainsStencilPlane = InResourceLocation.IsDepthStencilResource() && GetPlaneSliceFromViewFormat(InResourceLocation.GetDesc().Format, InDesc.Format) == 1;
			bSkipFastClearFinalize = InSkipFastClearFinalize;

			CreateView(InDesc, InResourceLocation);
		}

		void Initialize(NodeDevice* InParent, D3D12_SHADER_RESOURCE_VIEW_DESC& InDesc, ResourceHolder& InResourceLocation, uint32 InStride, bool InSkipFastClearFinalize = false)
		{
			if (!this->GetParentDevice())
			{
				// This is a null SRV created without viewing on any resource
				// We need to set its device and allocate a descriptor slot before moving forward
				this->SetParentDevice(InParent);
			}
			lconstraint(GetParentDevice() == InParent);
			Initialize(InDesc, InResourceLocation, InStride, InSkipFastClearFinalize);
		}

		void Rename(ResourceHolder& InResourceLocation)
		{
			// Update the first element index, then reinitialize the SRV
			if (Desc.ViewDimension == D3D12_SRV_DIMENSION_BUFFER)
			{
				Desc.Buffer.FirstElement = InResourceLocation.GetOffsetFromBaseOfResource() / Stride;
			}

			Initialize(Desc, InResourceLocation, Stride);
		}

		void Rename(float ResourceMinLODClamp)
		{
			lconstraint(ResourceLocation);
			lconstraint(Desc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D);

			// Update the LODClamp, the reinitialize the SRV
			Desc.Texture2D.ResourceMinLODClamp = ResourceMinLODClamp;
			CreateView(Desc, *ResourceLocation);
		}

		FORCEINLINE bool IsDepthStencilResource()	const { return bContainsDepthPlane || bContainsStencilPlane; }
		FORCEINLINE bool IsDepthPlaneResource()		const { return bContainsDepthPlane; }
		FORCEINLINE bool IsStencilPlaneResource()	const { return bContainsStencilPlane; }
		FORCEINLINE bool GetSkipFastClearFinalize()	const { return bSkipFastClearFinalize; }
	};

	class UnorderedAccessView : public platform::Render::UnorderedAccessView, public TView < D3D12_UNORDERED_ACCESS_VIEW_DESC >
	{
	public:
		COMPtr<ID3D12Resource> CounterResource;
		bool CounterResourceInitialized;

		UnorderedAccessView(NodeDevice* InParent, D3D12_UNORDERED_ACCESS_VIEW_DESC& InDesc, ResourceHolder& InResourceLocation, ID3D12Resource* InCounterResource = nullptr)
			: TView(InParent)
			, CounterResource(InCounterResource)
			, CounterResourceInitialized(false)
		{
			CreateViewWithCounter(InDesc, InResourceLocation, InCounterResource);
		}

		~UnorderedAccessView()
		{
		}
	};

}