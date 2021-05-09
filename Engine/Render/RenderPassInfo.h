#pragma once

#include "IGPUResourceView.h"
#include "DeviceCaps.h"

namespace platform::Render {
	enum class RenderTargetLoadAction
	{
		NoAction,
		Load,
		Clear,
	};

	enum class RenderTargetStoreAction
	{
		NoAction,
		Store,
	};

	enum RenderTargetActions
	{
		LoadOpMask = 2,

#define RTACTION_MAKE_MASK(Load, Store) (((uint8)RenderTargetLoadAction::Load << (uint8)LoadOpMask) | (uint8)RenderTargetStoreAction::Store)

		DontLoad_DontStore = RTACTION_MAKE_MASK(NoAction, NoAction),

		DontLoad_Store = RTACTION_MAKE_MASK(NoAction, Store),

		Clear_Store = RTACTION_MAKE_MASK(Clear, Store),

		Load_Store = RTACTION_MAKE_MASK(Load, Store),

		Clear_DontStore = RTACTION_MAKE_MASK(Clear, NoAction),
#undef RTACTION_MAKE_MASK
	};

	inline RenderTargetLoadAction GetLoadAction(RenderTargetActions Action)
	{
		return (RenderTargetLoadAction)((uint8)Action >> (uint8)RenderTargetActions::LoadOpMask);
	}

	inline RenderTargetStoreAction GetStoreAction(RenderTargetActions Action)
	{
		return (RenderTargetStoreAction)((uint8)Action & ((1 << (uint8)RenderTargetActions::LoadOpMask) - 1));
	}

	enum class DepthStencilTargetActions : uint8
	{
		DepthMask = 4,

#define RTACTION_MAKE_MASK(Depth, Stencil) (((uint8)RenderTargetActions::Depth << (uint8)DepthMask) | (uint8)RenderTargetActions::Stencil)

		DontLoad_DontStore = RTACTION_MAKE_MASK(DontLoad_DontStore, DontLoad_DontStore),

		ClearDepthStencil_StoreDepthStencil = RTACTION_MAKE_MASK(Clear_Store, Clear_Store),

		LoadDepthStencil_StoreDepthStencil = RTACTION_MAKE_MASK(Load_Store, Load_Store),

		ClearDepthStencil_StoreDepthNotStencil = RTACTION_MAKE_MASK(Clear_Store, Clear_DontStore),
#undef RTACTION_MAKE_MASK
	};

	inline RenderTargetActions GetDepthActions(DepthStencilTargetActions Action)
	{
		return (RenderTargetActions)((uint8)Action >> (uint8)DepthStencilTargetActions::DepthMask);
	}

	inline RenderTargetActions GetStencilActions(DepthStencilTargetActions Action)
	{
		return (RenderTargetActions)((uint8)Action & ((1 << (uint8)DepthStencilTargetActions::DepthMask) - 1));
	}

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

		explicit DepthRenderTarget()
		{
		}

		explicit DepthRenderTarget(platform::Render::Texture* InTexture, RenderTargetLoadAction InDepthLoadAction, RenderTargetStoreAction InDepthStoreAction, RenderTargetLoadAction InStencilLoadAction, RenderTargetStoreAction InStencilStoreAction) 
			:
			Texture(InTexture),
			DepthLoadAction(InDepthLoadAction),
			DepthStoreAction(InDepthStoreAction),
			StencilLoadAction(InStencilLoadAction),
			StencilStoreAction(InStencilStoreAction)
		{
		}
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
			RenderTargetActions Action;
		};
		ColorEntry ColorRenderTargets[MaxSimultaneousRenderTargets];

		struct DepthStencilEntry
		{
			Texture* DepthStencilTarget;
			DepthStencilTargetActions Action;
		};

		DepthStencilEntry DepthStencilRenderTarget;

		int32 NumUAVs = 0;
		UnorderedAccessView* UAVs[MaxSimultaneousUAVs];

		bool bIsMSAA = false;

		// Color ,no depth
		explicit RenderPassInfo(Texture* ColorRT, RenderTargetActions ColorAction)
		{
			ColorRenderTargets[0].RenderTarget = ColorRT;
			ColorRenderTargets[0].ArraySlice = -1;
			ColorRenderTargets[0].MipIndex = 0;
			ColorRenderTargets[0].Action = ColorAction;

			bIsMSAA = ColorRT->GetSampleCount() > 1;

			DepthStencilRenderTarget.DepthStencilTarget = nullptr;
			DepthStencilRenderTarget.Action = DepthStencilTargetActions::DontLoad_DontStore;
			std::memset(&ColorRenderTargets[1], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - 1));
		}

		// Depth, no color
		explicit RenderPassInfo(Texture* DepthRT, DepthStencilTargetActions DepthActions)
		{
			bIsMSAA = DepthRT->GetSampleCount() > 1;

			DepthStencilRenderTarget.DepthStencilTarget = DepthRT;
			DepthStencilRenderTarget.Action = DepthActions;
			std::memset(ColorRenderTargets, 0, sizeof(ColorEntry) * MaxSimultaneousRenderTargets);
		}

		// Color and depth
		explicit RenderPassInfo(Texture* ColorRT, RenderTargetActions ColorAction, Texture* DepthRT, DepthStencilTargetActions DepthActions)
		{
			ColorRenderTargets[0].RenderTarget = ColorRT;
			ColorRenderTargets[0].ArraySlice = -1;
			ColorRenderTargets[0].MipIndex = 0;
			ColorRenderTargets[0].Action = ColorAction;

			bIsMSAA = ColorRT->GetSampleCount() > 1;

			DepthStencilRenderTarget.DepthStencilTarget = DepthRT;
			DepthStencilRenderTarget.Action = DepthActions;
			std::memset(&ColorRenderTargets[1], 0,sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - 1));
		}
	};

	class RenderTargetsInfo
	{
	public:
		RenderTarget ColorRenderTarget[MaxSimultaneousRenderTargets];
		int32 NumColorRenderTargets = 0;

		bool bClearColor = false;

		DepthRenderTarget DepthStencilRenderTarget;

		bool bClearDepth = false;
		bool bClearStencil = false;

		RenderTargetsInfo(const RenderPassInfo& Info);
	};

}
