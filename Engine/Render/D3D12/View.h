#pragma once

#include "d3d12_dxgi.h"
#include "Common.h"
#include "Utility.h"
#include "ResourceHolder.h"
#include "../IGPUResourceView.h"

namespace platform_ex::Windows::D3D12 {
	using platform::Render::SampleDesc;

	class NodeDevice;

	enum ViewSubresourceSubsetFlags
	{
		ViewSubresourceSubsetFlags_None = 0x0,
		ViewSubresourceSubsetFlags_DepthOnlyDsv = 0x1,
		ViewSubresourceSubsetFlags_StencilOnlyDsv = 0x2,
		ViewSubresourceSubsetFlags_DepthAndStencilDsv = (ViewSubresourceSubsetFlags_DepthOnlyDsv | ViewSubresourceSubsetFlags_StencilOnlyDsv),
	};

	class CSubresourceSubset
	{
	public:
		CSubresourceSubset() {}
		
		inline explicit CSubresourceSubset(const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, DXGI_FORMAT ResourceFormat) :
			m_BeginArray(0),
			m_EndArray(1),
			m_BeginMip(0),
			m_EndMip(1),
			m_BeginPlane(0),
			m_EndPlane(1)
		{
			switch (Desc.ViewDimension)
			{
			default: lassume(0 && "Corrupt Resource Type on Shader Resource View"); break;

			case (D3D12_SRV_DIMENSION_BUFFER):
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE1D):
				m_BeginMip = uint8(Desc.Texture1D.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.Texture1D.MipLevels);
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE1DARRAY):
				m_BeginArray = uint16(Desc.Texture1DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture1DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture1DArray.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.Texture1DArray.MipLevels);
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE2D):
				m_BeginMip = uint8(Desc.Texture2D.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.Texture2D.MipLevels);
				m_BeginPlane = uint8(Desc.Texture2D.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2D.PlaneSlice + 1);
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE2DARRAY):
				m_BeginArray = uint16(Desc.Texture2DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture2DArray.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.Texture2DArray.MipLevels);
				m_BeginPlane = uint8(Desc.Texture2DArray.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2DArray.PlaneSlice + 1);
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE2DMS):
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY):
				m_BeginArray = uint16(Desc.Texture2DMSArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DMSArray.ArraySize);
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE3D):
				m_EndArray = uint16(-1); //all slices
				m_BeginMip = uint8(Desc.Texture3D.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.Texture3D.MipLevels);
				break;

			case (D3D12_SRV_DIMENSION_TEXTURECUBE):
				m_BeginMip = uint8(Desc.TextureCube.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.TextureCube.MipLevels);
				m_BeginArray = 0;
				m_EndArray = 6;
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;

			case (D3D12_SRV_DIMENSION_TEXTURECUBEARRAY):
				m_BeginArray = uint16(Desc.TextureCubeArray.First2DArrayFace);
				m_EndArray = uint16(m_BeginArray + Desc.TextureCubeArray.NumCubes * 6);
				m_BeginMip = uint8(Desc.TextureCubeArray.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.TextureCubeArray.MipLevels);
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;
			case (D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE):
				// Nothing here
				break;
			}
		}
		inline explicit CSubresourceSubset(const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc) :
			m_BeginArray(0),
			m_EndArray(1),
			m_BeginMip(0),
			m_BeginPlane(0),
			m_EndPlane(1)
		{
			switch (Desc.ViewDimension)
			{
			default: lassume(0 && "Corrupt Resource Type on Unordered Access View"); break;

			case (D3D12_UAV_DIMENSION_BUFFER): break;

			case (D3D12_UAV_DIMENSION_TEXTURE1D):
				m_BeginMip = uint8(Desc.Texture1D.MipSlice);
				break;

			case (D3D12_UAV_DIMENSION_TEXTURE1DARRAY):
				m_BeginArray = uint16(Desc.Texture1DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture1DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture1DArray.MipSlice);
				break;

			case (D3D12_UAV_DIMENSION_TEXTURE2D):
				m_BeginMip = uint8(Desc.Texture2D.MipSlice);
				m_BeginPlane = uint8(Desc.Texture2D.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2D.PlaneSlice + 1);
				break;

			case (D3D12_UAV_DIMENSION_TEXTURE2DARRAY):
				m_BeginArray = uint16(Desc.Texture2DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture2DArray.MipSlice);
				m_BeginPlane = uint8(Desc.Texture2DArray.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2DArray.PlaneSlice + 1);
				break;

			case (D3D12_UAV_DIMENSION_TEXTURE3D):
				m_BeginArray = uint16(Desc.Texture3D.FirstWSlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture3D.WSize);
				m_BeginMip = uint8(Desc.Texture3D.MipSlice);
				break;
			}

			m_EndMip = m_BeginMip + 1;
		}
		inline explicit CSubresourceSubset(const D3D12_RENDER_TARGET_VIEW_DESC& Desc) :
			m_BeginArray(0),
			m_EndArray(1),
			m_BeginMip(0),
			m_BeginPlane(0),
			m_EndPlane(1)
		{
			switch (Desc.ViewDimension)
			{
			default: lassume(0 && "Corrupt Resource Type on Render Target View"); break;

			case (D3D12_RTV_DIMENSION_BUFFER): break;

			case (D3D12_RTV_DIMENSION_TEXTURE1D):
				m_BeginMip = uint8(Desc.Texture1D.MipSlice);
				break;

			case (D3D12_RTV_DIMENSION_TEXTURE1DARRAY):
				m_BeginArray = uint16(Desc.Texture1DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture1DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture1DArray.MipSlice);
				break;

			case (D3D12_RTV_DIMENSION_TEXTURE2D):
				m_BeginMip = uint8(Desc.Texture2D.MipSlice);
				m_BeginPlane = uint8(Desc.Texture2D.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2D.PlaneSlice + 1);
				break;

			case (D3D12_RTV_DIMENSION_TEXTURE2DMS): break;

			case (D3D12_RTV_DIMENSION_TEXTURE2DARRAY):
				m_BeginArray = uint16(Desc.Texture2DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture2DArray.MipSlice);
				m_BeginPlane = uint8(Desc.Texture2DArray.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2DArray.PlaneSlice + 1);
				break;

			case (D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY):
				m_BeginArray = uint16(Desc.Texture2DMSArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DMSArray.ArraySize);
				break;

			case (D3D12_RTV_DIMENSION_TEXTURE3D):
				m_BeginArray = uint16(Desc.Texture3D.FirstWSlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture3D.WSize);
				m_BeginMip = uint8(Desc.Texture3D.MipSlice);
				break;
			}

			m_EndMip = m_BeginMip + 1;
		}

		inline explicit CSubresourceSubset(const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags) :
			m_BeginArray(0),
			m_EndArray(1),
			m_BeginMip(0),
			m_BeginPlane(0),
			m_EndPlane(GetPlaneCount(ResourceFormat))
		{
			switch (Desc.ViewDimension)
			{
			default: lassume(0 && "Corrupt Resource Type on Depth Stencil View"); break;

			case (D3D12_DSV_DIMENSION_TEXTURE1D):
				m_BeginMip = uint8(Desc.Texture1D.MipSlice);
				break;

			case (D3D12_DSV_DIMENSION_TEXTURE1DARRAY):
				m_BeginArray = uint16(Desc.Texture1DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture1DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture1DArray.MipSlice);
				break;

			case (D3D12_DSV_DIMENSION_TEXTURE2D):
				m_BeginMip = uint8(Desc.Texture2D.MipSlice);
				break;

			case (D3D12_DSV_DIMENSION_TEXTURE2DMS): break;

			case (D3D12_DSV_DIMENSION_TEXTURE2DARRAY):
				m_BeginArray = uint16(Desc.Texture2DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture2DArray.MipSlice);
				break;

			case (D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY):
				m_BeginArray = uint16(Desc.Texture2DMSArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DMSArray.ArraySize);
				break;
			}

			m_EndMip = m_BeginMip + 1;

			if (m_EndPlane == 2)
			{
				if ((Flags & ViewSubresourceSubsetFlags_DepthAndStencilDsv) != ViewSubresourceSubsetFlags_DepthAndStencilDsv)
				{
					if (Flags & ViewSubresourceSubsetFlags_DepthOnlyDsv)
					{
						m_BeginPlane = 0;
						m_EndPlane = 1;
					}
					else if (Flags & ViewSubresourceSubsetFlags_StencilOnlyDsv)
					{
						m_BeginPlane = 1;
						m_EndPlane = 2;
					}
				}
			}
		}

		bool DoesNotOverlap(const CSubresourceSubset& other) const
		{
			if (m_EndArray <= other.m_BeginArray)
			{
				return true;
			}

			if (other.m_EndArray <= m_BeginArray)
			{
				return true;
			}

			if (m_EndMip <= other.m_BeginMip)
			{
				return true;
			}

			if (other.m_EndMip <= m_BeginMip)
			{
				return true;
			}

			if (m_EndPlane <= other.m_BeginPlane)
			{
				return true;
			}

			if (other.m_EndPlane <= m_BeginPlane)
			{
				return true;
			}

			return false;
		}

	protected:
		uint16 m_BeginArray; // Also used to store Tex3D slices.
		uint16 m_EndArray; // End - Begin == Array Slices
		uint8 m_BeginMip;
		uint8 m_EndMip; // End - Begin == Mip Levels
		uint8 m_BeginPlane;
		uint8 m_EndPlane;
	};

	template<typename TDesc>
	class TView;

	class CViewSubresourceSubset : public CSubresourceSubset
	{
		friend class TView < D3D12_SHADER_RESOURCE_VIEW_DESC >;
		friend class TView < D3D12_RENDER_TARGET_VIEW_DESC >;
		friend class TView < D3D12_DEPTH_STENCIL_VIEW_DESC >;
		friend class TView < D3D12_UNORDERED_ACCESS_VIEW_DESC >;

	public:
		CViewSubresourceSubset() {}

		inline CViewSubresourceSubset(uint32 Subresource, uint8 MipLevels, uint16 ArraySize, uint8 PlaneCount)
			: m_MipLevels(MipLevels)
			, m_ArraySlices(ArraySize)
			, m_PlaneCount(PlaneCount)
		{
			if (Subresource < uint32(MipLevels) * uint32(ArraySize))
			{
				m_BeginArray = Subresource / MipLevels;
				m_EndArray = m_BeginArray + 1;
				m_BeginMip = Subresource % MipLevels;
				m_EndMip = m_EndArray + 1;
			}
			else
			{
				m_BeginArray = 0;
				m_BeginMip = 0;
				if (Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
				{
					m_EndArray = ArraySize;
					m_EndMip = MipLevels;
				}
				else
				{
					m_EndArray = 0;
					m_EndMip = 0;
				}
			}
			m_MostDetailedMip = m_BeginMip;
			m_ViewArraySize = m_EndArray - m_BeginArray;
		}

		inline CViewSubresourceSubset(const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags /*Flags*/)
			: CSubresourceSubset(Desc, ResourceFormat)
			, m_MipLevels(MipLevels)
			, m_ArraySlices(ArraySize)
			, m_PlaneCount(GetPlaneCount(ResourceFormat))
		{
			if (Desc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE3D)
			{
				lconstraint(m_BeginArray == 0);
				m_EndArray = 1;
			}
			m_MostDetailedMip = m_BeginMip;
			m_ViewArraySize = m_EndArray - m_BeginArray;
			Reduce();
		}

		inline CViewSubresourceSubset(const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags /*Flags*/)
			: CSubresourceSubset(Desc)
			, m_MipLevels(MipLevels)
			, m_ArraySlices(ArraySize)
			, m_PlaneCount(GetPlaneCount(ResourceFormat))
		{
			if (Desc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE3D)
			{
				m_BeginArray = 0;
				m_EndArray = 1;
			}
			m_MostDetailedMip = m_BeginMip;
			m_ViewArraySize = m_EndArray - m_BeginArray;
			Reduce();
		}

		inline CViewSubresourceSubset(const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags)
			: CSubresourceSubset(Desc, ResourceFormat, Flags)
			, m_MipLevels(MipLevels)
			, m_ArraySlices(ArraySize)
			, m_PlaneCount(GetPlaneCount(ResourceFormat))
		{
			m_MostDetailedMip = m_BeginMip;
			m_ViewArraySize = m_EndArray - m_BeginArray;
			Reduce();
		}

		inline CViewSubresourceSubset(const D3D12_RENDER_TARGET_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags /*Flags*/)
			: CSubresourceSubset(Desc)
			, m_MipLevels(MipLevels)
			, m_ArraySlices(ArraySize)
			, m_PlaneCount(GetPlaneCount(ResourceFormat))
		{
			if (Desc.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE3D)
			{
				m_BeginArray = 0;
				m_EndArray = 1;
			}
			m_MostDetailedMip = m_BeginMip;
			m_ViewArraySize = m_EndArray - m_BeginArray;
			Reduce();
		}

		template<typename T>
		static CViewSubresourceSubset FromView(const T* pView)
		{
			return CViewSubresourceSubset(
				pView->Desc(),
				static_cast<uint8>(pView->GetResource()->GetMipLevels()),
				static_cast<uint16>(pView->GetResource()->GetArraySize()),
				static_cast<uint8>(pView->GetResource()->GetPlaneCount())
			);
		}

	public:
		class CViewSubresourceIterator;

	public:
		CViewSubresourceIterator begin() const;
		CViewSubresourceIterator end() const;
		bool IsWholeResource() const;
		uint32 ArraySize() const;

		uint8 MostDetailedMip() const;
		uint16 ViewArraySize() const;

		uint32 MinSubresource() const;
		uint32 MaxSubresource() const;

	private:
		// Strictly for performance, allows coalescing contiguous subresource ranges into a single range
		inline void Reduce()
		{
			if (m_BeginMip == 0
				&& m_EndMip == m_MipLevels
				&& m_BeginArray == 0
				&& m_EndArray == m_ArraySlices
				&& m_BeginPlane == 0
				&& m_EndPlane == m_PlaneCount)
			{
				uint32 startSubresource = CalcSubresource(0, 0, m_BeginPlane, m_MipLevels, m_ArraySlices);
				uint32 endSubresource = CalcSubresource(0, 0, m_EndPlane, m_MipLevels, m_ArraySlices);

				// Only coalesce if the full-resolution UINTs fit in the UINT8s used for storage here
				if (endSubresource < static_cast<uint8>(-1))
				{
					m_BeginArray = 0;
					m_EndArray = 1;
					m_BeginPlane = 0;
					m_EndPlane = 1;
					m_BeginMip = static_cast<uint8>(startSubresource);
					m_EndMip = static_cast<uint8>(endSubresource);
				}
			}
		}

	protected:
		uint8 m_MipLevels;
		uint16 m_ArraySlices;
		uint8 m_PlaneCount;
		uint8 m_MostDetailedMip;
		uint16 m_ViewArraySize;
	};

	class CViewSubresourceSubset::CViewSubresourceIterator
	{
	public:
		inline CViewSubresourceIterator(CViewSubresourceSubset const& SubresourceSet, uint16 ArraySlice, uint8 PlaneSlice)
			: m_Subresources(SubresourceSet)
			, m_CurrentArraySlice(ArraySlice)
			, m_CurrentPlaneSlice(PlaneSlice)
		{
		}

		inline CViewSubresourceSubset::CViewSubresourceIterator& operator++()
		{
			lconstraint(m_CurrentArraySlice < m_Subresources.m_EndArray);

			if (++m_CurrentArraySlice >= m_Subresources.m_EndArray)
			{
				lconstraint(m_CurrentPlaneSlice < m_Subresources.m_EndPlane);
				m_CurrentArraySlice = m_Subresources.m_BeginArray;
				++m_CurrentPlaneSlice;
			}

			return *this;
		}

		inline CViewSubresourceSubset::CViewSubresourceIterator& operator--()
		{
			if (m_CurrentArraySlice <= m_Subresources.m_BeginArray)
			{
				m_CurrentArraySlice = m_Subresources.m_EndArray;

				lconstraint(m_CurrentPlaneSlice > m_Subresources.m_BeginPlane);
				--m_CurrentPlaneSlice;
			}

			--m_CurrentArraySlice;

			return *this;
		}

		inline bool operator==(CViewSubresourceIterator const& other) const
		{
			return &other.m_Subresources == &m_Subresources
				&& other.m_CurrentArraySlice == m_CurrentArraySlice
				&& other.m_CurrentPlaneSlice == m_CurrentPlaneSlice;
		}

		inline bool operator!=(CViewSubresourceIterator const& other) const
		{
			return !(other == *this);
		}

		inline uint32 StartSubresource() const
		{
			return CalcSubresource(m_Subresources.m_BeginMip, m_CurrentArraySlice, m_CurrentPlaneSlice, m_Subresources.m_MipLevels, m_Subresources.m_ArraySlices);
		}

		inline uint32 EndSubresource() const
		{
			return CalcSubresource(m_Subresources.m_EndMip, m_CurrentArraySlice, m_CurrentPlaneSlice, m_Subresources.m_MipLevels, m_Subresources.m_ArraySlices);
		}

		inline std::pair<uint32, uint32> operator*() const
		{
			std::pair<uint32, uint32> NewPair;
			NewPair.first = StartSubresource();
			NewPair.second = EndSubresource();
			return NewPair;
		}

	private:
		CViewSubresourceSubset const& m_Subresources;
		uint16 m_CurrentArraySlice;
		uint8 m_CurrentPlaneSlice;
	};

	inline CViewSubresourceSubset::CViewSubresourceIterator CViewSubresourceSubset::begin() const
	{
		return CViewSubresourceIterator(*this, m_BeginArray, m_BeginPlane);
	}

	inline CViewSubresourceSubset::CViewSubresourceIterator CViewSubresourceSubset::end() const
	{
		return CViewSubresourceIterator(*this, m_BeginArray, m_EndPlane);
	}

	inline bool CViewSubresourceSubset::IsWholeResource() const
	{
		return m_BeginMip == 0 && m_BeginArray == 0 && m_BeginPlane == 0 && (m_EndMip * m_EndArray * m_EndPlane == m_MipLevels * m_ArraySlices * m_PlaneCount);
	}

	inline uint32 CViewSubresourceSubset::ArraySize() const
	{
		return m_ArraySlices;
	}

	inline uint8 CViewSubresourceSubset::MostDetailedMip() const
	{
		return m_MostDetailedMip;
	}

	inline uint16 CViewSubresourceSubset::ViewArraySize() const
	{
		return m_ViewArraySize;
	}

	inline uint32 CViewSubresourceSubset::MinSubresource() const
	{
		return (*begin()).first;
	}

	inline uint32 CViewSubresourceSubset::MaxSubresource() const
	{
		return (*(--end())).second;
	}

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
		ViewSubresourceSubsetFlags Flags;
		ResourceHolder* ResourceLocation;
		CViewSubresourceSubset ViewSubresourceSubset;
		TDesc Desc;

		explicit TView(NodeDevice* InParent, ViewSubresourceSubsetFlags InFlags)
			: Descriptor(InParent)
			, Flags(InFlags)
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

			ViewSubresourceSubset = CViewSubresourceSubset(Desc,
				static_cast<uint8>(Resource->GetDesc().MipLevels),
				Resource->GetDesc().DepthOrArraySize,
				Resource->GetDesc().Format,
				Flags);
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
		const CViewSubresourceSubset& GetViewSubresourceSubset()	const {
			return ViewSubresourceSubset;
		}
		void SetParentDevice(NodeDevice* InParent)
		{
			Descriptor.SetParentDevice(InParent);
		}
	};

	class RenderTargetView : public TView<D3D12_RENDER_TARGET_VIEW_DESC>
	{
	public:
		RenderTargetView(NodeDevice* InParent, const D3D12_RENDER_TARGET_VIEW_DESC& InRTVDesc, ResourceHolder& InResourceLocation)
			: TView(InParent, ViewSubresourceSubsetFlags_None)
		{
			CreateView(InRTVDesc, InResourceLocation);
		}
	};

	class DepthStencilView :public TView<D3D12_DEPTH_STENCIL_VIEW_DESC>
	{
		const bool bHasDepth;
		const bool bHasStencil;

		CViewSubresourceSubset DepthOnlyViewSubresourceSubset;
		CViewSubresourceSubset StencilOnlyViewSubresourceSubset;
	public:
		DepthStencilView(NodeDevice* InParent, const D3D12_DEPTH_STENCIL_VIEW_DESC& InDSVDesc, ResourceHolder& InResourceLocation, bool InHasStencil)
			: TView(InParent, ViewSubresourceSubsetFlags_DepthAndStencilDsv)
			, bHasDepth(true)				// Assume all DSVs have depth bits in their format
			, bHasStencil(InHasStencil)		// Only some DSVs have stencil bits in their format
		{
			CreateView(InDSVDesc, InResourceLocation);

			auto desc = InResourceLocation.GetDesc();

			// Create individual subresource subsets for each plane
			if (bHasDepth)
			{
				DepthOnlyViewSubresourceSubset = CViewSubresourceSubset(InDSVDesc,
					static_cast<uint8>(desc.MipLevels),
					desc.DepthOrArraySize,
					desc.Format,
					ViewSubresourceSubsetFlags_DepthOnlyDsv);
			}

			if (bHasStencil)
			{
				StencilOnlyViewSubresourceSubset = CViewSubresourceSubset(InDSVDesc,
					static_cast<uint8>(desc.MipLevels),
					desc.DepthOrArraySize,
					desc.Format,
					ViewSubresourceSubsetFlags_StencilOnlyDsv);
			}
		}

		bool HasDepth() const
		{
			return bHasDepth;
		}

		bool HasStencil() const
		{
			return bHasStencil;
		}

		CViewSubresourceSubset& GetDepthOnlyViewSubresourceSubset()
		{
			return DepthOnlyViewSubresourceSubset;
		}

		CViewSubresourceSubset& GetStencilOnlyViewSubresourceSubset()
		{
			return StencilOnlyViewSubresourceSubset;
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
			:TView(InParent, ViewSubresourceSubsetFlags_None)
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
			: TView(InParent, ViewSubresourceSubsetFlags_None)
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