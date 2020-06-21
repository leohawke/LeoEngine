#pragma once

#include "IGPUResourceView.h"
#include "DeviceCaps.h"

namespace platform::Render {
	enum RenderTargetActions
	{};

	enum class RenderTargetLoadAction
	{
		NoAction
	};

	enum class RenderTargetStoreAction
	{
		NoAction
	};

	class ExclusiveDepthStencil
	{
	public:
		uint32 GetIndex() const
		{
			return 0;
		}
	};

	class RenderTarget
	{
	public:
		Texture* Texture = nullptr;
		uint32 MipIndex = 0;

		uint32 ArraySlice = -1;

		RenderTargetLoadAction LoadAction = RenderTargetLoadAction::NoAction;
		RenderTargetStoreAction StoreAction = RenderTargetStoreAction::NoAction;
	};

	class DepthRenderTarget
	{
	public:
		Texture* Texture = nullptr;
		RenderTargetLoadAction DepthLoadAction = RenderTargetLoadAction::NoAction;
		RenderTargetStoreAction DepthStoreAction = RenderTargetStoreAction::NoAction;
		RenderTargetLoadAction StencilLoadAction = RenderTargetLoadAction::NoAction;
	private:
		RenderTargetStoreAction StencilStoreAction = RenderTargetStoreAction::NoAction;
	};

	struct RenderPassInfo
	{
		struct ColorEntry
		{
			Texture* RenderTarget;
			int32 ArraySlice;
			uint8 MipIndex;
			RenderTargetActions Actions;
		};
		ColorEntry ColorRenderTargets[MaxSimultaneousRenderTargets];

		Texture* DepthStencilTarget;

		int32 NumUAVs = 0;
		UnorderedAccessView* UAVs[MaxSimultaneousUAVs];

		bool bIsMSAA = false;

		// Color and depth
		explicit RenderPassInfo(Texture* ColorRT, Texture* DepthRT)
		{
			ColorRenderTargets[0].RenderTarget = ColorRT;
			ColorRenderTargets[0].ArraySlice = -1;
			ColorRenderTargets[0].MipIndex = 0;

			bIsMSAA = ColorRT->GetSampleCount() > 1;

			DepthStencilTarget = DepthRT;
			std::memset(&ColorRenderTargets[1], 0,sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - 1));
		}
	};

	class RenderTargetsInfo
	{
	public:
		RenderTarget ColorRenderTarget[MaxSimultaneousRenderTargets];
		int32 NumColorRenderTargets = 0;

		bool bClearColor = true;

		DepthRenderTarget DepthStencilRenderTarget;

		bool bClearDepth = true;
		bool bClearStencil = true;

		RenderTargetsInfo(const RenderPassInfo& Info)
		{
			for (int32 Index = 0; Index < MaxSimultaneousRenderTargets; ++Index)
			{
				if (!Info.ColorRenderTargets[Index].RenderTarget)
				{
					break;
				}
				ColorRenderTarget[Index].Texture = Info.ColorRenderTargets[Index].RenderTarget;
				ColorRenderTarget[Index].ArraySlice = Info.ColorRenderTargets[Index].ArraySlice;
				ColorRenderTarget[Index].MipIndex = Info.ColorRenderTargets[Index].MipIndex;
				++NumColorRenderTargets;
			}

			DepthStencilRenderTarget.Texture = Info.DepthStencilTarget;
		}
	};

}
