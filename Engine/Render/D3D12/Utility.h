#pragma once

#include <LBase/linttype.hpp>
#include "d3d12_dxgi.h"


namespace platform_ex::Windows::D3D12 {
	class Fence;

	class SyncPoint
	{
	public:
		explicit SyncPoint()
			: Fence(nullptr)
			, Value(0)
		{
		}

		explicit SyncPoint(Fence* InFence, uint64 InValue)
			: Fence(InFence)
			, Value(InValue)
		{
		}

		bool IsValid() const;
		bool IsComplete() const;
		void WaitForCompletion() const;

	private:
		Fence* Fence;
		uint64 Value;
	};

	/** Find the appropriate depth-stencil typeless DXGI format for the given format. */
	inline DXGI_FORMAT FindDepthStencilParentDXGIFormat(DXGI_FORMAT InFormat)
	{
		switch (InFormat)
		{
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_R24G8_TYPELESS;
			// Changing Depth Buffers to 32 bit on Dingo as D24S8 is actually implemented as a 32 bit buffer in the hardware
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_R32G8X24_TYPELESS;
		case DXGI_FORMAT_D32_FLOAT:
			return DXGI_FORMAT_R32_TYPELESS;
		case DXGI_FORMAT_D16_UNORM:
			return DXGI_FORMAT_R16_TYPELESS;
		};
		return InFormat;
	}

	inline uint8 GetPlaneSliceFromViewFormat(DXGI_FORMAT ResourceFormat, DXGI_FORMAT ViewFormat)
	{
		// Currently, the only planar resources used are depth-stencil formats
		switch (FindDepthStencilParentDXGIFormat(ResourceFormat))
		{
		case DXGI_FORMAT_R24G8_TYPELESS:
			switch (ViewFormat)
			{
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
				return 0;
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
				return 1;
			}
			break;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			switch (ViewFormat)
			{
			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
				return 0;
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
				return 1;
			}
			break;
		}

		return 0;
	}

	inline uint8 GetPlaneCount(DXGI_FORMAT Format)
	{
		// Currently, the only planar resources used are depth-stencil formats
		// Note there is a D3D12 helper for this, D3D12GetFormatPlaneCount
		switch (FindDepthStencilParentDXGIFormat(Format))
		{
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			return 2;
		default:
			return 1;
		}
	}
}